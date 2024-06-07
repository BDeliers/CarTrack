#pragma once

#include "AppDebug.hpp"

/// @brief Main application class
class AppMain
{

public:
	/// @brief 	Singleton
	/// @return Reference to the AppMain singleton
	static AppMain& GetInstance(void)
	{
		static AppMain instance;
		return instance;
	}

	/// @brief Permanent loop
	void MainLoop(void);

	/// @brief 		Push a character to the main logger. Used to redirect from printf
	/// @param c 	The character to push
	/// @return 	True if success
	bool PushToLogger(char c)
	{
		return app_debug.Push(c);
	}

private:
	AppDebug app_debug;

};
