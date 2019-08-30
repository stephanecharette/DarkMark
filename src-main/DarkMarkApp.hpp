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

			std::unique_ptr<DMWnd> wnd;
	};
}
