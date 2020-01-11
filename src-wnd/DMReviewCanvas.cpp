/* DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include "DarkMark.hpp"


dm::DMReviewCanvas::DMReviewCanvas(const MReviewInfo & m) :
	mri(m)
{
	const int h = cfg().get_int("review_table_row_height");
	if (h > getRowHeight())
	{
		setRowHeight(h);
	}

	getHeader().addColumn("#"			, 1, 100, 30, -1, TableHeaderComponent::notSortable);
	getHeader().addColumn("mark"		, 2, 100, 30, -1, TableHeaderComponent::notSortable);
	getHeader().addColumn("size"		, 3, 100, 30, -1, TableHeaderComponent::notSortable);
	getHeader().addColumn("filename"	, 4, 100, 30, -1, TableHeaderComponent::notSortable);
	getHeader().addColumn("type"		, 5, 100, 30, -1, TableHeaderComponent::notSortable);
	// if changing columns, also update paintCell() below

	if (cfg().containsKey("review_columns"))
	{
		getHeader().restoreFromString(cfg().get_str("review_columns"));
	}
	else
	{
		getHeader().setStretchToFitActive(true);
	}

	getHeader().setPopupMenuActive(false);
	setModel(this);

	return;
}


dm::DMReviewCanvas::~DMReviewCanvas()
{
	cfg().setValue("review_columns", getHeader().toString());

	return;
}


int dm::DMReviewCanvas::getNumRows()
{
	return mri.size();
}


void dm::DMReviewCanvas::cellDoubleClicked(int rowNumber, int columnId, const MouseEvent & event)
{
	// rows are 0-based, columns are 1-based
	if (rowNumber >= 0 and rowNumber < (int)mri.size())
	{
		const auto & review_info = mri.at(rowNumber);
		const std::string & fn = review_info.filename;

		// we know which image we want to load, but we need the index of that image within the vector of image filenames

		const VStr & image_filenames = dmapp().review_wnd->content.image_filenames;
		size_t idx = 0;
		while (true)
		{
			if (idx >= image_filenames.size())
			{
				idx = 0;
				break;
			}

			if (image_filenames.at(idx) == fn)
			{
				break;
			}

			idx ++;
		}

		dmapp().review_wnd->content.load_image(idx);
	}

	return;
}


String dm::DMReviewCanvas::getCellTooltip(int rowNumber, int columnId)
{
	// rows are 0-based, columns are 1-based
	if (rowNumber >= 0 and rowNumber < (int)mri.size())
	{
		const auto & review_info = mri.at(rowNumber);
		const std::string & fn = review_info.filename;

		return "double click to open " + fn;
	}

	return "";
}


void dm::DMReviewCanvas::paintRowBackground(Graphics & g, int rowNumber, int width, int height, bool rowIsSelected)
{
	Colour colour = Colours::white;

	if (rowNumber >= 0 and
		rowNumber < (int)mri.size())
	{
		const auto & review_info = mri.at(rowNumber);
		if (review_info.msg != "image/jpeg")
		{
			// looks like something about this image is different than "normal", so highlight it to the user
			colour = Colours::lightyellow;
		}
	}

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


void dm::DMReviewCanvas::paintCell(Graphics & g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
	if (rowNumber < 0					or
		rowNumber >= (int)mri.size()	or
		columnId < 1					or
		columnId > 5					)
	{
		// rows are 0-based, columns are 1-based
		return;
	}

	g.setOpacity(1.0);

	const auto & review_info = mri.at(rowNumber);

	/* columns:
		*		1: row number
		*		2: mark (image)
		*		3: size
		*		4: filename
		*		5: msg
		*/

	if (columnId == 2)
	{
		if (review_info.mat.empty() == false)
		{
			// draw a thumbnail of the image
			auto image = convert_opencv_mat_to_juce_image(review_info.mat);
			g.drawImageWithin(image, 0, 0, width, height,
					RectanglePlacement::xLeft				|
					RectanglePlacement::yMid				|
					RectanglePlacement::onlyReduceInSize	);
		}
	}
	else
	{
		std::string str;
		if (columnId == 1)	str = std::to_string(rowNumber + 1);
		if (columnId == 3)	str = std::to_string(review_info.mat.cols) + " x " + std::to_string(review_info.mat.rows);
		if (columnId == 4)	str = review_info.filename;
		if (columnId == 5)	str = review_info.msg;

		// draw the text for this cell
		g.setColour( Colours::black );
		Rectangle<int> r(0, 0, width, height);
		g.drawFittedText(str, r.reduced(2), Justification::centredLeft, 1);
	}

	// draw the divider on the right side of the column
	g.setColour( Colours::black.withAlpha( 0.5f ) );
	g.drawLine( width, 0, width, height );

	return;
}
