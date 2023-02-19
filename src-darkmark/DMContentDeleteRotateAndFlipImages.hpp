// DarkMark (C) 2019-2023 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class DMContentDeleteRotateAndFlipImages : public ThreadWithProgressWindow
	{
	public:

		DMContentDeleteRotateAndFlipImages(dm::DMContent & c);

		virtual ~DMContentDeleteRotateAndFlipImages();

		virtual void run();

		DMContent & content;
	};
}
