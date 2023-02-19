// DarkMark (C) 2019-2023 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"


dm::DMReviewCanvas::DMReviewCanvas(const MReviewInfo & m, const MStrSize & md5s) :
	mri(m),
	md5s(md5s)
{
	const int h = cfg().get_int("review_table_row_height");
	if (h > getRowHeight())
	{
		setRowHeight(h);
	}

	getHeader().addColumn("#"			, 1, 100, 30, -1, TableHeaderComponent::defaultFlags);
	getHeader().addColumn("mark"		, 2, 100, 30, -1, TableHeaderComponent::defaultFlags);
	getHeader().addColumn("zoom"		, 3, 100, 30, -1, TableHeaderComponent::defaultFlags);
	getHeader().addColumn("size"		, 4, 100, 30, -1, TableHeaderComponent::defaultFlags);
	getHeader().addColumn("aspect ratio", 5, 100, 30, -1, TableHeaderComponent::defaultFlags);
	getHeader().addColumn("rectangle"	, 6, 100, 30, -1, TableHeaderComponent::defaultFlags);
	getHeader().addColumn("overlap"		, 7, 100, 30, -1, TableHeaderComponent::defaultFlags);
	getHeader().addColumn("filename"	, 8, 100, 30, -1, TableHeaderComponent::defaultFlags);
	getHeader().addColumn("type"		, 9, 100, 30, -1, TableHeaderComponent::defaultFlags);
	getHeader().addColumn("md5"			, 10, 100, 30, -1, TableHeaderComponent::defaultFlags);
	getHeader().addColumn("duplicates"	, 11, 100, 30, -1, TableHeaderComponent::defaultFlags);
	getHeader().addColumn("message"		, 12, 100, 30, -1, TableHeaderComponent::defaultFlags);
	// if changing columns, also update paintCell() below

	if (cfg().containsKey("review_columns"))
	{
		getHeader().restoreFromString(cfg().get_str("review_columns"));
	}
	else
	{
		getHeader().setStretchToFitActive(true);
	}

	sort_idx.clear();
	sort_idx.reserve(mri.size());
	for (size_t i = 0; i < mri.size(); i ++)
	{
		sort_idx.push_back(i);
	}

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
		const auto & review_info = mri.at(sort_idx[rowNumber]);
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

		// jump out of "zoom" mode before we switch to the next image
		dmapp().wnd->content.zoom_point_of_interest = cv::Size(0, 0);
		dmapp().wnd->content.user_specified_zoom_factor = -1.0;

		dmapp().wnd->content.load_image(idx);
#if 0
		// see if the review window is hiding the rectangle we want to highlight
		auto r1 = localAreaToGlobal(Rectangle<int>(0, 0, getWidth(), getHeight()));
		auto r2 = dmapp().wnd->content.localAreaToGlobal(Rectangle<int>(review_info.r.x, review_info.r.y, review_info.r.width, review_info.r.height));

		Log("r1: x=" + std::to_string(r1.getX()) + " y=" + std::to_string(r1.getY()) + " w=" + std::to_string(r1.getWidth()) + " h=" + std::to_string(r1.getHeight()));
		Log("r2: x=" + std::to_string(r2.getX()) + " y=" + std::to_string(r2.getY()) + " w=" + std::to_string(r2.getWidth()) + " h=" + std::to_string(r2.getHeight()));

		if (r1.intersectRectangle(r2))
		{
			dmapp().review_wnd->setMinimised(true);
		}
#endif
		dmapp().wnd->content.highlight_rectangle(review_info.r);
	}

	return;
}


String dm::DMReviewCanvas::getCellTooltip(int rowNumber, int columnId)
{
	// rows are 0-based, columns are 1-based
	if (rowNumber >= 0 and rowNumber < (int)mri.size())
	{
		const auto & review_info = mri.at(sort_idx[rowNumber]);
		const std::string & fn = review_info.filename;

		return "double click to open " + fn;
	}

	return "";
}


void dm::DMReviewCanvas::paintRowBackground(Graphics & g, int rowNumber, int width, int height, bool rowIsSelected)
{
	if (rowNumber < 0					or
		rowNumber >= (int)mri.size()	)
	{
		// how did we ever get an invalid row?
		Log("invalid row detected: " + std::to_string(rowNumber) + "/" + std::to_string(mri.size()));
		return;
	}

	Colour colour = Colours::white;

	if (rowNumber >= 0 and
		rowNumber < (int)mri.size())
	{
		const auto & review_info = mri.at(sort_idx[rowNumber]);

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
		else if (md5s.at(review_info.md5) > 1)
		{
			colour = Colours::lightcoral;
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
		columnId > 12					)
	{
		// rows are 0-based, columns are 1-based
		return;
	}

	auto colour = Colours::black;

	g.setOpacity(1.0);

	const auto & review_info = mri.at(sort_idx[rowNumber]);

	/* columns:
	 *		1: row number
	 *		2: mark (image)
	 *		3: zoom
	 *		4: size
	 *		5: aspect ratio
	 *		6: rectangle
	 *		7: overlap
	 *		8: filename
	 *		9: mime type
	 *		10: md5
	 *		11: duplicates
	 *		12: warning + error messages
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
		if (columnId == 1)	str = std::to_string(sort_idx[rowNumber] + 1);
		if (columnId == 3)	str = std::to_string((int)std::round(100.0 * static_cast<double>(review_info.mat.cols) / std::max(1.0, static_cast<double>(review_info.r.width)))) + "%";
		if (columnId == 4)	str = std::to_string(review_info.r.width) + " x " + std::to_string(review_info.r.height);
		if (columnId == 5)	str = std::to_string(static_cast<double>(review_info.r.width) / static_cast<double>(review_info.r.height));
		if (columnId == 9)	str = review_info.mime_type;
		if (columnId == 10)	str = review_info.md5;

		if (columnId == 11)
		{
			const auto count = md5s.at(review_info.md5);
			if (count > 1)
			{
				str = std::to_string(count);
			}
		}

		if (columnId == 6)
		{
			if (review_info.r.area() > 0)
			{
				str =	std::to_string(review_info.r.tl().x) + ", " +
						std::to_string(review_info.r.tl().y) + ", " +
						std::to_string(review_info.r.br().x) + ", " +
						std::to_string(review_info.r.br().y);
			}
		}

		if (columnId == 7)
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
			str = review_info.filename;

			// see if this string will fit in the cell, and if not attempt to shorten it
			const int max_len = 1.05 * width;
			auto font = g.getCurrentFont();
			String fn = review_info.filename;
			while (true)
			{
				const auto len = font.getStringWidth(fn);
				if (len < max_len)
				{
					// we found a string to use
					break;
				}

				// string is too long -- see if we can remove a directory
				int pos = fn.indexOfChar(1, '/');
				if (pos < 1)
				{
					// the string cannot be simplified any further
					break;
				}
				fn = fn.substring(pos);
			}

			if (fn.toStdString() != review_info.filename)
			{
				str = "..." + fn.toStdString();
			}
		}

		if (columnId == 12)
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


void dm::DMReviewCanvas::sortOrderChanged(int newSortColumnId, bool isForwards)
{
	/* note the sort column is 1-based:
	 *		1: row number
	 *		2: mark (image)
	 *		3: zoom
	 *		4: size
	 *		5: aspect ratio
	 *		6: rectangle
	 *		7: overlap
	 *		8: filename
	 *		9: mime type
	 *		10: md5
	 *		11: duplicates
	 *		12: warning + error messages
	 */

	if (newSortColumnId < 1 or newSortColumnId > 12)
	{
		Log("cannot sort table on invalid column=" + std::to_string(newSortColumnId));
		return;
	}

	if (mri.size() == 0 or sort_idx.size() == 0 or mri.size() != sort_idx.size())
	{
		// nothing to sort!?
		Log("cannot sort table -- invalid number of rows...!?  (mri=" + std::to_string(mri.size()) + ", sort=" + std::to_string(sort_idx.size()) + ")");
		return;
	}

	const size_t class_idx = mri.at(0).class_idx;
	Log("sorting " + std::to_string(sort_idx.size()) + " rows of data for class #" + std::to_string(class_idx) + " using column #" + std::to_string(newSortColumnId));

	std::sort(sort_idx.begin(), sort_idx.end(),
			[&](size_t lhs_idx, size_t rhs_idx)
			{
				if (isForwards == false)
				{
					std::swap(lhs_idx, rhs_idx);
				}

				const auto & lhs_info = mri.at(lhs_idx);
				const auto & rhs_info = mri.at(rhs_idx);

				switch (newSortColumnId)
				{
					case 3:
					{
						const auto lhs_zoom = static_cast<double>(lhs_info.mat.cols) / std::max(1.0, static_cast<double>(lhs_info.r.width));
						const auto rhs_zoom = static_cast<double>(rhs_info.mat.cols) / std::max(1.0, static_cast<double>(rhs_info.r.width));

						if (lhs_zoom != rhs_zoom)
						{
							return lhs_zoom < rhs_zoom;
						}

						// if the zoom is the same, then sort by index
						return lhs_idx < rhs_idx;
					}
					case 2:
					case 4:
					{
						const auto lhs_area = lhs_info.r.width * lhs_info.r.height;
						const auto rhs_area = rhs_info.r.width * rhs_info.r.height;

						if (lhs_area != rhs_area)
						{
							return lhs_area < rhs_area;
						}
						// if the area is the same, then sort by index
						return lhs_idx < rhs_idx;
					}
					case 5:
					case 6:
					{
						const auto lhs_aspect_ratio = static_cast<double>(lhs_info.r.width) / static_cast<double>(lhs_info.r.height);
						const auto rhs_aspect_ratio = static_cast<double>(rhs_info.r.width) / static_cast<double>(rhs_info.r.height);

						if (lhs_aspect_ratio != rhs_aspect_ratio)
						{
							return lhs_aspect_ratio < rhs_aspect_ratio;
						}
						// if the aspect ratio is the same, then sort by index
						return lhs_idx < rhs_idx;
					}
					case 7:
					{
						if (lhs_info.overlap_sum != rhs_info.overlap_sum)
						{
							return lhs_info.overlap_sum < rhs_info.overlap_sum;
						}
						// if the overlap is the same, then sort by index
						return lhs_idx < rhs_idx;
					}
					case 8:
					{
						if (lhs_info.filename != rhs_info.filename)
						{
							return lhs_idx < rhs_idx;
						}
						// if the mime-type is the same, then sort by index
						return lhs_idx < rhs_idx;
					}
					case 9:
					{
						if (lhs_info.mime_type != rhs_info.mime_type)
						{
							return lhs_info.mime_type < rhs_info.mime_type;
						}
						// if the mime-type is the same, then sort by index
						return lhs_idx < rhs_idx;
					}
					case 10:
					{
						if (lhs_info.md5 != rhs_info.md5)
						{
							return lhs_info.md5 < rhs_info.md5;
						}
						// if the mime-type is the same, then sort by index
						return lhs_idx < rhs_idx;
					}
					case 11:
					{
						const auto & lhs_count = md5s.at(lhs_info.md5);
						const auto & rhs_count = md5s.at(rhs_info.md5);
						if (lhs_count != rhs_count)
						{
							return lhs_count < rhs_count;
						}
						if (lhs_info.md5 != rhs_info.md5)
						{
							return lhs_info.md5 < rhs_info.md5;
						}
						// if the mime-type is the same, then sort by index
						return lhs_idx < rhs_idx;
					}
					case 12:
					{
						if (lhs_info.errors.size()		!= rhs_info.errors.size())		return lhs_info.errors.size()	< rhs_info.errors.size();
						if (lhs_info.warnings.size()	!= rhs_info.warnings.size())	return lhs_info.warnings.size()	< rhs_info.warnings.size();

						// otherwise if there are the same number of errors and warning, sort by mark index
						return lhs_idx < rhs_idx;
					}
					default:
					{
						return lhs_idx < rhs_idx;
					}
				}
			});

	/*
	for (size_t i = 0; i < sort_idx.size(); i ++)
	{
		Log("sort result: at idx=" + std::to_string(i) + " val=" + std::to_string(sort_idx[i]));
	}
	*/

	return;
}
