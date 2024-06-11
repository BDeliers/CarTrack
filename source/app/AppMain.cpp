#include "AppMain.hpp"
#include "AppCore.hpp"

void AppMain::MainLoop(void)
{
	app_debug.Init();
	app_modem.Init();

	log_trace("Power up");
	
	if (!app_modem.DetectModem())
	{
		log_warn("Modem not detected");
	}

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
