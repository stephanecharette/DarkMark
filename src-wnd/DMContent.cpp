/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include "DarkMark.hpp"

#include "json.hpp"
using json = nlohmann::json;


dm::DMContent::DMContent() :
	canvas(*this),
	need_to_save(false),
	selected_mark(-1),
	scale_factor(1.0),
	image_directory(cfg().get_str("image_directory")),
	image_filename_index(0)
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

	const std::regex image_filename_regex(cfg().get_str("image_regex"), std::regex::icase | std::regex::nosubs | std::regex::optimize | std::regex::ECMAScript);

	DirectoryIterator iter(image_directory, true, "*", File::findFiles + File::ignoreHiddenFiles);
	while (iter.next())
	{
		File f = iter.getFile();
		const std::string filename = f.getFullPathName().toStdString();
		if (std::regex_match(filename, image_filename_regex))
		{
			image_filenames.push_back(filename);
		}
	}
	Log("number of images found in " + image_directory.getFullPathName().toStdString() + ": " + std::to_string(image_filenames.size()));
	std::sort(image_filenames.begin(), image_filenames.end());

	return;
}


dm::DMContent::~DMContent(void)
{
	if (need_to_save)
	{
		save_json();
		save_text();
	}

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

#if 0	// enable this to debug resizing
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
			" - "	+ std::to_string(1 + image_filename_index) + "/" + std::to_string(image_filenames.size()) +
			" - "	+ short_filename +
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
	const std::string darknet_cfg		= cfg().get_str("darknet_config"	);
	const std::string darknet_weights	= cfg().get_str("darknet_weights"	);
	const std::string darknet_names		= cfg().get_str("darknet_names"		);
	try
	{
		dmapp().darkhelp.reset(new DarkHelp(darknet_cfg, darknet_weights, darknet_names));
//		Log("neural network loaded in " + darkhelp().duration_string());
	}
	catch (const std::exception & e)
	{
		Log("failed to load darknet (cfg=" + darknet_cfg + ", weights=" + darknet_weights + ", names=" + darknet_names + "): " + e.what());
	}
	names = darkhelp().names;
	annotation_colours = darkhelp().annotation_colours;

	load_image(0);

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
//	Log("code=" + std::to_string(key.getKeyCode()) + " char=" + std::to_string(key.getTextCharacter()) + " description=" + key.getTextDescription().toStdString());

	const auto keycode = key.getKeyCode();

	const KeyPress key0 = KeyPress::createFromDescription("0");
	const KeyPress key9 = KeyPress::createFromDescription("9");

	int digit = -1;
	if (keycode >= key0.getKeyCode() and keycode <= key9.getKeyCode())
	{
		digit = keycode - key0.getKeyCode();
	}

	if (keycode == KeyPress::tabKey)
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
	else if (digit >= 0 and digit <= 9)
	{
		if (key.getModifiers().isCtrlDown())
		{
			digit += 10;
		}
		else if (key.getModifiers().isAltDown())
		{
			digit += 20;
		}

		// change the class for the selected mark
		if (selected_mark >= 0)
		{
			auto & m = marks[selected_mark];
			m.class_idx = digit;
			m.name = names.at(m.class_idx);
			m.description = names.at(m.class_idx);
			rebuild_image_and_repaint();
			need_to_save = true;
			return true; // event has been handled
		}
	}
	else if (keycode == KeyPress::homeKey)
	{
		load_image(0);
		return true;
	}
	else if (keycode == KeyPress::endKey)
	{
		load_image(image_filenames.size() - 1);
		return true;
	}
	else if (keycode == KeyPress::rightKey)
	{
		if (image_filename_index < image_filenames.size() - 1)
		{
			load_image(image_filename_index + 1);
		}
		return true;
	}
	else if (keycode == KeyPress::leftKey)
	{
		if (image_filename_index > 0)
		{
			load_image(image_filename_index - 1);
		}
		return true;
	}
	else if (keycode == KeyPress::pageUpKey)
	{
		// go to the previous available image with no marks
		while (image_filename_index > 0)
		{
			File f(image_filenames[image_filename_index]);
			f = f.withFileExtension(".json");
			if (count_marks_in_json(f) == 0)
			{
				break;
			}
			image_filename_index --;
		}
		load_image(image_filename_index);
		return true;

	}
	else if (keycode == KeyPress::pageDownKey)
	{
		// go to the next available image with no marks
		while (image_filename_index < image_filenames.size() - 1)
		{
			File f(image_filenames[image_filename_index]);
			f = f.withFileExtension(".json");
			if (count_marks_in_json(f) == 0)
			{
				break;
			}
			image_filename_index ++;
		}
		load_image(image_filename_index);
		return true;
	}
	else if (keycode == KeyPress::deleteKey or keycode == KeyPress::backspaceKey)
	{
		if (selected_mark >= 0)
		{
			auto iter = marks.begin() + selected_mark;
			marks.erase(iter);
			if (selected_mark >= (int)marks.size())
			{
				selected_mark --;
			}
			if (marks.empty())
			{
				selected_mark = -1;
			}
			rebuild_image_and_repaint();
			need_to_save = true;
			return true;
		}
	}
	else if (key.getTextCharacter() == 'r')
	{
		std::random_shuffle(image_filenames.begin(), image_filenames.end());
		load_image(0);
		return true;
	}

	return false;
}


dm::DMContent & dm::DMContent::load_image(const size_t new_idx)
{
	if (need_to_save)
	{
		save_json();
		save_text();
	}

	selected_mark	= -1;
	original_image	= cv::Mat();
	marks.clear();

	if (new_idx >= image_filenames.size())
	{
		image_filename_index = image_filenames.size() - 1;
	}
	else
	{
		image_filename_index = new_idx;
	}
	long_filename	= image_filenames.at(image_filename_index);
	short_filename	= File(long_filename).getFileName().toStdString();
	json_filename	= File(long_filename).withFileExtension(".json"	).getFullPathName().toStdString();
	text_filename	= File(long_filename).withFileExtension(".txt"	).getFullPathName().toStdString();

	try
	{
		Log("loading image " + long_filename);
		original_image = cv::imread(image_filenames.at(image_filename_index));

		bool success = load_json();
		if (not success)
		{
			success = load_text();
			if (success)
			{
				Log("imported " + text_filename + " instead of " + json_filename);
				need_to_save = true;
			}
		}

		if (not success)
		{
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
		}
	}
	catch(const std::exception & e)
	{
		Log("Error: exception caught while loading " + long_filename + ": " + e.what());
	}
	catch(...)
	{
		Log("Error: failed to load image " + long_filename);
	}

	if (marks.size() > 0)
	{
		selected_mark = 0;
	}

	resized();
	rebuild_image_and_repaint();

	return *this;
}


dm::DMContent & dm::DMContent::save_text()
{
	if (text_filename.empty() == false)
	{
		std::ofstream fs(text_filename);
		for (const auto & m : marks)
		{
			const cv::Rect2d r	= m.get_normalized_bounding_rect();
			const double w		= r.width;
			const double h		= r.height;
			const double x		= r.x + w / 2.0;
			const double y		= r.y + h / 2.0;
			fs << std::fixed << std::setprecision(10) << m.class_idx << " " << x << " " << y << " " << w << " " << h << std::endl;
		}
	}

	return *this;
}


dm::DMContent & dm::DMContent::save_json()
{
	if (json_filename.empty() == false)
	{
		json root;
		for (size_t idx = 0; idx < marks.size(); idx ++)
		{
			const auto & m = marks.at(idx);
			root["mark"][idx]["class_idx"	] = m.class_idx;
			root["mark"][idx]["name"		] = m.name;
			for (size_t point_idx = 0; point_idx < m.normalized_all_points.size(); point_idx ++)
			{
				const cv::Point2d & p = m.normalized_all_points.at(point_idx);
				root["mark"][idx]["points"][point_idx]["x"] = p.x;
				root["mark"][idx]["points"][point_idx]["y"] = p.y;

				// DarkMark doesn't use these integer values, but make them available for 3rd party software which wants to reads the .json file
				root["mark"][idx]["points"][point_idx]["int_x"] = (int)(std::round(p.x * (double)original_image.cols));
				root["mark"][idx]["points"][point_idx]["int_y"] = (int)(std::round(p.y * (double)original_image.rows));
			}
		}
		root["image"]["scale"]	= scale_factor;
		root["image"]["width"]	= original_image.cols;
		root["image"]["height"]	= original_image.rows;
		root["timestamp"]		= std::time(nullptr);
		root["version"]			= DARKMARK_VERSION;

		std::ofstream fs(json_filename);
		fs << root.dump(1, '\t') << std::endl;
	}

	need_to_save = false;

	return *this;
}


size_t dm::DMContent::count_marks_in_json(File & f)
{
	size_t result = 0;

	if (f.existsAsFile())
	{
		json root = json::parse(f.loadFileAsString().toStdString());
		result = root["mark"].size();
	}

	return result;
}


bool dm::DMContent::load_text()
{
	bool success = false;

	File f(text_filename);
	if (f.existsAsFile())
	{
		success = true;
		StringArray sa;
		f.readLines(sa);
		sa.removeEmptyStrings();
		for (auto iter = sa.begin(); iter != sa.end(); iter ++)
		{
			std::stringstream ss(iter->toStdString());
			int class_idx = 0;
			double x = 0.0;
			double y = 0.0;
			double w = 0.0;
			double h = 0.0;
			ss >> class_idx >> x >> y >> w >> h;
			Mark m(cv::Point2d(x, y), cv::Size2d(w, h), cv::Size(0, 0), class_idx);
			m.name = names.at(class_idx);
			m.description = m.name;
			marks.push_back(m);
		}
	}

	return success;
}


bool dm::DMContent::load_json()
{
	bool success = false;

	File f(json_filename);
	if (f.existsAsFile())
	{
		json root = json::parse(f.loadFileAsString().toStdString());

		for (size_t idx = 0; idx < root["mark"].size(); idx ++)
		{
			Mark m;
			m.class_idx = root["mark"][idx]["class_idx"];
			m.name = root["mark"][idx]["name"];
			m.description = m.name;
			m.normalized_all_points.clear();
			for (size_t point_idx = 0; point_idx < root["mark"][idx]["points"].size(); point_idx ++)
			{
				cv::Point2d p;
				p.x = root["mark"][idx]["points"][point_idx]["x"];
				p.y = root["mark"][idx]["points"][point_idx]["y"];
				m.normalized_all_points.push_back(p);
			}
			m.rebalance();
			marks.push_back(m);
		}

		// Sort the marks based on a gross (rounded) X and Y position of the midpoint.  This way when
		// the user presses TAB or SHIFT+TAB the marks appear in a consistent and predictable order.
		std::sort(marks.begin(), marks.end(),
			[](auto & lhs, auto & rhs)
			{
				const auto & p1 = lhs.get_normalized_midpoint();
				const auto & p2 = rhs.get_normalized_midpoint();

				const int y1 = std::round(15.0 * p1.y);
				const int y2 = std::round(15.0 * p2.y);

				if (y1 < y2) return true;
				if (y2 < y1) return false;

				// if we get here then y1 and y2 are the same, so now we compare x1 and x2

				const int x1 = std::round(15.0 * p1.x);
				const int x2 = std::round(15.0 * p2.x);

				if (x1 < x2) return true;

				return false;
			} );

		success = true;
	}

	return success;
}
