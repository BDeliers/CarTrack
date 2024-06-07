#include "AppMain.hpp"

void AppMain::MainLoop(void)
{
	AppDebug::GetInstance().Init();

	log_trace("Power up");

	for (;;)
	{
		AppDebug::GetInstance().Flush();
	}
}
