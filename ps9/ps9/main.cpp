#include <stdio.h>
#include <math.h>
#include <vector>
#include <fssimplewindow.h>
#include <ysgl.h>


#include "renderer.h"
#include "glutil.h"


class ApplicationMain
{
protected:
	bool term=false;

	BezierRenderer bezier;

	YsMatrix4x4 Rc;
	double d;
	YsVec3 t;

	YsVec3 ctp[16];	// Cubic Bezier Patch control points.
	// ctp[0] ---- ctp[1] ---- ctp[2] ---- ctp[3]
	//   |           |           |           |
	//   |           |           |           |
	// ctp[4] ---- ctp[5] ---- ctp[6] ---- ctp[7]
	//   |           |           |           |
	//   |           |           |           |
	// ctp[8] ---- ctp[9] ---- ctp[10] --- ctp[11]
	//   |           |           |           |
	//   |           |           |           |
	// ctp[12] --- ctp[13] --- ctp[14] --- ctp[15]


	YsVec3 CubicBezierCurve(const YsVec3 ctp[4],double s) const;
	YsVec3 CubicBezierSurface(const YsVec3 ctp[16],double s,double t) const;
	void GetCubicBezierBoundingBox(const YsVec3 ctp[16],YsVec3 bbx[2]) const;

	std::vector <float> vtx,nom,col;
	YsVec3 bbx[2];

	void RemakeVertexArray(void);
	YsMatrix4x4 GetProjection(void) const;
	YsMatrix4x4 GetModelView(void) const;


public:
	ApplicationMain();
	void RunOneStep(void);
	void Draw(void);
	bool MustTerminate(void) const;
};

YsVec3 ApplicationMain::CubicBezierCurve(const YsVec3 ctp[4],double s) const
{
	// Recursive form of Cubic Bezier Curve.
	// (1) Interpolate ctp[0-1], ctp[1-2], and ctp[2-3] with parameter s -> a[0], a[1], a[2]
	// (2) Interpolate a[0-1], a[1-2] with parameter s -> b[0], b[1]
	// (3) Interpolate b[0-1]
	// Easy to remember.  You don't have to remember the Bezier parameter equation.
	const YsVec3 a[3]=
	{
		ctp[0]*(1.0-s)+ctp[1]*s,
		ctp[1]*(1.0-s)+ctp[2]*s,
		ctp[2]*(1.0-s)+ctp[3]*s,
	};
	const YsVec3 b[2]=
	{
		a[0]*(1.0-s)+a[1]*s,
		a[1]*(1.0-s)+a[2]*s,
	};
	return b[0]*(1.0-s)+b[1]*s;
}

YsVec3 ApplicationMain::CubicBezierSurface(const YsVec3 ctp[16],double s,double t) const
{
	// Calculating a point on a Cubic Bezier Patch.
	// (1) First, calculate a point on four Bezier curves defined by ctp[0-3], [4-7], [8-11], [12-15], and parameter s -> a[0], a[1], a[2], a[3]
	// (2) Calculate on a point on the Bezier curve defined by a[0-4] and parameter t.
	// Easy to remember.  You don't have to remember the Bezier parameter equation.
	const YsVec3 a[4]=
	{
		CubicBezierCurve(ctp   ,s),
		CubicBezierCurve(ctp +4,s),
		CubicBezierCurve(ctp +8,s),
		CubicBezierCurve(ctp+12,s),
	};
	return CubicBezierCurve(a,t);
}

void ApplicationMain::GetCubicBezierBoundingBox(const YsVec3 ctp[16],YsVec3 bbx[2]) const
{
	YsBoundingBoxMaker3 mkBbx;
	for(int i=0; i<=100; i+=5)
	{
		const double s=(double)i/100.0;
		for(int j=0; j<=100; j+=5)
		{
			const double t=(double)j/100.0;
			mkBbx.Add(CubicBezierSurface(ctp,s,t));
		}
	}
	mkBbx.Get(bbx);
}

void ApplicationMain::RemakeVertexArray(void)
{
	vtx.clear();
	nom.clear();
	col.clear();

	for(int i=0; i<100; i+=5)
	{
		const double s0=(double)i/100.0,s1=(double)(i+5)/100.0;
		for(int j=0; j<100; j+=5)
		{
			const double t0=(double)j/100.0,t1=(double)(j+5)/100.0;
			const double triParam[2][6]=
			{
				{s0,t0,s1,t0,s0,t1},
				{s1,t0,s1,t1,s0,t1}
			};

			for(int k=0; k<2; ++k)
			{
				const YsVec3 triVtPos[3]=
				{
					CubicBezierSurface(ctp,triParam[k][0],triParam[k][1]),
					CubicBezierSurface(ctp,triParam[k][2],triParam[k][3]),
					CubicBezierSurface(ctp,triParam[k][4],triParam[k][5]),
				};
				auto plNom=YsGetAverageNormalVector(3,triVtPos);
				for(auto vtPos : triVtPos)
				{
					vtx.push_back(vtPos.xf());
					vtx.push_back(vtPos.yf());
					vtx.push_back(vtPos.zf());
					nom.push_back(plNom.xf());
					nom.push_back(plNom.yf());
					nom.push_back(plNom.zf());
					col.push_back(0.0f);
					col.push_back(0.0f);
					col.push_back(1.0f);
					col.push_back(1.0f);
				}
			}
		}
	}
}

YsMatrix4x4 ApplicationMain::GetProjection(void) const
{
	int wid,hei;
	FsGetWindowSize(wid,hei);
	auto aspect=(double)wid/(double)hei;
	return MakePerspective(45.0,aspect,d/10.0,d*2.0);
}

YsMatrix4x4 ApplicationMain::GetModelView(void) const
{
	YsMatrix4x4 globalToCamera=Rc;
	globalToCamera.Invert();

	YsMatrix4x4 modelView;
	modelView.Translate(0,0,-d);
	modelView*=globalToCamera;
	modelView.Translate(-t);
	return modelView;
}

ApplicationMain::ApplicationMain()
{
	d=10.0;
	t=YsVec3::Origin();

	ctp[ 0].Set(0.0,0.0,3.0);
	ctp[ 1].Set(1.0,0.5,3.0);
	ctp[ 2].Set(2.0,0.5,3.0);
	ctp[ 3].Set(3.0,0.0,3.0);
	ctp[ 4].Set(0.0,0.0,2.0);
	ctp[ 5].Set(1.0,2.0,2.0);
	ctp[ 6].Set(2.0,2.0,2.0);
	ctp[ 7].Set(3.0,0.0,2.0);
	ctp[ 8].Set(0.0,0.0,1.0);
	ctp[ 9].Set(1.0,2.0,1.0);
	ctp[10].Set(2.0,2.0,1.0);
	ctp[11].Set(3.0,0.0,1.0);
	ctp[12].Set(0.0,0.0,0.0);
	ctp[13].Set(1.0,0.5,0.0);
	ctp[14].Set(2.0,0.5,0.0);
	ctp[15].Set(3.0,0.0,0.0);

	FsChangeToProgramDir();

	RemakeVertexArray();
	GetCubicBezierBoundingBox(ctp,bbx);

	t=(bbx[0]+bbx[1])/2.0;
	d=(bbx[1]-bbx[0]).GetLength()*1.2;

	printf("Target %s\n",t.Txt());
	printf("Diagonal %lf\n",d);

	bezier.CompileFile("bezier_vertexShader.glsl","bezier_fragmentShader.glsl");
}

void ApplicationMain::RunOneStep(void)
{
	auto currentTime = (double)FsSubSecondTimer();//FsPassedTime();
	currentTime = currentTime/1000.0;
	
	ctp[0].Set(0.0, 2.0 * sin(currentTime), 3.0);
	ctp[3].Set(3.0, 2.0 * sin(currentTime), 3.0);
	ctp[5].Set(1.0, (1.0 + cos(currentTime) * 2.0), 2.0);
	ctp[6].Set(2.0, (1.0 + cos(currentTime) * 2.0), 2.0);
	ctp[9].Set(1.0, (1.0 + cos(currentTime) * 2.0), 1.0);
	ctp[10].Set(2.0, (1.0 + cos(currentTime) * 2.0), 1.0);
	ctp[12].Set(0.0, 2.0 * sin(currentTime), 0.0);
	ctp[15].Set(3.0, 2.0 * sin(currentTime), 0.0);
	
	auto key=FsInkey();
	if(FSKEY_ESC==key)
	{
		term=true;
	}

	if(FsGetKeyState(FSKEY_LEFT))
	{
		Rc.RotateXZ(YsPi/60.0);
	}
	if(FsGetKeyState(FSKEY_RIGHT))
	{
		Rc.RotateXZ(-YsPi/60.0);
	}
	if(FsGetKeyState(FSKEY_UP))
	{
		Rc.RotateYZ(YsPi/60.0);
	}
	if(FsGetKeyState(FSKEY_DOWN))
	{
		Rc.RotateYZ(-YsPi/60.0);
	}

	FsSleep(10);
}
/* virtual */ void ApplicationMain::Draw(void)
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);


	int wid,hei;
	FsGetWindowSize(wid,hei);
	glViewport(0,0,wid,hei);



    glClear(GL_DEPTH_BUFFER_BIT);

	YsMatrix4x4 projection=GetProjection();
	GLfloat projectionGl[16];
	projection.GetOpenGlCompatibleMatrix(projectionGl);


	glUseProgram(bezier.programIdent);
	glUniformMatrix4fv(bezier.uniformProjectionPos,1,GL_FALSE,projectionGl);


	YsMatrix4x4 modelView=GetModelView();
	GLfloat modelViewGl[16];
	modelView.GetOpenGlCompatibleMatrix(modelViewGl);
	glUniformMatrix4fv(bezier.uniformModelViewPos,1,GL_FALSE,modelViewGl);


	std::vector <float> ctp;
	for(auto p : this->ctp)
	{
		ctp.push_back(p.xf());
		ctp.push_back(p.yf());
		ctp.push_back(p.zf());
	}
	glUniform3fv(bezier.uniformCtpPos,16,ctp.data());

	std::vector <float> param;
	for(int i=0; i<100; i+=5)
	{
		float s0=(float)i/100.0f,s1=(float)(i+5)/100.0f;
		for(int j=0; j<100; j+=5)
		{
			float t0=(float)j/100.0f,t1=(float)(j+5)/100.0f;
			param.push_back(s0);	param.push_back(t0);
			param.push_back(s1);	param.push_back(t0);
			param.push_back(s0);	param.push_back(t1);
			param.push_back(s1);	param.push_back(t0);
			param.push_back(s1);	param.push_back(t1);
			param.push_back(s0);	param.push_back(t1);
		}
	}

    glEnableVertexAttribArray(bezier.attribParamPos);
    glVertexAttribPointer(bezier.attribParamPos,2,GL_FLOAT,GL_FALSE,0,param.data());
    glDrawArrays(GL_TRIANGLES,0,param.size()/2);
    glDisableVertexAttribArray(bezier.attribParamPos);

    FsSwapBuffers();
}

bool ApplicationMain::MustTerminate(void) const
{
	return term;
}

int main(int argc,char *argv[])
{
	FsOpenWindow(0,0,800,600,1);
	ApplicationMain app;
	while(true!=app.MustTerminate())
	{
		FsPollDevice();
		app.RunOneStep();
		app.Draw();
	}
	return 0;
}
