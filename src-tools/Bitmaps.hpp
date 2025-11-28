// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	Image DarkMarkLogo();

	Image convert_opencv_mat_to_juce_image(const cv::Mat & mat);

	Image AboutLogoWhiteBackground();
	Image AboutLogoRedSwirl();
	Image AboutLogoDarknet();
}
