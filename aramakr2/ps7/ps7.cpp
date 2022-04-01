#include <fssimplewindow.h>
#include "ysclass.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>
#include <algorithm>
#include <iostream>

// You can use fstream or stdio, whichever you like for writing a binary-stl.
// I have a feeling that it is easier to do with C standard library.

const double PI=3.14159276;

class ApplicationMain
{
private:
	bool term=false;

																										std::vector <float> vtx2d;	// Clicked points.
																										std::vector <float> col2d;	// Color for clicked points.  Make it all (0,0,0,1)

	double FOV=PI/6.0;  // 30 degrees

	YsMatrix3x3 viewRotation;
	YsVec3 viewTarget;
	double viewDistance=10.0;
	std::vector <float> vtx;	// Extruded Shape Triangles
	std::vector <float> col;
	std::vector <float> nom;

	decltype(std::chrono::high_resolution_clock::now()) lastT;
public:
	ApplicationMain();
	bool MustTerminate(void) const;
	void RunOneStep(void);

	// Fill inside Draw function so that it renders vertex arrays (vtx,col, and nom).
	void Draw(void) const;

	// Write this function.
	// This function must make vertex, color, and normal buffers of a shape
	// by extruding a cross section and store them in vtx, col, and nom
	// member variables.
	// The cross section is an equilateral polygon of the given radius with 
	// nDiv sides, and parallel to the XY plane.
	// The center of the cross section is on the Z axis.
	// The shape must take -height<=Z<=height.
	void MakeExtrusion(int nDiv,double radius,double height);

	// See Binary-STL Viewer example, and fill this function.
	void ResetViewDistance(void);

	// Write this function.
	// This function must save the extruded shape in "extruded.stl" in the current working directory.
	void SaveSTL(const char fileName[]) const;

	void CalculateNormal(void);
};

ApplicationMain::ApplicationMain()
{
	viewTarget=YsVec3::Origin();
	lastT=std::chrono::high_resolution_clock::now();
}
bool ApplicationMain::MustTerminate(void) const
{
	return term;
}
void ApplicationMain::RunOneStep(void)
{
	auto deltaT=std::chrono::high_resolution_clock::now()-lastT;
	auto deltaTinMS=std::chrono::duration_cast<std::chrono::microseconds>(deltaT).count();
	double dt=(double)deltaTinMS/1000000.0;
	lastT=std::chrono::high_resolution_clock::now();

	auto key=FsInkey();
	if(FSKEY_ESC==key)
	{
		term=true;
	}

	if(FsGetKeyState(FSKEY_LEFT))
	{
		YsMatrix3x3 rot;
		rot.RotateXZ(dt*PI/6);
		viewRotation=rot*viewRotation;
	}
	if(FsGetKeyState(FSKEY_RIGHT))
	{
		YsMatrix3x3 rot;
		rot.RotateXZ(-dt*PI/6);
		viewRotation=rot*viewRotation;
	}
	if(FsGetKeyState(FSKEY_UP))
	{
		YsMatrix3x3 rot;
		rot.RotateYZ(dt*PI/6);
		viewRotation=rot*viewRotation;
	}
	if(FsGetKeyState(FSKEY_DOWN))
	{
		YsMatrix3x3 rot;
		rot.RotateYZ(-dt*PI/6);
		viewRotation=rot*viewRotation;
	}
}
void ApplicationMain::Draw(void) const
{
	int wid,hei;
	FsGetWindowSize(wid,hei);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);


	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(FOV*180.0/PI,(double)wid/(double)hei,viewDistance/10.0,viewDistance*3.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	// This is the correct timing to set up a head light (a light moves with the view point) >>
	// Also the light needs to be normalized.
	GLfloat lightDir[]={0,1/sqrt(2.0f),1/sqrt(2.0f),0};
	glLightfv(GL_LIGHT0,GL_POSITION,lightDir);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	// This is the correct timing to set up a head light (a light moves with the view point) <<


	// Set up model-view transformation from viewDistance, viewRotation, and viewTarget-------------------------------------added-------------------------------------->>
	YsMatrix4x4 tfm;
	tfm.Translate(0.0, 0.0, -viewDistance);
	tfm *= viewRotation;
	tfm.Translate(-viewTarget);

	double glTfm[16];
	tfm.GetOpenGlCompatibleMatrix(glTfm);
	glMultMatrixd(glTfm);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glColorPointer(4, GL_FLOAT, 0, col.data());
	glVertexPointer(3, GL_FLOAT, 0, vtx.data());
	glNormalPointer(GL_FLOAT, 0, nom.data());
	glDrawArrays(GL_TRIANGLES, 0, vtx.size() / 3);  // Divide by 3 because a 3D vector consists of 3 components.
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);


	glDisable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, wid, hei, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_FLOAT, 0, col2d.data());
	glVertexPointer(2, GL_FLOAT, 0, vtx2d.data());
	glDrawArrays(GL_LINE_STRIP, 0, vtx2d.size() / 2);  // Divide by 2 because a 2D vector consists of 2 components.
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	// 3D Rendering Here >>-----------------------------------------------------------------------------------------------------added!--------------------------------<<
	// 3D Rendering Here <<

	FsSwapBuffers();
}

void ApplicationMain::MakeExtrusion(int nDiv,double radius,double height)
{
	// This function must make vertex, color, and normal buffers of a shape
	// by extruding a cross section and store them in vtx, col, and nom
	// member variables.
	// The cross section is an equilateral polygon of the given radius with 
	// nDiv sides, and parallel to the XY plane.
	// The center of the cross section is on the Z axis.
	// The shape must take -height<=Z<=height.
	//-------------------------------------------------------------------------------------------added>>--------------------------------------------------------->>
	int nPlg = vtx2d.size() / 2;
	float cenX = 0, cenY = 0;
	for (int i = 0; i + 1 < vtx2d.size(); i += 2)
	{
		cenX += vtx2d[i];
		cenY += -vtx2d[i + 1];
	}
	cenX /= (float)nPlg;
	cenY /= (float)nPlg;


	vtx.clear();
	col.clear();
	for (int i = 0; i + 1 < vtx2d.size(); i += 2)
	{
		float x0 = vtx2d[i];
		float y0 = -vtx2d[i + 1];
		float x1 = vtx2d[(i + 2) % vtx2d.size()];
		float y1 = -vtx2d[(i + 3) % vtx2d.size()];

		// Front-side Triangle
		vtx.push_back(x0);  vtx.push_back(y0);  vtx.push_back(-100.0f);
		vtx.push_back(x1);  vtx.push_back(y1);  vtx.push_back(-100.0f);
		vtx.push_back(cenX); vtx.push_back(cenY); vtx.push_back(-100.0f);
		col.push_back(0); col.push_back(0); col.push_back(1); col.push_back(1);
		col.push_back(0); col.push_back(0); col.push_back(1); col.push_back(1);
		col.push_back(0); col.push_back(0); col.push_back(1); col.push_back(1);

		// Back-side Triangle
		vtx.push_back(x1);  vtx.push_back(y1);  vtx.push_back(100.0f);
		vtx.push_back(x0);  vtx.push_back(y0);  vtx.push_back(100.0f);
		vtx.push_back(cenX); vtx.push_back(cenY); vtx.push_back(100.0f);
		col.push_back(0); col.push_back(0); col.push_back(1); col.push_back(1);
		col.push_back(0); col.push_back(0); col.push_back(1); col.push_back(1);
		col.push_back(0); col.push_back(0); col.push_back(1); col.push_back(1);

		// Two Side Triangles
		vtx.push_back(x1);  vtx.push_back(y1);  vtx.push_back(-100.0f);
		vtx.push_back(x0);  vtx.push_back(y0);  vtx.push_back(-100.0f);
		vtx.push_back(x0);  vtx.push_back(y0);  vtx.push_back(100.0f);
		col.push_back(0); col.push_back(1); col.push_back(0); col.push_back(1);
		col.push_back(0); col.push_back(1); col.push_back(0); col.push_back(1);
		col.push_back(0); col.push_back(1); col.push_back(0); col.push_back(1);

		vtx.push_back(x0);  vtx.push_back(y0);  vtx.push_back(100.0f);
		vtx.push_back(x1);  vtx.push_back(y1);  vtx.push_back(100.0f);
		vtx.push_back(x1);  vtx.push_back(y1);  vtx.push_back(-100.0f);
		col.push_back(0); col.push_back(1); col.push_back(0); col.push_back(1);
		col.push_back(0); col.push_back(1); col.push_back(0); col.push_back(1);
		col.push_back(0); col.push_back(1); col.push_back(0); col.push_back(1);
		//------------------------------------------------------------------------------------------<<added----------------------------------------------------------<<
}
void ApplicationMain::ResetViewDistance(void)
{
	// Calculate bounding box.
	// Calculate diagonal length of the bounding box.
	// Make view distance diagonal times 0.5 divided by sin(0.5*FOV)
	// FOV stands for Field of View, and is given as member variable FOV.
	//-----------------------------------------------------------------------------------------------added>>------------------------------------------------------------>>
	if (3 <= vtx.size())
	{
		float minx = vtx[0], miny = vtx[1], minz = vtx[2];
		float maxx = vtx[0], maxy = vtx[1], maxz = vtx[2];
		for (int i = 0; i + 2 < vtx.size(); i += 3)
		{
			minx = std::min(minx, vtx[i]);
			miny = std::min(miny, vtx[i + 1]);
			minz = std::min(minz, vtx[i + 2]);
			maxx = std::max(maxx, vtx[i]);
			maxy = std::max(maxy, vtx[i + 1]);
			maxz = std::max(maxz, vtx[i + 2]);
		}
		viewTarget.Set((minx + maxx) / 2.0f, (miny + maxy) / 2.0f, (minz + maxz) / 2.0f);

		double dx = maxx - minx;
		double dy = maxy - miny;
		double dz = maxz - minz;
		double d = sqrt(dx * dx + dy * dy + dz * dz);
		viewDistance = (d / 2.0) / sin(FOV / 2);
		//-----------------------------------------------------------------------------------------------<<added------------------------------------------------------------<<
	}

}
void ApplicationMain::SaveSTL(const char fName[]) const
{
	// Save the shape as a binary STL file.
	//-----------------------------------------------------------------------------------------------added>>----------------------------------------------------------------->>
	unsigned char buffer[80];
	for (auto& b : buffer)
	{
		b = 0;
	}

	FILE* fp = fopen("extruded.stl", "wb");
	if (nullptr != fp)
	{
		fwrite(buffer, 1, 80, fp); // 80 bytes.  Comment

		unsigned int nTri = vtx.size() / 9;
		fwrite(&nTri, 1, 4, fp);

		for (unsigned int i = 0; i < nTri; ++i)
		{
			*(float*)(buffer) = nom[i * 9];
			*(float*)(buffer + 4) = nom[i * 9 + 1];
			*(float*)(buffer + 8) = nom[i * 9 + 2];

			*(float*)(buffer + 12) = vtx[i * 9];
			*(float*)(buffer + 16) = vtx[i * 9 + 1];
			*(float*)(buffer + 20) = vtx[i * 9 + 2];
			*(float*)(buffer + 24) = vtx[i * 9 + 3];
			*(float*)(buffer + 28) = vtx[i * 9 + 4];
			*(float*)(buffer + 32) = vtx[i * 9 + 5];
			*(float*)(buffer + 36) = vtx[i * 9 + 6];
			*(float*)(buffer + 40) = vtx[i * 9 + 7];
			*(float*)(buffer + 44) = vtx[i * 9 + 8];

			buffer[48] = 0;
			buffer[49] = 0;
			fwrite(buffer, 1, 50, fp);
		}

		fclose(fp);
	}
	//-----------------------------------------------------------------------------------------------added<<-----------------------------------------------------------------<<
}

int main(int ac,char *av[])
{
	if(ac<4)
	{
		std::cout << "Usage: ps6 nDiv radius height" << std::endl;
		return 1;
	}

	FsOpenWindow(0,0,800,600,1);
	ApplicationMain app;

	int nDiv=atoi(av[1]);
	double rad=atof(av[2]);
	double h=atof(av[3]);
	app.MakeExtrusion(nDiv,rad,h);
	app.ResetViewDistanceAndTarget();
	app.SaveSTL("output.stl");

	while(true!=app.MustTerminate())
	{
		FsPollDevice();
		app.RunOneStep();
		app.Draw();
	}
	return 0;
}
