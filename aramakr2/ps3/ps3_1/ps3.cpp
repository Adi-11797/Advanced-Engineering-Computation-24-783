#include "fssimplewindow.h"
#include "data.h"


class ApplicationMain
{
public:
	int terminate = 0;
	int userX = 434, userY = 112;

	ApplicationMain();
	bool MustTerminate(void) const;
	void RunOneStep(void);
	void Draw(void);
};

ApplicationMain::ApplicationMain()
{
}

bool ApplicationMain::MustTerminate(void) const
{
	return 0 != terminate;
}


void ApplicationMain::RunOneStep(void)
{
	FsPollDevice();

	auto key = FsInkey();

	switch (key)
	{
	case FSKEY_ESC:
	{
		terminate = 1;
	}
	break;
	case FSKEY_UP:
		userY -= 7;
		break;
	case FSKEY_DOWN:
		userY += 7;
		break;
	case FSKEY_LEFT:
		userX -= 7;
		if (0 > userX)
		{
			userX += 1008;
		}
		break;
	case FSKEY_RIGHT:
		userX += 7;
		if (1008 <= userX)
		{
			userX -= 1008;
		}
		break;
	case FSKEY_ENTER:
		userX = 0;
		userY = 0;
		break;
	}	
}

void ApplicationMain::Draw(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (int i = 0; i < (40); ++i)
	{
		for (int j = 0; j < (32); ++j)
		{
			int y_val = (userY*24)+j*24;

			glRasterPos2i(i * 24, j * 24);

			if ((y_val >= 0 && y_val < 24 * 42) || (y_val >= 24 * 462 && y_val < 24 * 504))
			{
				glDrawPixels(24, 24, GL_RGBA, GL_UNSIGNED_BYTE, tiles_arctic[world_map[(j + userY) * 1008 + (i + userX)]]);
			}

			if ((y_val >= 24 * 42 && y_val < 24 * 84) || (y_val >= 24 * 420 && y_val < 24 * 462))
			{
				glDrawPixels(24, 24, GL_RGBA, GL_UNSIGNED_BYTE, tiles_subArctic[world_map[(j + userY) * 1008 + (i + userX)]]);
			}

			if ((y_val >= 24 * 84 && y_val < 24 * 168) || (y_val >= 24 * 336 && y_val < 24 * 420))
			{
				glDrawPixels(24, 24, GL_RGBA, GL_UNSIGNED_BYTE, tiles_midLattitude[world_map[(j + userY) * 1008 + (i + userX)]]);
			}

			if ((y_val >= 24 * 168) && (y_val < 24 * 336))
			{
				glDrawPixels(24, 24, GL_RGBA, GL_UNSIGNED_BYTE, tiles_equator[world_map[(j + userY) * 1008 + (i + userX)]]);
			}
		}
	}

	for (int k = 0; k < 70; k++)
	{
		if ((port_locations[2*k] >= userX && port_locations[2*k] < userX + 24 * 40) && (port_locations[2*k+1] >= userY && port_locations[2*k+1] < userY + 24 * 32))
		{
			int X_p = port_locations[2*k] - userX;
			int Y_p = port_locations[2*k+1] - userY;

			glRasterPos2i(X_p * 24, Y_p * 24);
			glDrawPixels(24, 24, GL_RGBA, GL_UNSIGNED_BYTE, tile_port);
		}
	}

	FsSwapBuffers();

	FsSleep(10);
}


int main(void)
{
	FsOpenWindow(0, 0, 960, 768, 1);
	ApplicationMain app;

	while (true != app.MustTerminate())
	{
		FsPollDevice();
		app.RunOneStep();
		app.Draw();
	}
	return 0;
}



