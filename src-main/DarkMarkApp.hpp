// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class DarkMarkApplication : public JUCEApplication
	{
		public:

			DarkMarkApplication(void);
			virtual ~DarkMarkApplication(void);

			static void signal_handler(int signal_number);

			static void setup_signal_handling();

			virtual const String getApplicationName		(void)	override	{ return "DarkMark";		}
			virtual const String getApplicationVersion	(void)	override	{ return DARKMARK_VERSION;	}
			virtual bool moreThanOneInstanceAllowed		(void)	override	{ return true;				}
			virtual void systemRequestedQuit			(void)	override	{ quit();					}
			virtual void initialise(const String & commandLine)	override;
			virtual void shutdown() override;

			void unhandledException(const std::exception * e, const String &sourceFilename, int lineNumber) override;

			MStr cli_options;

			TooltipWindow tool_tip;

			std::unique_ptr<Cfg>			cfg;
			std::unique_ptr<DMWnd>			wnd;
			std::unique_ptr<DarkHelp::NN>	darkhelp_nn;
			std::unique_ptr<DMStatsWnd>		stats_wnd;
			std::unique_ptr<AboutWnd>		about_wnd;
			std::unique_ptr<DMJumpWnd>		jump_wnd;
			std::unique_ptr<DMReviewWnd>	review_wnd;
			std::unique_ptr<DarknetWnd>		darknet_wnd;
			std::unique_ptr<StartupWnd>		startup_wnd;
			std::unique_ptr<SettingsWnd>	settings_wnd;
			std::unique_ptr<FilterWnd>		filter_wnd;
	};


	/** Return a reference to the single running application.  Will throw if the
	 * app somehow does not exist, such as early in the startup process, or late
	 * in the shutdown sequence.
	 */
	inline DarkMarkApplication & dmapp()
	{
		#if 0
			/* I'd rather use this next line, but it is causing a strange typeinfo error during linking that I don't understand,
			 * so we'll have to do the reinterpret cast instead.  This should be fine since in this case the problem isn't that
			 * getInstance() will return a different pointer type; either the JUCE app exists, or it hasn't yet been created in
			 * which case we'll still get back a NULL pointer.
			 */
			DarkMarkApplication * app = dynamic_cast<DarkMarkApplication*>(JUCEApplication::getInstance());
		#else
			DarkMarkApplication * app = reinterpret_cast<DarkMarkApplication*>(JUCEApplication::getInstance());
		#endif
		if (app == nullptr)
		{
			throw std::runtime_error("failed to find an active application pointer");
		}

		return *app;
	}

	/// Quick and easy access to configuration.  Will throw if the application does not exist.
	inline Cfg & cfg()
	{
		return *dmapp().cfg;
	}

	/// Quick and easy access to DarkHelp (darknet).  Will throw if the application does not exist.
	inline DarkHelp::NN & darkhelp_nn()
	{
		return *dmapp().darkhelp_nn;
	}
}
