#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <ysglfontdata.h>
#include <fssimplewindow.h>

#include "bintree.h"

class ApplicationMain
{
protected:
	bool term=false;
	BinaryTree <int,int> tree;
	BinaryTree <int,int>::NodeHandle mouseOn;

public:
	ApplicationMain();
	bool MustTerminate(void) const;
	void RunOneStep(void);
	void Draw(void) const;

	void DrawNode(int x0,int x1,int y0,int yStep,int prevX,int prevY,BinaryTree<int,int>::NodeHandle ndHd) const;
	BinaryTree <int,int>::NodeHandle FindNodeFromMouseCoord(int mx,int my) const;
	BinaryTree <int,int>::NodeHandle FindNodeFromMouseCoord(int mx,int my,int x0,int x1,int y0,int yStep,BinaryTree<int,int>::NodeHandle ndHd) const;
};

ApplicationMain::ApplicationMain()
{
	mouseOn.Nullify();
	srand((int)time(nullptr));

	/// <autoRebalancing implementation: Question 5.5>
	tree.autoRebalancing = true;

	for(int i=0; i<50; ++i)
	{
		tree.Insert(rand()%100,0);
	}
}
bool ApplicationMain::MustTerminate(void) const
{
	return term;
}
void ApplicationMain::RunOneStep(void)
{
	FsPollDevice();
	auto key=FsInkey();
	if(FSKEY_ESC==key)
	{
		term=true;
	}
	if(FSKEY_DEL==key)
	{
		tree.Delete(mouseOn);
	}
	else if(FSKEY_INS==key)
	{
		tree.Insert(rand()%100,0);
	}
	else if (FSKEY_SPACE == key) /// <random number insertion: Question 5.6>
	{
		tree.Insert(rand() % 100, 0);
	}
	else if(FSKEY_L==key)
	{
		tree.RotateLeft(mouseOn);
	}

	int lb,mb,rb,mx,my;
	auto evt=FsGetMouseEvent(lb,mb,rb,mx,my);
	auto pick=FindNodeFromMouseCoord(mx,my);
	if(pick!=mouseOn)
	{
		mouseOn=pick;
	}
}
void ApplicationMain::Draw(void) const
{
	int wid,hei;
	FsGetWindowSize(wid,hei);

	glViewport(0,0,wid,hei);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,(float)wid-1,(float)hei-1,0,-1,1);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	DrawNode(0,wid,0,40, wid/2,20, tree.RootNode());
	FsSwapBuffers();
}

void ApplicationMain::DrawNode(int x0,int x1,int y0,int yStep,int prevX,int prevY,BinaryTree<int,int>::NodeHandle ndHd) const
{
	if(ndHd.IsNotNull())
	{
		auto size_left = tree.GetHeight(tree.Left(ndHd));
		auto size_right = tree.GetHeight(tree.Right(ndHd));

		int x=(x0+x1)/2,y=y0+yStep/2;
		if(ndHd==mouseOn)
		{
			glColor3ub(0,255,0); // Changed from Red -> Green for better clarity - purely cosmetic change
		}
		else
		{
			if ((size_right - size_left) > 0)
			{
				glColor3ub(255, 0, 0);
			}
			else if ((size_left - size_right) > 0)
			{
				glColor3ub(0, 0, 255);
			}
			else
			{
				glColor3ub(0, 0, 0);
			}
		}
		glBegin(GL_LINES);
		glVertex2i(prevX,prevY);
		glVertex2i(x,y);
		glEnd();

		glRasterPos2i(x,y);
		char str[256];
		sprintf(str,"%d(%d)",tree.GetKey(ndHd),tree.GetHeight(ndHd));
		YsGlDrawFontBitmap12x16(str);

		DrawNode(x0,(x0+x1)/2,y0+yStep,yStep,x,y,tree.Left(ndHd));
		DrawNode((x0+x1)/2,x1,y0+yStep,yStep,x,y,tree.Right(ndHd));
	}
}

BinaryTree <int,int>::NodeHandle ApplicationMain::FindNodeFromMouseCoord(int mx,int my) const
{
	int wid,hei;
	FsGetWindowSize(wid,hei);
	return FindNodeFromMouseCoord(mx,my,0,wid,0,40, tree.RootNode());
}

BinaryTree <int,int>::NodeHandle ApplicationMain::FindNodeFromMouseCoord(
    int mx,int my,int x0,int x1,int y0,int yStep,BinaryTree<int,int>::NodeHandle ndHd) const
{
	if(ndHd.IsNotNull())
	{
		if(x0<=mx && mx<x1 && y0<=my && my<y0+yStep)
		{
			return ndHd;
		}
		auto fromLeft=FindNodeFromMouseCoord(mx,my,x0,(x0+x1)/2,y0+yStep,yStep,tree.Left(ndHd));
		if(fromLeft.IsNotNull())
		{
			return fromLeft;
		}
		auto fromRight=FindNodeFromMouseCoord(mx,my,(x0+x1)/2,x1,y0+yStep,yStep,tree.Right(ndHd));
		if(fromRight.IsNotNull())
		{
			return fromRight;
		}
	}
	return tree.Null();
}

int main(void)
{
	FsOpenWindow(0,0,800,600,1);
	ApplicationMain app;
	while(true!=app.MustTerminate())
	{
		app.RunOneStep();
		app.Draw();
	}
	return 0;
}
