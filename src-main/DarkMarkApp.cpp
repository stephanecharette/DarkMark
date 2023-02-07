// DarkMark (C) 2019-2022 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"
#include <csignal>
#include <cstring>
#ifndef WIN32
#include <cxxabi.h>
#include <execinfo.h>
#endif // !WIN32


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

#ifndef WIN32
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
			const std::string path			=								what.str(1);
			const std::string name		= demangle_cpp( what.str(2));
			const std::string offset		=								what.str(3);
			const std::string address	=								what.str(4);

			v.push_back( path + ": " + name + " +" + offset + " [" + address + "]" );
		}
	}

	free(symbols);

	return v;
}

#else
dm::VStr get_backtrace()
{
	dm::VStr v;
	v.push_back("Not implemented in Windows yet!");
	return v;
}

std::string strsignal(int sig)
{
	return std::to_string(sig);
}
#endif // !WIN32


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


bool validPositiveInt(const std::string & str)
{
	try
	{
		size_t pos = 0;
		const auto i = std::stoi(str, &pos);

		// catch the difference between "4" and "4abc"
		if (pos == str.size() and i >= 0)
		{
			return true;
		}
	}
	catch (...)
	{
	}

	return false;
}


bool validPositiveFloat(const std::string & str)
{
	try
	{
		size_t pos = 0;
		const auto f = std::stof(str, &pos);

		if (pos == str.size() and f > 0.0f)
		{
			return true;
		}
	}
	catch (...)
	{
	}

	return false;
}


bool validBool(const std::string & str)
{
	if (str == "true"	||
		str == "TRUE"	||
		str == "yes"	||
		str == "YES"	||
		str == "on"		||
		str == "ON"		||
		str == "1"		||
		str == "false"	||
		str == "FALSE"	||
		str == "no"		||
		str == "NO"		||
		str == "off"	||
		str == "OFF"	||
		str == "0"		)
	{
		return true;
	}

	return false;
}


void dm::DarkMarkApplication::initialise(const String & commandLine)
{
	// This method is where you should put your application's initialisation code.

	std::set_terminate(DarkMark_CPlusPlus_Terminate_Handler);
#ifndef WIN32
	std::set_unexpected(DarkMark_CPlusPlus_Unexpected_Handler);
#endif // !WIN32
	SystemStats::setApplicationCrashHandler(DarkMark_Juce_Crash_Handler);

	setup_signal_handling();

	dm::Log("starting DarkMark v" DARKMARK_VERSION);

	std::srand(std::time(nullptr));

	#if DARKNET_GEN_SIMPLIFIED
		// different default font is needed for Japanese characters; this requires: "sudo apt-get install fonts-ipafont-gothic"
		Desktop::getInstance().getDefaultLookAndFeel().setDefaultSansSerifTypefaceName("IPAPGothic");
	#else
		/* Until 2022-04, JUCE would default to "Liberation Sans" on Ubuntu.  But at some point in April 2022, JUCE seems to
		 * have changed the default to an italic version of "DejaVu Sans".  Unfortunately, I don't like how it makes things
		 * look since it is a heavy font and -- in my opinion! -- italics is a poor choice for the default font in a GUI.
		 * So I'm now hard-coding the font back to "Liberation Sans".  But I worry this will cause font issues for people
		 * who want to run DarkMark on systems without that font.  This may need to be revisited.
		 */
		Desktop::getInstance().getDefaultLookAndFeel().setDefaultSansSerifTypefaceName("Liberation Sans");
//		Desktop::getInstance().getDefaultLookAndFeel().setDefaultSansSerifTypefaceName("DejaVu Sans");
//		Desktop::getInstance().getDefaultLookAndFeel().setDefaultSansSerifTypefaceName("Ubuntu Condensed");
//		Desktop::getInstance().getDefaultLookAndFeel().setDefaultSansSerifTypefaceName("Ubuntu");
	#endif

	cfg.reset(new Cfg);

	for (auto parm : StringArray::fromTokens(commandLine, true))
	{
		parm = parm.unquoted();
		auto key = parm.toStdString();
		auto val = key;
		dm::Log("parsing parameter: " + parm.toStdString());
		auto pos = parm.indexOfChar('=');
		if (pos > 0)
		{
			// split the parm into 2 parts based on the "=" sign
			key = parm.substring(0, pos).toStdString();
			val = parm.substring(pos + 1).toStdString();
		}

		if (cli_options.count(key))
		{
			dm::Log("Error: duplicate key \"" + key + "\" in CLI parameters: 1=\"" + cli_options[key] + "\", 2=\"" + val + "\"");
			throw std::runtime_error("CLI parameters has a duplicate key: \"" + key + "\"");
		}

		cli_options[key] = val;
	}

	String project_key;
	for (const auto & [key, val] : cli_options)
	{
		if (key == "help" or key == "--help")
		{
			dm::Log("For help with DarkMark's CLI options, please see: https://www.ccoderun.ca/darkmark/CLI.html");
			systemRequestedQuit();
		}
		else if (key == "version" or key == "--version")
		{
			dm::Log("DarkMark v" DARKMARK_VERSION);
			systemRequestedQuit();
		}
		else if (key == "editor" and val == "gen-darknet")
		{
			// if "editor" is specified, the only action currently supported is "gen-darknet"
		}
		else if (key == "darknet" and val == "run")
		{
		}
		else if (key == "add")
		{
			File f(val);
			if (f.isDirectory() == false)
			{
				dm::Log("Error: directory does not exist: \"" + val + "\"");
				throw std::runtime_error("cannot add directory \"" + val + "\"");
			}

			// go through all of the existing directories and make sure we don't already have this project
			for (const String & k : cfg->getAllProperties().getAllKeys())
			{
				if (k.endsWith("_dir"))
				{
					File kf(cfg->getValue(k));

					if (kf.getFullPathName() == f.getFullPathName())
					{
						dm::Log("Error: project already exists with this directory: " + k.toStdString() + "=" + kf.getFullPathName().toStdString());
						throw std::runtime_error("cannot add new project since it already seems to exist: " + k.toStdString() + "=" + kf.getFullPathName().toStdString());
					}

					if (kf.isAChildOf(f) or f.isAChildOf(kf))
					{
						dm::Log("Error: directories " + f.getFullPathName().toStdString() + " and " + kf.getFullPathName().toStdString() + " seem to overlap");
						throw std::runtime_error("cannot add " + val + " due to existing project " + k.toStdString() + "=" + kf.getFullPathName().toStdString());
					}
				}
			}
		}
		else if (key == "del")
		{
			File f(val);

			String name;
			for (const String & k : cfg->getAllProperties().getAllKeys())
			{
				if (k.endsWith("_dir"))
				{
					if (File(cfg->getValue(k)).getFullPathName() == f.getFullPathName())
					{
						dm::Log("found a match: " + k.toStdString() + " uses directory " + val);
						auto p1 = k.indexOfChar('_');
						auto p2 = k.lastIndexOfChar('_');
						if (p1 > 0 and p2 > p1)
						{
							project_key = k.substring(p1 + 1, p2);
							name = "project_" + project_key;
							break;
						}
					}
				}
			}

			if (name.isNotEmpty())
			{
				SStr keys_to_delete;
				for (const String & k : cfg->getAllProperties().getAllKeys())
				{
					if (k.startsWith(name))
					{
						keys_to_delete.insert(k.toStdString());
					}
				}
				for (const std::string & k : keys_to_delete)
				{
					dm::Log(val + ": removing key from configuration: " + k);
					cfg->removeValue(k);
				}

				// this is intentional, once the project has been removed from configuration we want to completely exit
				// note this will be processed asynchronously once the message queue is running
				systemRequestedQuit();
			}
			else
			{
				dm::Log("Error: did not find a project to delete using the directory name \"" + val + "\"");
				throw std::runtime_error("cannot delete project " + val);
			}
		}
		else if (key == "load")
		{
			/* Check to see if this project exits in configuration.
			 * For example, configuration might have these lines:
			 *
			 *		<VALUE name="project_1760944655_dir" val="/home/stephane/nn/driving"/>
			 *		<VALUE name="project_1760944655_name" val="driving"/>
			 *
			 * ...in which case if "load=driving" or "load=1760944655" was specified,
			 * we'd want to match and record the key "1760944655".
			 */
			for (const String & k : cfg->getAllProperties().getAllKeys())
			{
				if (k.endsWith("_dir") or k.endsWith("_name"))
				{
					if (cfg->getValue(k).endsWith(val) or k.toStdString() == "project_" + val + "_dir")
					{
						// found which project we need to load!
						dm::Log("match for \"" + val + "\" found in " + k.toStdString() + "=" + cfg->getValue(k).toStdString());
						auto p1 = k.indexOfChar('_');
						auto p2 = k.lastIndexOfChar('_');
						if (p1 > 0 and p2 > p1)
						{
							project_key = k.substring(p1 + 1, p2);
							dm::Log("project key=" + project_key.toStdString());
						}
						break;
					}
				}
			}

			if (project_key.isEmpty())
			{
				dm::Log("Error: failed to match project name, dir, or key: \"" + val + "\"");
				throw std::runtime_error("cannot find project \"" + val + "\"");
			}
		}
		else if (key == "template")
		{
			File f(val);
			if (!f.existsAsFile())
			{
				dm::Log("template file \"" + val + "\" not found");
				throw std::runtime_error("cannot find template \"" + val + "\"");
			}
		}
		else if (
			validPositiveInt(val) and (
				key == "width"					or
				key == "height"					or
				key == "max_batches"			or
				key == "batch_size"				or
				key == "subdivisions"			))
		{
			// no further validation performed here
		}
		else if (
			validBool(val) and (
				key == "do_not_resize_images"		or
				key == "resize_images"				or
				key == "tile_images"				or
				key == "zoom_images"				or
				key == "limit_neg_samples"			or
				key == "limit_validation_images"	or
				key == "yolo_anchors"				or
				key == "class_imbalance"			or
				key == "mosaic"						or
				key == "cutmix"						or
				key == "mixup"						or
				key == "flip"						or
				key == "restart_training"			))
		{
			// no further validation performed here
		}
		else if (
			validPositiveFloat(val) and (
				key == "learning_rate"			))
		{
			// no further validation performed here
		}
		else
		{
			dm::Log("Error: unknown CLI option: key=\"" + key + "\", val=\"" + val + "\"");
			throw std::runtime_error("CLI parameter is invalid: \"" + key + "\"");
		}
	}
	if (project_key.isNotEmpty())
	{
		cli_options["project_key"] = project_key.toStdString();
	}

	startup_wnd.reset(new StartupWnd);

	// before we go any further, check to see if Darknet is installed where we think it is
	const auto darknet_dir = cfg->get_str("darknet_dir");
	File f(darknet_dir);
	if (f.isDirectory() == false)
	{
		Log("darknet directory does not exist: " + darknet_dir);
		AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon, "DarkMark",
				"The darknet directory is set to " + darknet_dir + ", but that directory does not exist.\n"
				"\n"
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
