// DarkMark (C) 2019-2021 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class DMContentImportTxt : public ThreadWithProgressWindow
	{
		public:

			DMContentImportTxt(dm::DMContent & c, const VStr & v);

			virtual ~DMContentImportTxt();

			virtual void run();

			DMContent & content;

			VStr image_filenames;
	};
}
