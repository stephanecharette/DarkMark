/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#pragma once

#include <ctime>
#include <fstream>
#include <regex>
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
	class DMContent;
	class CrosshairComponent;
	class DarkMarkApplication;

	typedef std::vector<std::string> VStr;
}

#include "Log.hpp"
#include "Cfg.hpp"
#include "Bitmaps.hpp"
#include "Mark.hpp"
#include "CrosshairComponent.hpp"
#include "DMCorner.hpp"
#include "DMCanvas.hpp"
#include "DMContent.hpp"
#include "DMWnd.hpp"
#include "DarkMarkApp.hpp"
