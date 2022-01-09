// DarkMark (C) 2019-2022 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	Image DarkMarkLogo();

	Image convert_opencv_mat_to_juce_image(cv::Mat mat);

	Image AboutLogoWhiteBackground();
	Image AboutLogoRedSwirl();
	Image AboutLogoDarknet();
}
