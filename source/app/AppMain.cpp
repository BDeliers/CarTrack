#include "AppMain.hpp"
#include "AppCore.hpp"

void AppMain::MainLoop(void)
{
	app_debug.Init();

	log_trace("Power up");
	app_debug.Flush();
	
	if (app_modem.Init())
	{
		if (app_modem.EnableGnss(true))
		{
			modem_init_success = true;
		}
	}

	if (!modem_init_success)
	{
		log_warn("Modem init failed");
	}

	for (;;)
	{
		app_debug.Flush();

		if (modem_init_success)
		{
			app_modem.RetrieveGnssData(last_gps_info);
		}

		AppCore_BlockingDelayMs(1000);
	}
}

extern "C"
{

int __io_putchar(int ch)
{
    return (AppMain::GetInstance().PushToLogger(ch) ? ch : -1);
}

}
