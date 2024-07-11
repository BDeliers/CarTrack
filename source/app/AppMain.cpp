#include "AppMain.hpp"
#include "AppCore.hpp"

void AppMain::MainLoop(void)
{
	char ip[17];

	app_debug.Init();

	log_trace("Power up");
	app_debug.Flush();
	
	if (app_modem.Init())
	{
		if (app_modem.EnableGnss(true)
			&& app_modem.SetPhoneMode(true)
			&& app_modem.CheckSimReady()
			&& app_modem.ConfigureLte())
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

			if (!modem_network_attached)
			{
				modem_network_attached = app_modem.CheckNetworkRegistration();
			}

			if (modem_network_attached && !modem_network_configured)
			{
				modem_network_configured = app_modem.EnableGprs(true, "\"iot.1nce.net\"", "\"\"", "\"\"");

				if (modem_network_configured)
				{
					app_modem.EnableAppNetwork(true);
				}
			}

			app_modem.CheckSignalQuality();

			if (modem_network_configured && !got_ip)
			{
				if (app_modem.GetIp(ip))
				{
					if (strcmp(ip, "0.0.0.0") != 0)
					{
						log_info("IP address assigned %s", ip);
						got_ip = true;
					}
				}
			}

			if (got_ip)
			{
				// Main app will go here
			}
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
