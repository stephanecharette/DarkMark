/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include "DarkMark.hpp"


void DarkMark_Juce_Crash_Handler(void *ptr)
{
	dm::Log("crash handler invoked -- exiting");

	exit(1);
}


void DarkMark_CPlusPlus_Terminate_Handler(void)
{
	dm::Log("terminate handler invoked");

	exit(2);
}


void DarkMark_CPlusPlus_Unexpected_Handler(void)
{
	dm::Log("unexpected handler invoked");

	exit(3);
}


dm::DarkMarkApplication::DarkMarkApplication(void)
{
	return;
}


dm::DarkMarkApplication::~DarkMarkApplication(void)
{
	return;
}


void dm::DarkMarkApplication::initialise(const String & commandLine)
{
	// This method is where you should put your application's initialisation code.

	std::set_terminate(DarkMark_CPlusPlus_Terminate_Handler);
	std::set_unexpected(DarkMark_CPlusPlus_Unexpected_Handler);
	SystemStats::setApplicationCrashHandler(DarkMark_Juce_Crash_Handler);

	dm::Log("starting DarkMark v" DARKMARK_VERSION);

	std::srand(std::time(nullptr));

	wnd.reset(new DMWnd);

	return;
}


void dm::DarkMarkApplication::shutdown()
{
	// shutdown the application
	dm::Log("shutting down DarkMark v" DARKMARK_VERSION);

	return;
}


void dm::DarkMarkApplication::anotherInstanceStarted(const String & commandLine)
{
	// When another instance of the app is launched while this one is running,
	// this method is invoked, and the commandLine parameter tells you what
	// the other instance's command-line arguments were.

	return;
}


void dm::DarkMarkApplication::unhandledException(const std::exception * e, const String & sourceFilename, int lineNumber)
{
	std::string str = sourceFilename.toStdString();
	str += " #" + std::to_string(lineNumber);
	if (e != nullptr)
	{
		str += ": ";
		str += e->what();
	}
	dm::Log(str);

	return;
}
