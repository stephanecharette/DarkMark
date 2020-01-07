/* DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#pragma once

#include <ctime>
#include <fstream>
#include <regex>
#include <thread>
#include <chrono>
#include <DarkHelp.hpp>

#include <JuceHeader.h>


// forward declare a few classes
namespace dm
{
	class Cfg;
	class DMWnd;
	class Mark;
	class DMCorner;
	class DMCanvas;
	class Notebook;
	class DMContent;
	class DMJumpWnd;
	class DMStatsWnd;
	class DMAboutWnd;
	class DarknetWnd;
	class StartupWnd;
	class StartupCanvas;
	class SettingsWnd;
	class ProjectInfo;
	class DMContentReview;
	class DMReviewWnd;
	class DMReviewCanvas;
	class CrosshairComponent;
	class DarkMarkApplication;

	struct ReviewInfo;

	typedef std::vector<std::string> VStr;
	typedef std::set<std::string> SStr;
}

#include "Log.hpp"
#include "Cfg.hpp"
#include "Bitmaps.hpp"
#include "Mark.hpp"
#include "Tools.hpp"
#include "CrosshairComponent.hpp"
#include "ProjectInfo.hpp"
#include "Notebook.hpp"
#include "DMJumpWnd.hpp"
#include "DMCorner.hpp"
#include "DMCanvas.hpp"
#include "DMContent.hpp"
#include "DMStatsWnd.hpp"
#include "DMAboutWnd.hpp"
#include "DMReviewWnd.hpp"
#include "DMReviewCanvas.hpp"
#include "DarknetWnd.hpp"
#include "StartupWnd.hpp"
#include "SettingsWnd.hpp"
#include "StartupCanvas.hpp"
#include "DMContentReloadResave.hpp"
#include "DMContentImageFilenameSort.hpp"
#include "DMContentStatistics.hpp"
#include "DMContentReview.hpp"
#include "DMWnd.hpp"
#include "DarkMarkApp.hpp"
