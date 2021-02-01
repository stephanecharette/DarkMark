// DarkMark (C) 2019-2021 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class DMReviewCanvas : public TableListBox, public TableListBoxModel
	{
		public:

			/// Constructor.
			DMReviewCanvas(const MReviewInfo & m);

			/// Destructor.
			virtual ~DMReviewCanvas();

			virtual int getNumRows();
			virtual void cellDoubleClicked(int rowNumber, int columnId, const MouseEvent & event);
			virtual String getCellTooltip(int rowNumber, int columnId);
			virtual void paintRowBackground(Graphics & g, int rowNumber, int width, int height, bool rowIsSelected);
			virtual void paintCell(Graphics & g, int rowNumber, int columnId, int width, int height, bool rowIsSelected);

			const MReviewInfo & mri;
	};
}
