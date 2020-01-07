/* DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include "DarkMark.hpp"


dm::DMReviewWnd::DMReviewWnd(DMContent & c) :
	DocumentWindow("DarkMark v" DARKMARK_VERSION " Review", Colours::darkgrey, TitleBarButtons::closeButton),
	content(c)
{
	setContentNonOwned		(&notebook, true);
	setUsingNativeTitleBar	(true			);
	setResizable			(true, false	);
	setDropShadowEnabled	(true			);

	setIcon(DarkMarkLogo());
	ComponentPeer *peer = getPeer();
	if (peer)
	{
		peer->setIcon(DarkMarkLogo());
	}

	if (cfg().containsKey("ReviewWnd"))
	{
		restoreWindowStateFromString( cfg().getValue("ReviewWnd") );
	}
	else
	{
		centreWithSize(600, 200);
	}

	setVisible(true);

	return;
}


dm::DMReviewWnd::~DMReviewWnd()
{
	cfg().setValue("ReviewWnd", getWindowStateAsString());

	return;
}


void dm::DMReviewWnd::closeButtonPressed()
{
	// close button

	dmapp().review_wnd.reset(nullptr);

	return;
}


void dm::DMReviewWnd::userTriedToCloseWindow()
{
	// ALT+F4

	dmapp().review_wnd.reset(nullptr);

	return;
}


void dm::DMReviewWnd::rebuild_notebook()
{
	while (notebook.getNumTabs() > 0)
	{
		notebook.removeTab(0);
	}

	for (auto iter : m)
	{
		const size_t class_idx = iter.first;
		const MReviewInfo & mri = m.at(class_idx);

		std::string name = "#" + std::to_string(class_idx);
		if (content.names.size() > class_idx)
		{
			// if we can, we'd much rather use the "official" name for this class
			name = content.names.at(class_idx);
		}

		Log("creating a notebook tab for class \"" + name + "\", mri has " + std::to_string(mri.size()) + " entries");

		notebook.addTab(name, Colours::darkgrey, new DMReviewCanvas(mri), true);
	}

	return;
}


#if 0

//	tlb.updateContent();


int dm::DMStatsWnd::getNumRows()
{
	return m.size();
}


void dm::DMStatsWnd::cellDoubleClicked(int rowNumber, int columnId, const MouseEvent & event)
{
	// rows are 0-based, columns are 1-based
	if (rowNumber >= 0 and rowNumber < (int)m.size())
	{
		std::string filename;

		const auto & s = m.at(rowNumber);

		if (columnId == 5)	filename = s.min_filename;
		if (columnId == 7)	filename = s.max_filename;

		if (filename.empty() == false)
		{
			for (size_t idx = 0; idx < content.image_filenames.size(); idx ++)
			{
				if (content.image_filenames.at(idx) == filename)
				{
					content.load_image(idx);
					break;
				}
			}
		}
	}

	return;
}


String dm::DMStatsWnd::getCellTooltip(int rowNumber, int columnId)
{
	if (columnId == 3) return "the number of times this object appears across all images";
	if (columnId == 4) return "the number of images where this object appears at least once";
	// 5 is minimum size
	if (columnId == 6) return "the average size of this object (in pixels) across all images";
	// 7 is maximum size
	if (columnId == 8) return "standard deviation of the object's width (in pixels)";
	if (columnId == 9) return "standard deviation of the object's height (in pixels)";

	// rows are 0-based, columns are 1-based
	if (rowNumber >= 0 and rowNumber < (int)m.size())
	{
		const auto & s = m.at(rowNumber);

		if (columnId == 5)
		{
			return "double click to open " + s.min_filename;
		}
		else if (columnId == 7)
		{
			return "double click to open " + s.max_filename;
		}
	}

	return "";

}


void dm::DMStatsWnd::paintRowBackground(Graphics & g, int rowNumber, int width, int height, bool rowIsSelected)
{
	Colour colour = Colours::white;
	if ( rowIsSelected )
	{
		colour = Colours::lightblue; // selected rows will have a blue background
	}
	g.fillAll( colour );

	// draw the cell bottom divider between rows
	g.setColour( Colours::black.withAlpha( 0.5f ) );
	g.drawLine( 0, height, width, height );
	return;
}


void dm::DMStatsWnd::paintCell(Graphics & g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
	if (rowNumber < 0				or
		rowNumber >= (int)m.size()	or
		columnId < 1				or
		columnId > 9				)
	{
		// rows are 0-based, columns are 1-based
		return;
	}

	const auto & s = m.at(rowNumber);

	/* columns:
	 *		1: class id
	 *		2: class name
	 *		3: count
	 *		4: files
	 *		5: min size
	 *		6: avg size
	 *		7: max size
	 *		8: SD width
	 *		9: SD height
	 */
	std::stringstream ss;
	ss << std::fixed << std::setprecision(2);

	switch (columnId)
	{
		case 1: ss << rowNumber;										break;
		case 2: ss << content.names.at(rowNumber);						break;
		case 3: ss << s.count;											break;
		case 4: ss << s.filenames.size();								break;
		case 5: ss << s.min_size.width << " x " << s.min_size.height;	break;
		case 6: ss << s.avg_w << " x " << s.avg_h;						break;
		case 7: ss << s.max_size.width << " x " << s.max_size.height;	break;
		case 8: ss << s.standard_deviation_width;						break;
		case 9: ss << s.standard_deviation_height;						break;
	}

	// draw the text and the right-hand-side dividing line between cells
	g.setColour( Colours::black );
	Rectangle<int> r(0, 0, width, height);
	g.drawFittedText(ss.str(), r.reduced(2), Justification::centredLeft, 1 );

	// draw the divider on the right side of the column
	g.setColour( Colours::black.withAlpha( 0.5f ) );
	g.drawLine( width, 0, width, height );

	return;
}
#endif
