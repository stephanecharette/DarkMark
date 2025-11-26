// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"


dm::DMReviewIoUWnd::DMReviewIoUWnd(DMContent & c) :
	DocumentWindow("DarkMark v" DARKMARK_VERSION " Review IoU", Colours::lightgrey, TitleBarButtons::closeButton),
	content(c)
{
	const int h = cfg().get_int("review_table_row_height");
	if (h > tlb.getRowHeight())
	{
		tlb.setRowHeight(h);
	}

	tlb.getHeader().addColumn("#"					, 1, 100, 30, -1, TableHeaderComponent::defaultFlags);
	tlb.getHeader().addColumn("thumbnail"			, 2, 100, 30, -1, TableHeaderComponent::defaultFlags);
	tlb.getHeader().addColumn("filename"			, 3, 100, 30, -1, TableHeaderComponent::defaultFlags);
	tlb.getHeader().addColumn("min IoU"				, 4, 100, 30, -1, TableHeaderComponent::defaultFlags);
	tlb.getHeader().addColumn("avg IoU"				, 5, 100, 30, -1, TableHeaderComponent::defaultFlags);
	tlb.getHeader().addColumn("max IoU"				, 6, 100, 30, -1, TableHeaderComponent::defaultFlags);
	tlb.getHeader().addColumn("annotations"			, 7, 100, 30, -1, TableHeaderComponent::defaultFlags);
	tlb.getHeader().addColumn("predictions"			, 8, 100, 30, -1, TableHeaderComponent::defaultFlags);
	tlb.getHeader().addColumn("matches"				, 9, 100, 30, -1, TableHeaderComponent::defaultFlags);
	tlb.getHeader().addColumn("extra pred."			, 10, 100, 30, -1, TableHeaderComponent::defaultFlags);
	tlb.getHeader().addColumn("missed pred."		, 11, 100, 30, -1, TableHeaderComponent::defaultFlags);
	tlb.getHeader().addColumn("differences"			, 12, 100, 30, -1, TableHeaderComponent::defaultFlags);
	tlb.getHeader().addColumn("extra predictions"	, 13, 100, 30, -1, TableHeaderComponent::defaultFlags);
	tlb.getHeader().addColumn("missed predictions"	, 14, 100, 30, -1, TableHeaderComponent::defaultFlags);
	// if changing columns, also update paintCell() below

	if (cfg().containsKey("iou_review_columns"))
	{
		tlb.getHeader().restoreFromString(cfg().get_str("iou_review_columns"));
	}
	else
	{
		tlb.getHeader().setStretchToFitActive(true);
	}

	tlb.setModel(this);

	setContentNonOwned		(&tlb, true	);
	setUsingNativeTitleBar	(true		);
	setResizable			(true, false);
	setDropShadowEnabled	(true		);

	setIcon(DarkMarkLogo());
	ComponentPeer *peer = getPeer();
	if (peer)
	{
		peer->setIcon(DarkMarkLogo());
	}

	if (cfg().containsKey("ReviewIoUWnd"))
	{
		restoreWindowStateFromString( cfg().getValue("ReviewIoUWnd") );
	}
	else
	{
		centreWithSize(600, 200);
	}

	setVisible(true);

	return;
}


dm::DMReviewIoUWnd::~DMReviewIoUWnd()
{
	cfg().setValue("iou_review_columns", tlb.getHeader().toString());
	cfg().setValue("ReviewIoUWnd", getWindowStateAsString());

	return;
}


void dm::DMReviewIoUWnd::closeButtonPressed()
{
	// close button

	dmapp().review_iou_wnd.reset(nullptr);

	return;
}


void dm::DMReviewIoUWnd::userTriedToCloseWindow()
{
	// ALT+F4

	dmapp().review_iou_wnd.reset(nullptr);

	return;
}


int dm::DMReviewIoUWnd::getNumRows()
{
	return v.size();
}


void dm::DMReviewIoUWnd::cellDoubleClicked(int rowNumber, int columnId, const MouseEvent & event)
{
	// rows are 0-based, columns are 1-based
	if (rowNumber >= 0 and rowNumber < (int)v.size())
	{
		const auto & info = v.at(rowNumber);

		const std::string image_filename = info.image_filename;

		if (image_filename.empty() == false)
		{
			for (size_t idx = 0; idx < content.image_filenames.size(); idx ++)
			{
				if (content.image_filenames.at(idx) == image_filename)
				{
					content.load_image(idx);
					break;
				}
			}
		}
	}

	return;
}


String dm::DMReviewIoUWnd::getCellTooltip(int rowNumber, int columnId)
{
	// rows are 0-based, columns are 1-based
	if (rowNumber >= 0 and rowNumber < (int)v.size())
	{
		if (columnId <= 3)
		{
			const auto & info = v.at(rowNumber);

			return "double click to open " + info.image_filename;
		}
	}

	return "";
}


void dm::DMReviewIoUWnd::paintRowBackground(Graphics & g, int rowNumber, int width, int height, bool rowIsSelected)
{
	Colour colour = Colours::white;

	// rows are 0-based
	if (rowNumber >= 0				or
		rowNumber < (int)v.size()	)
	{
		const auto & info = v.at(rowNumber);
		if (info.minimum_iou < 0.5 or info.average_iou < 0.5)
		{
			colour = Colours::lightpink;
		}
	}

	if (rowIsSelected)
	{
		colour = Colours::lightblue; // selected rows will have a blue background
	}

	g.fillAll(colour);

	// draw the cell bottom divider between rows
	g.setColour( Colours::black.withAlpha(0.5f));
	g.drawLine(0, height, width, height);

	return;
}


void dm::DMReviewIoUWnd::paintCell(Graphics & g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
	if (rowNumber < 0				or
		rowNumber >= (int)v.size()	or
		columnId < 1				or
		columnId > 14				)
	{
		// rows are 0-based, columns are 1-based
		return;
	}

	const auto & info = v.at(rowNumber);

	/* columns:
	 *		1: id
	 *		2: thumbnail
	 *		3: filename
	 *		4: min IoU
	 *		5: avg IoU
	 *		6: max IoU
	 *		7: annotations
	 *		8: predictions
	 *		9: matches
	 *		10: extra pred.
	 *		11: missed pred.
	 *		12: differences
	 *		13: extra pred. names
	 *		14: missed pred. names
	 */
	std::stringstream ss;
	ss << std::fixed << std::setprecision(4);

	if (columnId == 2)
	{
		if (info.thumbnail.empty() == false)
		{
			// draw the given thumbnail
			auto image = convert_opencv_mat_to_juce_image(info.thumbnail);
			g.drawImageWithin(image, 0, 0, width, height,
							  RectanglePlacement::xLeft				|
							  RectanglePlacement::yMid				|
							  RectanglePlacement::onlyReduceInSize	);
		}
	}
	else
	{
		switch (columnId)
		{
			case 1:
			{
				ss << info.number;
				break;
			}
			case 3:
			{
				std::string str = info.image_filename;

				// see if this string will fit in the cell, and if not attempt to shorten it
				const int max_len = 1.05 * width;
				auto font = g.getCurrentFont();
				String fn = info.image_filename;
				while (true)
				{
					const auto len = juce::GlyphArrangement::getStringWidthInt(font, fn);
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

				if (fn.toStdString() != info.image_filename)
				{
					str = "..." + fn.toStdString();
				}

				ss << str;
				break;
			}
			case 4: ss << info.minimum_iou;									break;
			case 5: ss << info.average_iou;									break;
			case 6: ss << info.maximum_iou;									break;
			case 7: ss << info.number_of_annotations;						break;
			case 8: ss << info.number_of_predictions;						break;
			case 9: ss << info.number_of_matches;							break;
			case 10: ss << info.number_of_predictions_without_annotations;	break;
			case 11: ss << info.number_of_annotations_without_predictions;	break;
			case 12: ss << info.number_of_differences;						break;
			case 13: ss << info.predictions_without_annotations;			break;
			case 14: ss << info.annotations_without_predictions;			break;
		}

		auto justification = Justification::centredLeft;
		if (columnId >= 4 and columnId <= 9)
		{
			justification = Justification::centredRight;
		}
		if (columnId >= 10 and columnId <= 12)
		{
			justification = Justification::centred;
		}

		std::string str = ss.str();
		if (columnId >= 4 and columnId <= 12 and str == "0")
		{
			// don't bother printing a bunch of zeros in the table
			str = "";
		}

		// draw the text and the right-hand-side dividing line between cells
		g.setColour( Colours::black );
		Rectangle<int> r(0, 0, width, height);
		g.drawFittedText(str, r.reduced(2), justification, 3);
	}

	// draw the divider on the right side of the column
	g.setColour( Colours::black.withAlpha( 0.5f ) );
	g.drawLine( width, 0, width, height );

	return;
}


void dm::DMReviewIoUWnd::sortOrderChanged(int newSortColumnId, bool isForwards)
{
	if (newSortColumnId < 1 or newSortColumnId > 13)
	{
		return;
	}

	std::sort(v.begin(), v.end(),
			[&](ReviewIoUInfo lhs, ReviewIoUInfo rhs) // not by reference on purpose since we may need to swap the values if this is a reverse sort
			{
				if (isForwards == false)
				{
					std::swap(lhs, rhs);
				}

				switch (newSortColumnId)
				{
					case 1:
					case 2:
					{
						return lhs.number < rhs.number;
					}
					case 3:
					{
						return lhs.image_filename < rhs.image_filename;
					}
					case 4:
					{
						if (lhs.minimum_iou != rhs.minimum_iou)
						{
							return lhs.minimum_iou < rhs.minimum_iou;
						}
						if (lhs.average_iou != rhs.average_iou)
						{
							return lhs.average_iou < rhs.average_iou;
						}
						return lhs.number < rhs.number;
					}
					case 5:
					{
						if (lhs.average_iou != rhs.average_iou)
						{
							return lhs.average_iou < rhs.average_iou;
						}
						if (lhs.minimum_iou != rhs.minimum_iou)
						{
							return lhs.minimum_iou < rhs.minimum_iou;
						}
						return lhs.number < rhs.number;
					}
					case 6:
					{
						if (lhs.maximum_iou != rhs.maximum_iou)
						{
							return lhs.maximum_iou < rhs.maximum_iou;
						}
						if (lhs.average_iou != rhs.average_iou)
						{
							return lhs.average_iou < rhs.average_iou;
						}
						if (lhs.minimum_iou != rhs.minimum_iou)
						{
							return lhs.minimum_iou < rhs.minimum_iou;
						}
						return lhs.number < rhs.number;
					}
					case 7:
					{
						if (lhs.number_of_annotations != rhs.number_of_annotations)
						{
							return lhs.number_of_annotations < rhs.number_of_annotations;
						}
						return lhs.number < rhs.number;
					}
					case 8:
					{
						if (lhs.number_of_predictions != rhs.number_of_predictions)
						{
							return lhs.number_of_predictions < rhs.number_of_predictions;
						}
						return lhs.number < rhs.number;
					}
					case 9:
					{
						if (lhs.number_of_matches != rhs.number_of_matches)
						{
							return lhs.number_of_matches < rhs.number_of_matches;
						}
						return lhs.number < rhs.number;
					}
					case 10:
					{
						if (lhs.number_of_predictions_without_annotations != rhs.number_of_predictions_without_annotations)
						{
							return lhs.number_of_predictions_without_annotations < rhs.number_of_predictions_without_annotations;
						}
						return lhs.number < rhs.number;
					}
					case 11:
					{
						if (lhs.number_of_annotations_without_predictions != rhs.number_of_annotations_without_predictions)
						{
							return lhs.number_of_annotations_without_predictions < rhs.number_of_annotations_without_predictions;
						}
						return lhs.number < rhs.number;
					}
					case 12:
					{
						if (lhs.number_of_differences != rhs.number_of_differences)
						{
							return lhs.number_of_differences < rhs.number_of_differences;
						}
						return lhs.number < rhs.number;
					}
					case 13:
					{
						if (lhs.number_of_predictions_without_annotations != rhs.number_of_predictions_without_annotations)
						{
							return lhs.number_of_predictions_without_annotations < rhs.number_of_predictions_without_annotations;
						}
						if (lhs.predictions_without_annotations != rhs.predictions_without_annotations)
						{
							return lhs.predictions_without_annotations < rhs.predictions_without_annotations;
						}
						return lhs.number < rhs.number;
					}
					case 14:
					{
						if (lhs.number_of_annotations_without_predictions != rhs.number_of_annotations_without_predictions)
						{
							return lhs.number_of_annotations_without_predictions < rhs.number_of_annotations_without_predictions;
						}
						if (lhs.annotations_without_predictions != rhs.annotations_without_predictions)
						{
							return lhs.annotations_without_predictions < rhs.annotations_without_predictions;
						}
						return lhs.number < rhs.number;
					}
					default:
					{
						return lhs.number < rhs.number;
					}
				}
			});

	return;
}
