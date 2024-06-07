#pragma once

#include "AppDebug.hpp"

class AppMain
{

public:
	static AppMain& GetInstance(void)
	{
		static AppMain instance;
		return instance;
	}

	void MainLoop(void);

};
