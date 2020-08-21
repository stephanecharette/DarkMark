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
	getHeader().addColumn("rectangle"	, 4, 100, 30, -1, TableHeaderComponent::notSortable);
	getHeader().addColumn("overlap"		, 5, 100, 30, -1, TableHeaderComponent::notSortable);
	getHeader().addColumn("filename"	, 6, 100, 30, -1, TableHeaderComponent::notSortable);
	getHeader().addColumn("type"		, 7, 100, 30, -1, TableHeaderComponent::notSortable);
	getHeader().addColumn("message"		, 8, 100, 30, -1, TableHeaderComponent::notSortable);
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

		const VStr & image_filenames = dmapp().wnd->content.image_filenames;
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

//		dmapp().review_wnd->setMinimised(true);
		dmapp().wnd->content.load_image(idx);
		dmapp().wnd->content.highlight_rectangle(review_info.r);
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

		if (rowIsSelected)
		{
			colour = Colours::lightblue; // selected rows will have a blue background
		}
		else if (review_info.warnings.size() > 0)
		{
			colour = Colours::lightyellow;
		}
		else if (review_info.errors.size() > 0)
		{
			colour = Colours::lightpink;
		}
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
		columnId > 8					)
	{
		// rows are 0-based, columns are 1-based
		return;
	}

	auto colour = Colours::black;

	g.setOpacity(1.0);

	const auto & review_info = mri.at(rowNumber);

	/* columns:
		*		1: row number
		*		2: mark (image)
		*		3: size
		*		4: rectangle
		*		5: overlap
		*		6: filename
		*		7: mime type
		*		8: warning + error messages
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
		if (columnId == 6)	str = review_info.filename;
		if (columnId == 7)	str = review_info.mime_type;

		if (columnId == 4)
		{
			if (review_info.r.area() > 0)
			{
				str =	std::to_string(review_info.r.tl().x) + ", " +
						std::to_string(review_info.r.tl().y) + ", " +
						std::to_string(review_info.r.br().x) + ", " +
						std::to_string(review_info.r.br().y);
			}
		}

		if (columnId == 5)
		{
			const int percentage = std::round(review_info.overlap_sum * 100.0);
			if (percentage > 0)
			{
				str = std::to_string(percentage) + "%";
				if (percentage >= 10)
				{
					colour = Colours::darkred;
				}
			}
		}

		if (columnId == 8)
		{
			for (const auto & msg : review_info.warnings)
			{
				if (str.empty() == false)
				{
					str += "; ";
				}
				str += msg;
			}

			for (const auto & msg : review_info.errors)
			{
				if (str.empty() == false)
				{
					str += "; ";
				}
				str += msg;
			}

			if (str.empty() == false)
			{
				colour = Colours::darkred;
			}
		}

		// draw the text for this cell
		g.setColour(colour);
		Rectangle<int> r(0, 0, width, height);
		g.drawFittedText(str, r.reduced(2), Justification::centredLeft, 1);
	}

	// draw the divider on the right side of the column
	g.setColour(Colours::black.withAlpha(0.5f));
	g.drawLine(width, 0, width, height);

	return;
}
