// DarkMark (C) 2019-2021 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	/** There is one of these for every tab in the notebook.
	 * Also see @ref dm::DMReviewWnd which contains the window as well as the notebook itself.
	 */
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
			virtual void sortOrderChanged(int newSortColumnId, bool isForwards) override;

			/** This determines the order in which rows will appear.
			 * E.g., sort_idx[0] is the map index of the review info that must appear as the first row in the table.
			 */
			std::vector<size_t> sort_idx;

			/// Map of review info, where each map record has everything needed to represent a single row in the table.
			const MReviewInfo & mri;
	};
}
