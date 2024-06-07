#include "AppMain.hpp"
#include "AppCore.hpp"

void AppMain::MainLoop(void)
{
	app_debug.Init();

	log_trace("Power up");

	for (;;)
	{
		app_debug.Flush();
	}
}

extern "C"
{

int __io_putchar(int ch)
{
    return (AppMain::GetInstance().PushToLogger(ch) ? ch : -1);
}

}
