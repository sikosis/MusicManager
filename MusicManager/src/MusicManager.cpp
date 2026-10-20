#include "MainWindow.h"

#include <Application.h>
#include <OS.h>

#include <cstdlib>

class MusicManagerApplication : public BApplication {
public:
	MusicManagerApplication();
	virtual ~MusicManagerApplication();
	virtual void ReadyToRun();
};


MusicManagerApplication::MusicManagerApplication()
	:
	BApplication("application/x-vnd.sikosis-MusicManager")
{
}
//---------------------------------------------------------------------------------------------------------------------------------//


MusicManagerApplication::~MusicManagerApplication()
{
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
MusicManagerApplication::ReadyToRun()
{
	MainWindow* window = new MainWindow();
	window->Show();
}
//---------------------------------------------------------------------------------------------------------------------------------//


int
main()
{
	srand((unsigned int)system_time());
	MusicManagerApplication application;
	application.Run();
	return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//
