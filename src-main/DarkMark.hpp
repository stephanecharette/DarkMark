// DarkMark (C) 2019-2023 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include <ctime>
#include <fstream>
#include <regex>
#include <thread>
#include <chrono>
#include <random>
#include <algorithm>
#include <DarkHelp.hpp>
#include <JuceHeader.h>


/* OpenCV4 has renamed some common defines and placed them in the cv namespace.
 * Need to deal with this until older versions of OpenCV are no longer in use.
 */
#if 0
#ifndef CV_INTER_CUBIC
#define CV_INTER_CUBIC cv::INTER_CUBIC
#endif
#ifndef CV_AA
#define CV_AA cv::LINE_AA
#endif
#endif
#ifndef CV_INTER_AREA
#define CV_INTER_AREA cv::INTER_AREA
#endif
#ifndef CV_FILLED
#define CV_FILLED cv::FILLED
#endif
#ifndef CV_IMWRITE_PNG_COMPRESSION
#define CV_IMWRITE_PNG_COMPRESSION cv::ImwriteFlags::IMWRITE_PNG_COMPRESSION
#endif
#ifndef CV_IMWRITE_JPEG_QUALITY
#define CV_IMWRITE_JPEG_QUALITY cv::ImwriteFlags::IMWRITE_JPEG_QUALITY
#endif


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
	class CfgHandler;
	class DarknetWnd;
	class WndCfgTemplates;
	class StartupWnd;
	class StartupCanvas;
	class PdfImportWindow;
	class VideoImportWindow;
	class SettingsWnd;
	class FilterWnd;
	class ProjectInfo;
	class DMContentReview;
	class DMReviewWnd;
	class DMReviewCanvas;
	class DMContentMoveEmptyImages;
	class DMContentRotateImages;
	class DMContentFlipImages;
	class DMContentDeleteRotateAndFlipImages;
	class DMContentImportTxt;
	class ScrollField;
	class CrosshairComponent;
	class DarkMarkApplication;
	struct ReviewInfo;

	typedef std::vector<std::string> VStr;
	typedef std::set<std::string> SStr;
	typedef std::set<size_t> SId;
	typedef std::map<size_t, std::string> MIdxStr;
	typedef std::map<std::string, std::string> MStr;
	typedef std::map<std::string, size_t> MStrSize;
	typedef std::vector<cv::Point> Contour;
	typedef std::vector<Contour> VContours;
	typedef std::vector<size_t> VSizet;
}

#include "Text.hpp"
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
#include "CfgHandler.hpp"
#include "DarknetWnd.hpp"
#include "WndCfgTemplates.hpp"
#include "PdfImportWindow.hpp"
#include "VideoImportWindow.hpp"
#include "StartupWnd.hpp"
#include "SettingsWnd.hpp"
#include "FilterWnd.hpp"
#include "StartupCanvas.hpp"
#include "DMContentImportTxt.hpp"
#include "DMContentReloadResave.hpp"
#include "DMContentRotateImages.hpp"
#include "DMContentFlipImages.hpp"
#include "DMContentDeleteRotateAndFlipImages.hpp"
#include "DMContentMoveEmptyImages.hpp"
#include "DMContentImageFilenameSort.hpp"
#include "DMContentStatistics.hpp"
#include "DMContentReview.hpp"
#include "DMWnd.hpp"
#include "DarkMarkApp.hpp"
