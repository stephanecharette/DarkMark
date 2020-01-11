/* DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	struct ReviewInfo
	{
		cv::Mat mat;
		std::string filename;
		size_t class_idx;
		std::string msg;
	};

	/** Key is a sequential counter that starts at zero, value is the review info structure.  Was done this way instead of
	 * std::vector because there is no reason why the entries need to be in a single large block of memory, yet we need to
	 * easily retrieve an object with operator[] when we draw the table in the review.
	 */
	typedef std::map<size_t, ReviewInfo> MReviewInfo;

	/** Multiple MReviewInfo objects, typically one for each class defined.  The key is the class index, the value is the
	 * review info map.
	 */
	typedef std::map<size_t, MReviewInfo> MMReviewInfo;

	class DMReviewWnd : public DocumentWindow //, TableListBoxModel
	{
		public:

			DMReviewWnd(DMContent & c);

			virtual ~DMReviewWnd();

			virtual void closeButtonPressed();
			virtual void userTriedToCloseWindow();

			void rebuild_notebook();

			DMContent & content;
			Notebook notebook;
			MMReviewInfo m;
	};
}
