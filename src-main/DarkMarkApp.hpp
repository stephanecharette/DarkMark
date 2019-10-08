/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class DarkMarkApplication : public JUCEApplication
	{
		public:

			DarkMarkApplication(void);
			virtual ~DarkMarkApplication(void);

			const String getApplicationName		(void) override	{ return "DarkMark";		}
			const String getApplicationVersion	(void) override	{ return DARKMARK_VERSION;	}
			bool moreThanOneInstanceAllowed		(void) override	{ return false;				}
			void systemRequestedQuit			(void) override { quit();					}

			void initialise(const String& commandLine) override;

			void shutdown() override;

			void anotherInstanceStarted (const String& commandLine) override;

			void unhandledException(const std::exception *e, const String &sourceFilename, int lineNumber) override;

			TooltipWindow tool_tip;

			std::unique_ptr<Cfg>		cfg;
			std::unique_ptr<DMWnd>		wnd;
			std::unique_ptr<DarkHelp>	darkhelp;
			std::unique_ptr<DMStatsWnd>	stats_wnd;
			std::unique_ptr<DMAboutWnd>	about_wnd;
	};


	/** Return a reference to the single running application.  Will throw if the
	 * app somehow does not exist, such as early in the startup process, or late
	 * in the shutdown sequence.
	 */
	inline DarkMarkApplication & dmapp(void)
	{
		DarkMarkApplication * app = dynamic_cast<dm::DarkMarkApplication*>(JUCEApplication::getInstance());
		if (app == nullptr)
		{
			throw std::runtime_error("failed to find an active application pointer");
		}

		return *app;
	}


	/// Quick and easy access to configuration.  Will throw if the application does not exist.
	inline Cfg & cfg(void)
	{
		return *dmapp().cfg;
	}

	inline DarkHelp & darkhelp(void)
	{
		return *dmapp().darkhelp;
	}
}
