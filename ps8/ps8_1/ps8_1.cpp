#include <stdio.h>
#include <math.h>
#include <iostream>
#include <chrono>
#include "fssimplewindow.h"
#include "mesh.h"
#include "glutil.h"


const double PI=3.1415927;

double getAngle(YsVec3 a, YsVec3 b)
{
	auto dot_prod = a*b;

	double len_v1 = sqrt(double(pow(a[0],2))+double(pow(a[1],2))+double(pow(a[2],2)));
	double len_v2 = sqrt(double(pow(b[0],2))+double(pow(b[1],2))+double(pow(b[2],2)));

	auto angle = acos(float(dot_prod/(len_v1 * len_v2)));
	//angle = angle*(180/PI) //converting to degrees from radians
	return angle;

}

void AddXYZ(std::vector <GLfloat> &vtx,double x,double y,double z)
{
	vtx.push_back((GLfloat)x);
	vtx.push_back((GLfloat)y);
	vtx.push_back((GLfloat)z);
}
void AddRGBA(std::vector <GLfloat> &col,double r,double g,double b,double a)
{
	col.push_back((GLfloat)r);
	col.push_back((GLfloat)g);
	col.push_back((GLfloat)b);
	col.push_back((GLfloat)a);
}

void RemakeVertexArray(
    std::vector <GLfloat> &vtx,
    std::vector <GLfloat> &col,
    std::vector <GLfloat> &nom,
    const PolygonalMesh &mesh)
{
	vtx.clear();
	nom.clear();
	col.clear();
	for(auto plHd=mesh.FindFirstPolygon(); mesh.NullPolygon()!=plHd; plHd=mesh.FindNextPolygon(plHd))
	{
		YsVec3 n=mesh.GetNormal(plHd);
		auto c=mesh.GetColor(plHd);
		auto plVtHd=mesh.GetPolygonVertex(plHd);
		if(3==plVtHd.size())
		{
			for(int i=0; i<3; ++i)
			{
				YsVec3 pos=mesh.GetVertexPosition(plVtHd[i]);
				AddXYZ(vtx,pos.x(),pos.y(),pos.z());
				AddXYZ(nom,n.x(),n.y(),n.z());
				AddRGBA(col,c.Rd(),c.Gd(),c.Bd(),1.0);
			}
		}
	}
}

void MakeSmallDihedralAngleHighlight(
    std::vector <GLfloat> &vtx,
    std::vector <GLfloat> &col,
    const PolygonalMesh &mesh)
{
	// Write this function.

	for(auto plHd=mesh.FindFirstPolygon(); mesh.NullPolygon()!=plHd; plHd=mesh.FindNextPolygon(plHd))
	{
		
		auto my_normal = mesh.GetNormal(plHd);

		auto vtHd = mesh.GetPolygonVertex(plHd);
		int n = vtHd.size();

		for(int i = 0; i < n; ++i)
		{
			auto plHdNeighb = mesh.GetNeighborPolygon(plHd, i);
			YsVec3 neighb_normal = mesh.GetNormal(plHdNeighb);
			
			auto norm_angle = getAngle(my_normal, neighb_normal);

			if(norm_angle > (60*(PI/180)))
			{

				YsVec3 vt_my = mesh.GetVertexPosition(vtHd[i]);
				YsVec3 vt_neighb = mesh.GetVertexPosition(vtHd[(i+1) % n]);

				AddXYZ(vtx,vt_my.x(),vt_my.y(),vt_my.z());
				AddRGBA(col, 0.0, 0.0, 1.0, 1.0);

				AddXYZ(vtx,vt_neighb.x(),vt_neighb.y(),vt_neighb.z());
				AddRGBA(col, 0.0, 0.0, 1.0, 1.0);

			}
		}
	}
		// Write this function.--------------------------------------------------------added---------------------------------------->>	
}

void MouseToLine(YsVec3 &p1,YsVec3 &p2,int mx,int my,const YsMatrix4x4 &proj,const YsMatrix4x4 &modelView)
{
	int wid,hei;
	FsGetWindowSize(wid,hei);

	p1=WindowToViewPort(wid,hei,mx,my);
	p2=p1;
	p1.SetZ(-1.0);
	p2.SetZ(1.0);

	proj.MulInverse(p1,p1,1.0);
	modelView.MulInverse(p1,p1,1.0);
	proj.MulInverse(p2,p2,1.0);
	modelView.MulInverse(p2,p2,1.0);
}

int main(int argc,char *argv[])
{
	PolygonalMesh mesh;
	if(2<=argc)
	{
		auto t0=std::chrono::high_resolution_clock::now();

		if(true==mesh.LoadSTL(argv[1]))
		{
			printf("%d Vertices %d Polygons\n",(int)mesh.GetNumVertex(),(int)mesh.GetNumPolygon());
		}
		else
		{
			printf("Could not open STL\n");
			return 1;
		}

		auto dt=std::chrono::high_resolution_clock::now()-t0;
		unsigned int tick=std::chrono::duration_cast<std::chrono::microseconds>(dt).count();
		printf("%d us\n",tick);
	}

	FsOpenWindow(0,0,800,600,1);

	mesh.Stitch(10e-6);

	std::vector <GLfloat> vtx,col,nom;
	RemakeVertexArray(vtx,col,nom,mesh);

	std::vector <GLfloat> edgVtx,edgCol;
	MakeSmallDihedralAngleHighlight(edgVtx,edgCol,mesh);

	YsVec3 minmax[2];
	mesh.GetBoundingBox(minmax);

	bool term=false;

	YsVec3 viewTarget(0.0,0.0,0.0);
	double viewDistance=10.0;
	double FOV=PI/4.0;
	YsMatrix4x4 viewOrientation;

	viewTarget.Set((minmax[0][0]+minmax[1][0])/2.0,
	               (minmax[0][1]+minmax[1][1])/2.0,
	               (minmax[0][2]+minmax[1][2])/2.0);

	double dx,dy,dz;
	dx=minmax[1][0]-minmax[0][0];
	dy=minmax[1][1]-minmax[0][1];
	dz=minmax[1][2]-minmax[0][2];
	double diagonal=sqrt(dx*dx+dy*dy+dz*dz);

	viewDistance=0.5*diagonal/sin(FOV/2.0);


	YsMatrix4x4 proj,modelView;

	while(true!=term)
	{
		FsPollDevice();

		int key=FsInkey();
		if(FSKEY_ESC==key)
		{
			term=true;
		}

		if(0!=FsGetKeyState(FSKEY_LEFT))
		{
			viewOrientation.RotateXZ(PI/60.0);
		}
		if(0!=FsGetKeyState(FSKEY_RIGHT))
		{
			viewOrientation.RotateXZ(-PI/60.0);
		}
		if(0!=FsGetKeyState(FSKEY_UP))
		{
			viewOrientation.RotateZY(PI/60.0);
		}
		if(0!=FsGetKeyState(FSKEY_DOWN))
		{
			viewOrientation.RotateZY(-PI/60.0);
		}



		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1,1);

		int wid,hei;
		FsGetWindowSize(wid,hei);
		double aspect=(double)wid/(double)hei;

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		proj=MakePerspective(FOV,aspect,viewDistance/10.0,viewDistance*10.0);
		double proj16[16];
		proj.GetOpenGlCompatibleMatrix(proj16);
		glMultMatrixd(proj16);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

	    GLfloat lightDir[]={0,1/sqrt(2.0f),1/sqrt(2.0f),0};
	    glLightfv(GL_LIGHT0,GL_POSITION,lightDir);
	    glEnable(GL_COLOR_MATERIAL);
	    glEnable(GL_LIGHTING);
	    glEnable(GL_LIGHT0);


		YsMatrix4x4 viewRotationInverse;

		modelView.LoadIdentity();
		modelView.Translate(0,0,-viewDistance);

		viewRotationInverse=viewOrientation;
		viewRotationInverse.Invert();
		modelView*=viewRotationInverse;

		modelView.Translate(-viewTarget);

		double modelViewGL[16];
		modelView.GetOpenGlCompatibleMatrix(modelViewGL);
		glMultMatrixd(modelViewGL);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glColorPointer(4,GL_FLOAT,0,col.data());
		glVertexPointer(3,GL_FLOAT,0,vtx.data());
		glNormalPointer(GL_FLOAT,0,nom.data());
		glDrawArrays(GL_TRIANGLES,0,vtx.size()/3);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);

		glDisable(GL_LIGHTING);
		glLineWidth(4);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4,GL_FLOAT,0,edgCol.data());
		glVertexPointer(3,GL_FLOAT,0,edgVtx.data());
		glDrawArrays(GL_LINES,0,edgVtx.size()/3);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);



		FsSwapBuffers();

		FsSleep(25);
	}

	return 0;
}
