/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class DMCorner final : public CrosshairComponent
	{
		public:

			enum class EType
			{
				kNone,
				kTL,
				kTR,
				kBR,
				kBL
			};

			DMCorner();

			virtual ~DMCorner();

			virtual void set_type(const EType & t);

			virtual void rebuild_cache_image();

			EType type;

			cv::Mat original_image;

			/// Size of each cell within the corner image.  Size does @em not include the right and bottom border.
			int cell_size;
			int cols;
			int rows;

			cv::Point offset;
			cv::Point poi;
	};
}
