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
	class DMCanvas;
	class Notebook;
	class DMContent;
	class DMJumpWnd;
	class DMStatsWnd;
	class AboutWnd;
	class DarknetWnd;
	class WndCfgTemplates;
	class StartupWnd;
	class StartupCanvas;
	class SettingsWnd;
	class ProjectInfo;
	class DMContentReview;
	class DMReviewWnd;
	class DMReviewCanvas;
	class DMContentRotateImages;
	class ScrollField;
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
#include "ScrollField.hpp"
#include "DMCanvas.hpp"
#include "DMContent.hpp"
#include "DMStatsWnd.hpp"
#include "AboutWnd.hpp"
#include "DMReviewWnd.hpp"
#include "DMReviewCanvas.hpp"
#include "DarknetWnd.hpp"
#include "WndCfgTemplates.hpp"
#include "StartupWnd.hpp"
#include "SettingsWnd.hpp"
#include "StartupCanvas.hpp"
#include "DMContentReloadResave.hpp"
#include "DMContentRotateImages.hpp"
#include "DMContentImageFilenameSort.hpp"
#include "DMContentStatistics.hpp"
#include "DMContentReview.hpp"
#include "DMWnd.hpp"
#include "DarkMarkApp.hpp"
