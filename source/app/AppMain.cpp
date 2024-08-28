#include "AppMain.hpp"
#include "AppCore.hpp"

void AppMain::MainLoop(void)
{
	app_debug.Init();

	log_trace("Power up");
	app_debug.Flush();

	// Critical failure
	if (!app_main_state_machine.Init(app_debug))
	{
		for (;;)
		{
			app_debug.Flush();
			log_error("Modem init failed, can't continue");
			AppCore_BlockingDelayMs(1000);
		}
	}

	for (;;)
	{
		app_main_state_machine.Process();

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
