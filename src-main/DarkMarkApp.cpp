/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include "DarkMark.hpp"
#include <csignal>
#include <cstring>
#include <cxxabi.h>
#include <execinfo.h>


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


std::string demangle_cpp(std::string name)
{
	int status = 0;
	char *demangled = abi::__cxa_demangle( name.c_str(), nullptr, nullptr, &status );
	if (status == 0)
	{
		// we have a better demangled name we can use -- replace it within the line
		name = demangled;
	}

	free(demangled);

	return name;
}


dm::VStr get_backtrace()
{
	dm::VStr v;

	void *buffer[30];
	const int rows = backtrace( buffer, sizeof(buffer) );
	char **symbols = backtrace_symbols( buffer, rows );

	// for example, this will give us something along these lines:
	//
	//		symbols[0] == "/usr/lib/liblox.so(_ZN3Lox12getBacktraceEi+0x40) [0x7fd7a5488fc0]"
	//		symbols[1] == "/usr/lib/liblox.so(_ZN3Lox9Exception10initializeERKSsS2_iS2_S2_S2_+0x51) [0x7fd7a548a1c1]"
	//		symbols[2] == "/usr/lib/liblox.so(_ZN3Lox9ExceptionC1ERKSsS2_iS2_S2_+0xbf) [0x7fd7a548ac9f]"
	//
	const std::regex rx(
		"^"             // start of string
		"(.+)\\("       // 1st group:  path/library
		"(.+)\\+"       // 2nd group:  mangled function/method name
		"(0x.+)\\)"     // 3rd group:  offset into function/method
		" \\[(0x.+)\\]" // 4th group:  address
		"$"             // end of string
	);

	// loop through all the entries returned by backtrace_symbols() and format them neatly (with the C++ names demangled where possible)
	for ( int i = 0; i < rows; i ++ )
	{
		const std::string line = symbols[i];

		std::smatch what;
		const bool valid = std::regex_match( line, what, rx, std::regex_constants::match_default );
		if ( ! valid )
		{
			// if we cannot parse the line, store it as-is
			v.push_back( line );
		}
		else
		{
			const std::string path		=				what.str(1);
			const std::string name		= demangle_cpp(	what.str(2));
			const std::string offset	=				what.str(3);
			const std::string address	=				what.str(4);

			v.push_back( path + ": " + name + " +" + offset + " [" + address + "]" );
		}
	}

	free(symbols);

	return v;
}


void dm::DarkMarkApplication::signal_handler(int signal_number)
{
	dm::Log("aborting due to signal: \"" + std::string(strsignal(signal_number)) + "\" [signal #" + std::to_string(signal_number) + "]");

	try
	{
		const auto v = get_backtrace();
		for (size_t idx = 0; idx < v.size(); idx ++)
		{
			dm::Log("backtrace #" + std::to_string(idx) + ": " + v.at(idx));
		}
	}
	catch (...)
	{
		// ignore it, we're about to abort anyway
	}

	std::signal(SIGABRT, SIG_DFL);
	std::abort();

	return;
}


void dm::DarkMarkApplication::setup_signal_handling()
{
	std::signal(SIGINT	, dm::DarkMarkApplication::signal_handler); // 2: interrupt
	std::signal(SIGILL	, dm::DarkMarkApplication::signal_handler); // 4: illegal instruction
	std::signal(SIGABRT	, dm::DarkMarkApplication::signal_handler); // 6: abort(3)
	std::signal(SIGFPE	, dm::DarkMarkApplication::signal_handler); // 8: floating point exception
	std::signal(SIGSEGV	, dm::DarkMarkApplication::signal_handler); // 11: segfault
	std::signal(SIGTERM	, dm::DarkMarkApplication::signal_handler); // 15: terminate

	return;
}


void dm::DarkMarkApplication::initialise(const String & commandLine)
{
	// This method is where you should put your application's initialisation code.

	std::set_terminate(DarkMark_CPlusPlus_Terminate_Handler);
	std::set_unexpected(DarkMark_CPlusPlus_Unexpected_Handler);
	SystemStats::setApplicationCrashHandler(DarkMark_Juce_Crash_Handler);

	setup_signal_handling();

	dm::Log("starting DarkMark v" DARKMARK_VERSION);

	std::srand(std::time(nullptr));

	cfg.reset(new Cfg);
	startup_wnd.reset(new StartupWnd);

	// before we go any further, check to see if Darknet is installed where we think it is
	const auto darknet_dir = cfg->get_str("darknet_dir");
	File f(darknet_dir);
	if (f.isDirectory() == false)
	{
		Log("darknet directory does not exist: " + darknet_dir);
		AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon, "DarkMark",
				"The darknet directory is set to " + darknet_dir + ", but that directory does not exist.\n\n"
				"Please quit from DarkMark and edit the configuration file " + cfg->getFile().getFullPathName().toStdString() + " to set \"darknet_dir\" to the correct location.");
	}

	return;
}


void dm::DarkMarkApplication::shutdown()
{
	// shutdown the application
	dm::Log("shutting down DarkMark v" DARKMARK_VERSION);

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
