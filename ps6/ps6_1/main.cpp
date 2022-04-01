#include "fssimplewindow.h"
#include "ysclass.h"
#include "stdio.h"
#include "math.h"
#include "time.h"
#include "stdlib.h"
#include <chrono>
#include <thread> 
#include <mutex> 
#include <functional> 

// From Bounce MT
class WorkerThread
{
private:
    enum
    {
        TASK_NONE,
        TASK_QUIT,
        TASK_RUN
    };

    int taskType=TASK_NONE;
    std::function <void()> task;
    std::thread thr;
    std::mutex mtx;
    std::condition_variable cond;
    void ThreadFunc();
public:
    WorkerThread();
    ~WorkerThread();
    void Run(std::function <void()>);
    void Wait(void);
};

WorkerThread::WorkerThread()
{
    std::thread t(&WorkerThread::ThreadFunc,this);
    thr.swap(t);
}

WorkerThread::~WorkerThread()
{
    taskType=TASK_QUIT;
    cond.notify_one();
    thr.join();
}

void WorkerThread::Run(std::function <void()> newTask)
{
    mtx.lock();
    taskType=TASK_RUN;
    task=newTask;
    mtx.unlock();
    cond.notify_one();
}

void WorkerThread::Wait(void)
{
  std::unique_lock <std::mutex> lock(mtx); 
  cond.wait(lock,[&]{return taskType==TASK_NONE;});
}

void WorkerThread::ThreadFunc()
{
    for(;;)
    {
        std::unique_lock <std::mutex> lock(mtx);
        cond.wait(lock,[&]{return taskType!=TASK_NONE;});
        if(TASK_QUIT==taskType)
        {
            break;
        }
        else if(TASK_RUN==taskType)
        {
            task();
            taskType=TASK_NONE;
            cond.notify_one();
        }
    }
}

class FluidParticle
{
public:
	double m=1.0,rho=1.0;
	YsVec3 pos=YsVec3::Origin(),vel=YsVec3::Origin(),f=YsVec3::Origin();
	YsVec3 nom=YsVec3::Origin();
};

// Wpoly6 used everywhere except pressure and viscosity.
// Wspiky used for pressure force.
// Wviscosity used for viscosity force.

class Wpoly6
{
public:
	inline static double Evaluate(double r,double h)
	{
		if(r<=h)
		{
			double h2=h*h;
			double h3=h2*h;
			double h6=h3*h3;
			double h9=h6*h3;
			double hhrr=h2-r*r;
			return 315.0*hhrr*hhrr*hhrr/(64.0*YsPi*h9);
		}
		return 0.0;
	}
	inline static double Delta(double r,double h)
	{
		if(r<=h)
		{
			double h2=h*h;
			double h3=h2*h;
			double h4=h2*h2;
			double h6=h3*h3;
			double h9=h6*h3;
			double r2=r*r;
			double r3=r2*r;
			double r5=r2*r3;
			return 315.0*(-6.0*r*h4+12.0*r3*h2-6.0*r5)/(64.0*YsPi*h9);
		}
		return 0.0;
	}
	inline static double Laplacian(double r,double h)
	{
		if(r<=h)
		{
			double h2=h*h;
			double h3=h2*h;
			double h4=h2*h2;
			double h6=h3*h3;
			double h9=h6*h3;
			double r2=r*r;
			double r4=r2*r2;
			return 315.0*(-6.0*h4+36.0*r2*h2-30.0*r4)/(64.0*YsPi*h9);
		}
		return 0.0;
	}
};

class Wspiky
{
public:
	inline static double Evaluate(double r,double h)
	{
		if(r<=h)
		{
			double h2=h*h;
			double h3=h2*h;
			double h6=h3*h3;
			double hr=h-r;
			return 15.0*hr*hr*hr/(YsPi*h6);
		}
		return 0.0;
	}
	inline static double Delta(double r,double h)
	{
		if(r<=h)
		{
			double r2=r*r;
			double h2=h*h;
			double h3=h2*h;
			double h6=h3*h3;
			return 15.0*(-3.0*r2-3.0*h2+6.0*h*r)/(YsPi*h6);
		}
		return 0.0;
	}
	inline static double Laplacian(double r,double h)
	{
		if(r<=h)
		{
			double r2=r*r;
			double h2=h*h;
			double h3=h2*h;
			double h6=h3*h3;
			return 15.0*(-6.0*r+6.0*h)/(YsPi*h6);
		}
		return 0.0;
	}
};
class Wviscosity
{
public:
	inline static double Evaluate(double r,double h)
	{
		r=std::max(YsTolerance,r);
		if(r<=h)
		{
			double h2=h*h;
			double h3=h2*h;
			double h6=h3*h3;
			double r2=r*r;
			double r3=r2*r;
			return 15.0*(-r3/(2.0*h3)+r2/h2+h/(2.0*r)-1.0)/(2.0*YsPi*h3);
		}
		return 0.0;
	}
	inline static double Laplacian(double r,double h)
	{
		// Equation (22 and 1/2) of Muller 03
		if(r<=h)
		{
			double h2=h*h;
			double h4=h2*h2;
			double h6=h4*h2;
			return 45.0*(h-r)/(YsPi*h6);
		}
		return 0.0;
	}
};

class FluidSimulator
{
private:
	// k is the gas constant, and
	// rho0 is the rest density appears in Equation (12) of Muller 03
	YsVec3 PressureAndViscosity(const FluidParticle &receiver,const FluidParticle &actor,double k,double h,double rho0,double mu)
	{
		double pi=k*(receiver.rho-rho0);
		double pj=k*(actor.rho-rho0);
		double r=(receiver.pos-actor.pos).GetLength();

		// Equation (10) of Mueller 03
		double press=(actor.m*(pi+pj)/(2.0*actor.rho))*Wspiky::Delta(r,h);
		YsVec3 fPressure=receiver.pos-actor.pos;
		fPressure.Normalize();
		fPressure*=press;

		auto vjvi=actor.vel-receiver.vel;
		YsVec3 fViscosity=mu*actor.m*vjvi*Wviscosity::Laplacian(r,h)/actor.rho;

		return -fPressure+fViscosity;
	}

	YsVec3 WallForce(const FluidParticle &receiver) const
	{
		YsVec3 total=YsVec3::Origin();
		auto rho=receiver.rho;
		auto m=receiver.m;
		auto p=k*(receiver.rho-rho0);
		/*if(receiver.pos.x()<minX+h)
		{
			auto r=receiver.pos.x()-minX;
			if(r<0.0)
			{
				r=0.0;
			}
			double press=-(m*(p+p)/(2.0*rho))*Wspiky::Delta(r,h);
			total+=YsXVec()*press;
		}
		else if(maxX<receiver.pos.x()+h)
		{
			auto r=maxX-receiver.pos.x();
			if(r<0.0)
			{
				r=0.0;
			}
			double press=-(m*(p+p)/(2.0*rho))*Wspiky::Delta(r,h);
			total-=YsXVec()*press;
		}*/
		if(receiver.pos.y()<minY+h)
		{
			auto r=receiver.pos.y()-minY;
			if(r<0.0)
			{
				r=0.0;
			}
			double press=-(m*(p+p)/(2.0*rho))*Wspiky::Delta(r,h);
			total+=YsYVec()*press;
		}
		return total;
	}


public:
	bool twoDimension=true;
	double minX=-20.0,maxX=20.0,minY=0.0;                            /////////////////////////////////// 4.2 Expand the Field and Increase the Particle Count
	double rho0=0.5;
	double k=8.0;      // Pressure
	double mu=3.0;     // Viscosity
	double sigma=2.0;  // Surface tension
	double h=1.0;
	double gravity=9.8;
	std::vector <FluidParticle> allParticles;

	bool explode=false;

	std::vector <FluidParticle *> FindProximityParticles(YsVec3 fromPos,double h);
	std::vector <const FluidParticle *> FindProximityParticles(YsVec3 fromPos,double h) const;

	void UpdateDensity(int i0, int i1);
	void CalculateForce(int i0, int i1);
	void BounceOnWall(void);
	void Move(int i0, int i1, double dt);

	void MakeTestCase(void);
};

// Will be done with a lattice
std::vector <FluidParticle *> FluidSimulator::FindProximityParticles(YsVec3 fromPos,double h)
{
	std::vector <FluidParticle *> pp;
	double hh=h*h;


	for(auto &part : allParticles)
	{
		auto dd=(fromPos-part.pos).GetSquareLength();
		if(dd<hh)
		{
			pp.push_back(&part);
		}
	}
	return pp;
}
// Will be done with a lattice
std::vector <const FluidParticle *> FluidSimulator::FindProximityParticles(YsVec3 fromPos,double h) const
{
	std::vector <const FluidParticle *> pp;
	double hh=h*h;
	for(auto &part : allParticles)
	{
		auto dd=(fromPos-part.pos).GetSquareLength();
		if(dd<hh)
		{
			pp.push_back(&part);
		}
	}
	return pp;
}

void FluidSimulator::UpdateDensity(int i0, int i1)
{
	for(int i = i0; i< i1; ++i)
	{
		auto &part = allParticles[i];
		double rho=0.0;
		for(auto prox : FindProximityParticles(part.pos,h))
		{
			// Make sure to include self.
			double r=(prox->pos-part.pos).GetLength();
			rho+=prox->m*Wpoly6::Evaluate(r,h);
		}
		part.rho=rho;
	}
}
void FluidSimulator::CalculateForce(int i0, int i1)
{
	for(int i = i0; i< i1; ++i)
	{
		auto &part = allParticles[i];
		YsVec3 n=YsVec3::Origin();
		double csLaplacian=0.0;

		YsVec3 f=YsVec3::Origin();
		for(auto prox : FindProximityParticles(part.pos,h))
		{
			if(prox!=&part)
			{
				f+=PressureAndViscosity(part,*prox,k,h,rho0,mu);

				// For surface tension
				double r=(part.pos-prox->pos).GetLength();
				if(YsTolerance<r)
				{
					double deltaCS=prox->m*Wpoly6::Delta(r,h)/prox->rho;
					auto vec=prox->pos-part.pos;
					vec/=r;
					n+=vec*deltaCS;

					csLaplacian+=prox->m*Wpoly6::Laplacian(r,h)/prox->rho;
				}
			}
		}
		f+=WallForce(part);
		part.f=f;

		double Ln=n.GetLength();
		if(0.5<Ln)
		{
			n/=Ln;
			f-=sigma*csLaplacian*n;
			part.nom=n;
		}

		if(true==twoDimension)
		{
			part.f.SetZ(0.0);
		}

	}
}

void FluidSimulator::BounceOnWall(void)
{
	for(auto &part : allParticles)
	{
		if(part.pos.x()<minX)
		{
			part.pos.SetX(minX);
			if(part.vel.x()<0.0)
			{
				part.vel.MulX(-1.0);
			}
		}
		if(maxX<part.pos.x())
		{
			part.pos.SetX(maxX);
			if(0.0<part.vel.x())
			{
				part.vel.MulX(-1.0);
			}
		}
		if(part.pos.y()<0.0)
		{
			part.pos.SetY(0.0);
			if(part.vel.y()<0.0)
			{
				part.vel.MulY(-1.0);
			}
		}
	}
}

void FluidSimulator::Move(int i0, int i1, double dt)
{
	for(int i = i0; i< i1; ++i)
	{
		auto &part = allParticles[i];
		part.pos+=part.vel*dt;
		part.vel+=(part.f/part.m)*dt;
		part.vel.SubY(gravity*dt);

		if(true==twoDimension)
		{
			part.pos.SetZ(0.0);
			part.vel.SetZ(0.0);
		}
	}
	BounceOnWall();

	if(explode)
	{
		for(auto &part : allParticles)
		{
			YsVec3 base(0.0,4.0,0.0);
			auto vel=part.pos-base;
			if(vel.GetLength()<3.0)
			{
				part.vel.Set(0.0,-40.0,0.0);
			}
		}
	}
}

void FluidSimulator::MakeTestCase(void)
{
	double y0=5.0;
	double dx=1.2,dy=0.6;
	int i=0;
	for(double y=y0; y<=y0+100.0*dy; y+=dy)                            /////////////////////////////////// 4.2 Expand the Field and Increase the Particle Count
	{
		double shift=dx/2.0*(double)(i&1);
		for(double x=minX; x<=maxX; x+=dx)
		{
			FluidParticle p;
			p.pos.Set(x+shift,y,0.0);
			p.vel.SetX(5.0);
			allParticles.push_back(p);
		}
		++i;
	}
	UpdateDensity(0, allParticles.size());
	CalculateForce(0, allParticles.size());
}

int main(void)
{
	FsOpenWindow(0,0,800,600,1);

	srand(time(nullptr));

	FluidSimulator sim;
	sim.MakeTestCase();

	const int nThreads = 8; ///////////////////////////////////////////////////////

	int ctr=0;
	int fps = 0, millisecAccum = 0;
	FsPassedTime();
	for(;;)
	{
		FsPollDevice();
		auto key=FsInkey();
		if(FSKEY_ESC==key)
		{
			break;
		}

		auto millisec = FsPassedTime();
		double dt = (double)millisec / 1000.0;

		++fps;
		millisecAccum += millisec;

		if (1000 < millisecAccum)
		{
			printf("%d fps\n", fps);
			fps = 0;
			millisecAccum = 0;
		}

		sim.explode=(0!=FsGetKeyState(FSKEY_X));

		if(0!=FsGetKeyState(FSKEY_B))
		{
			for(auto &part : sim.allParticles)
			{
				part.vel*=0.99;
			}
		}


		WorkerThread subThr[nThreads];
		for(int i=0; i<nThreads; ++i)
			{
				subThr[i].Run([&,i]{sim.UpdateDensity(i* (sim.allParticles).size() /nThreads, (i+1)* (sim.allParticles).size() /nThreads);});
			}
		for(int i=0; i<nThreads; ++i)
			{
				subThr[i].Wait();
			}

		for(int i=0; i<nThreads; ++i)
			{
				subThr[i].Run([&,i]{sim.CalculateForce(i* (sim.allParticles).size() /nThreads, (i+1)* (sim.allParticles).size() /nThreads);});
			}
		for(int i=0; i<nThreads; ++i)
			{
				subThr[i].Wait();
			}

		//sim.Move(0.01);
		for(int i=0; i<nThreads; ++i)
			{
				subThr[i].Run([&,i]{sim.Move(i* (sim.allParticles).size() /nThreads, (i+1)* (sim.allParticles).size() /nThreads, 0.01);});
			}
		for(int i=0; i<nThreads; ++i)
			{
				subThr[i].Wait();
			}

		

		glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

		const double scale=20.0;
		double X0=400.0;
		double Y0=550.0;

		glPointSize(3);
		glBegin(GL_POINTS);
		for(auto &p : sim.allParticles)
		{
			glVertex2i(X0+p.pos.x()*scale,Y0-p.pos.y()*scale);
		}
		glEnd();

		FsSwapBuffers();
	}
}
