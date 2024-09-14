// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class DMContentReviewIoU : public ThreadWithProgressWindow
	{
	public:

		DMContentReviewIoU(dm::DMContent & c);

		virtual ~DMContentReviewIoU();

		virtual void run();

		DMContent & content;
	};
}
