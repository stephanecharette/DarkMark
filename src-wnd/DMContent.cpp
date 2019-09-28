/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include "DarkMark.hpp"


dm::DMContent::DMContent() :
	canvas(*this),
	selected_mark(-1)
{
	corners.push_back(new DMCorner(*this, ECorner::kTL));
	corners.push_back(new DMCorner(*this, ECorner::kTR));
	corners.push_back(new DMCorner(*this, ECorner::kBR));
	corners.push_back(new DMCorner(*this, ECorner::kBL));

	addAndMakeVisible(canvas);

	for (auto c : corners)
	{
		addAndMakeVisible(c);
	}

	return;
}


dm::DMContent::~DMContent(void)
{
	for (auto c : corners)
	{
		delete c;
	}
	corners.clear();

	return;
}


void dm::DMContent::resized()
{
	const auto r = getLocalBounds();
	const int h = r.getHeight();

	FlexItem::Margin margin;
	margin.bottom = 1.0f;

	// We have 4 "corner" windows to display, and a limited height in which we can do it.  So start off by dividing the height by 4.
	const double quarter = static_cast<double>(h - 6) / 4.0; // -6 because we want a 2-pixel border between each corner window

	// We need to figure out how many cells can fit in this quarter height.  Ideally, we'd like each cell to be around 11 pixels in size.  (Must be an odd number.)
	double number_of_cells = 16.0;
	double cell_size = 0.0;
	while (number_of_cells > 5.0)
	{
		cell_size = std::round((quarter - number_of_cells) / number_of_cells);

		if (cell_size < 11.0)
		{
			number_of_cells --;
			continue;
		}

		break;
	}

	// Now that we know how many cells, and the size of each one, we can calculate the exact size each corner window requires.
	const int corner_size = std::round((cell_size + 1.0) * number_of_cells - 1);

	FlexBox col(FlexBox::Direction::column, FlexBox::Wrap::noWrap, FlexBox::AlignContent::stretch, FlexBox::AlignItems::stretch, FlexBox::JustifyContent::spaceBetween);
	for (int i = 0; i < 4; i ++)
	{
		corners[i]->cell_size = cell_size;
		corners[i]->cols = number_of_cells;
		corners[i]->rows = number_of_cells;
		col.items.add(FlexItem(*corners[i]).withMargin(margin).withMinHeight(corner_size).withMaxHeight(corner_size));
	}

	FlexBox row(FlexBox::Direction::row, FlexBox::Wrap::noWrap, FlexBox::AlignContent::stretch, FlexBox::AlignItems::stretch, FlexBox::JustifyContent::flexStart);
	row.items.add(FlexItem(canvas).withMargin(0).withFlex(1.0));
	row.items.add(FlexItem(col).withMargin(0).withMinWidth(corner_size).withMaxWidth(corner_size));

	row.performLayout(r);
}


void dm::DMContent::start_darknet()
{
	Log("loading darknet neural network");
	dmapp().darkhelp.reset(new DarkHelp(cfg().get_str("darknet_config"), cfg().get_str("darknet_weights"), cfg().get_str("darknet_names")));
	Log("neural network loaded in " + darkhelp().duration_string());

	const std::string filename = "/home/stephane/src/DarkHelp/build/barcode_3.jpg";
	Log("calling darkhelp to process the image " + filename);
	try
	{
		names = darkhelp().names;

		annotation_colours = darkhelp().annotation_colours;

		canvas.original_image = cv::imread(filename);
		darkhelp().predict(canvas.original_image);
		Log("darkhelp processed the image in " + darkhelp().duration_string());

		// convert the predictions into marks
		for (auto prediction : darkhelp().prediction_results)
		{
			Mark m(cv::Point2d(prediction.mid_x, prediction.mid_y), cv::Size2d(prediction.width, prediction.height), cv::Size(0, 0), prediction.best_class);
			m.colour = annotation_colours.at(m.class_idx % annotation_colours.size());
			m.name = names.at(m.class_idx);
			m.description = prediction.name;
			marks.push_back(m);
		}

		if (marks.empty())
		{
			selected_mark = -1;
		}
		else
		{
			selected_mark = 0;
		}
	}
	catch(...)
	{
		Log("Error: failed to process image " + filename);
	}

	canvas.need_to_rebuild_cache_image = true;
	canvas.repaint();
	for (size_t idx = 0; idx < 4; idx ++)
	{
		corners[idx]->original_image = canvas.original_image;
		corners[idx]->need_to_rebuild_cache_image = true;
		corners[idx]->repaint();
	}

	return;
}


void dm::DMContent::rebuild_image_and_repaint()
{
	canvas.need_to_rebuild_cache_image = true;
	canvas.repaint();

	for (auto c : corners)
	{
		c->need_to_rebuild_cache_image = true;
		c->repaint();
	}

	return;
}
