/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include "DarkMark.hpp"


dm::DMContent::DMContent() :
	canvas(*this),
	selected_mark(-1),
	scale_factor(1.0)
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

	setWantsKeyboardFocus(true);

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
	const double window_width	= getWidth();
	const double window_height	= getHeight();
	if(	window_width	< 1.0 ||
		window_height	< 1.0 )
	{
		// window hasn't been created yet?
		return;
	}

	double image_width	= original_image.cols;
	double image_height	= original_image.rows;
	if (image_width		< 1.0 ||
		image_height	< 1.0 )
	{
		// image hasn't been loaded yet?
		image_width = 640;
		image_height = 480;
	}

	// the wider the window, the larger the cells should be
	const double pixels_per_cell				= std::floor(std::max(6.0, window_width / 85.0));	// 600=7, 800=9, 1000=11, 1600=18, ...

	const double min_number_of_cells			= 5.0;
	const double min_corner_width				= min_number_of_cells * (pixels_per_cell + 1);
	const double number_of_corner_windows		= corners.size();
	const double min_horizontal_spacer_height	= 2.0;

	// determine the size of the image once it is scaled
	const double width_ratio					= (window_width - min_corner_width) / image_width;
	const double height_ratio					= window_height / image_height;
	const double ratio							= std::min(width_ratio, height_ratio);
	const double new_image_width				= std::round(ratio * image_width);
	const double new_image_height				= std::round(ratio * image_height);

	// determine the size of each corner window
	const double max_corner_width				= std::floor(window_width - new_image_width);
	const double max_corner_height				= std::floor(window_height / number_of_corner_windows - (min_horizontal_spacer_height * (number_of_corner_windows - 1.0)));
	const double number_of_horizontal_cells		= std::floor(max_corner_width	/ (pixels_per_cell + 1));
	const double number_of_vertical_cells		= std::floor(max_corner_height	/ (pixels_per_cell + 1));
	const double new_corner_width				= number_of_horizontal_cells	* (pixels_per_cell + 1) - 1;
	const double new_corner_height				= number_of_vertical_cells		* (pixels_per_cell + 1) - 1;
	const double horizontal_spacer_height		= std::floor((window_height - new_corner_height * number_of_corner_windows) / (number_of_corner_windows - 1.0));

#if 0
	// enable this to debug resizing
	static size_t counter = 0;
	counter ++;
	if (counter % 10 == 0)
	{
		asm("int $3");
	}

	Log("resized:"
			" window size: "		+ std::to_string((int)window_width)		+ "x" + std::to_string((int)window_height)		+
			" new image size: "		+ std::to_string((int)new_image_width)	+ "x" + std::to_string((int)new_image_height)	+
			" min corner size: "	+ std::to_string((int)min_corner_width)	+ "x" + std::to_string((int)new_corner_height)	+
			" max corner size: "	+ std::to_string((int)max_corner_width)	+ "x" + std::to_string((int)max_corner_height)	+
			" new corner size: "	+ std::to_string((int)new_corner_width)	+ "x" + std::to_string((int)new_corner_height)	+
			" horizontal spacer: "	+ std::to_string((int)horizontal_spacer_height)											+
			" pixels per cell: "	+ std::to_string((int)pixels_per_cell)													+
			" horizontal cells: "	+ std::to_string((int)number_of_horizontal_cells)										+
			" vertical cells: "		+ std::to_string((int)number_of_vertical_cells)											);
#endif

	canvas.setBounds(0, 0, new_image_width, new_image_height);
	int y = 0;
	for (auto c : corners)
	{
		c->setBounds(new_image_width, y, new_corner_width, new_corner_height);
		y += new_corner_height + horizontal_spacer_height;
		c->cell_size = pixels_per_cell;
		c->cols = number_of_horizontal_cells;
		c->rows = number_of_vertical_cells;
	}

	// remember some of the important numbers so we don't have to re-calculate them later
	scaled_image_size = cv::Size(new_image_width, new_image_height);
	scale_factor = ratio;

	// update the window title to show the scale factor
	if (dmapp().wnd)
	{
		// since we're going to be messing around with the window title, make a copy of the original window name

		static const std::string original_title = dmapp().wnd->getName().toStdString();

		std::string title =
			original_title +
			" - "	+ std::string("barcode_3.jpg") +
			" - "	+ std::to_string(original_image.cols) +
			"x"		+ std::to_string(original_image.rows) +
			" - "	+ std::to_string(static_cast<int>(std::round(scale_factor * 100.0))) + "%";

		dmapp().wnd->setName(title);
	}

	return;
}


void dm::DMContent::start_darknet()
{
	Log("loading darknet neural network");
	dmapp().darkhelp.reset(new DarkHelp(cfg().get_str("darknet_config"), cfg().get_str("darknet_weights"), cfg().get_str("darknet_names")));
	Log("neural network loaded in " + darkhelp().duration_string());

//	const std::string filename = "/home/stephane/src/DarkHelp/build/barcode_3.jpg";
	const std::string filename = "/home/stephane/mailboxes/DSCN0435.JPG";
	Log("calling darkhelp to process the image " + filename);
	try
	{
		names = darkhelp().names;

		annotation_colours = darkhelp().annotation_colours;

		original_image = cv::imread(filename);
		darkhelp().predict(original_image);
		Log("darkhelp processed the image in " + darkhelp().duration_string());

		// convert the predictions into marks
		for (auto prediction : darkhelp().prediction_results)
		{
			Mark m(cv::Point2d(prediction.mid_x, prediction.mid_y), cv::Size2d(prediction.width, prediction.height), cv::Size(0, 0), prediction.best_class);
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


bool dm::DMContent::keyPressed(const KeyPress &key)
{
	const char c = key.getTextCharacter();

	if (key.getKeyCode() == KeyPress::tabKey)
	{
		if (marks.empty())
		{
			selected_mark = -1;
		}
		else
		{
			if (key.getModifiers().isShiftDown())
			{
				// select previous mark
				selected_mark --;
				if (selected_mark < 0)
				{
					selected_mark = marks.size() - 1;
				}
			}
			else
			{
				// select next mark
				selected_mark ++;
				if (selected_mark >= (int)marks.size())
				{
					selected_mark = 0;
				}
			}

			rebuild_image_and_repaint();
			return true; // event has been handled
		}
	}
	else if (c >= '0' and c <= '9')
	{
		// change the class for the selected mark
		auto & m = marks[selected_mark];
		m.class_idx = (c - '0');
		m.name = names.at(m.class_idx);
		m.description = names.at(m.class_idx);
		rebuild_image_and_repaint();
		return true; // event has been handled
	}

	return false;
}
