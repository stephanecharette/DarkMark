/* DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include "DarkMark.hpp"
#include "json.hpp"
using json = nlohmann::json;


dm::ScrollField::ScrollField(DMContent & c) :
	Thread("scrollfield loading thread"),
	content(c),
	line_width(0)
{
	setName("ScrollField");

	mouse_drag_is_enabled = false;

	return;
}


dm::ScrollField::~ScrollField()
{
	if (isThreadRunning())
	{
		signalThreadShouldExit();
		stopThread(500);
	}

	return;
}


void dm::ScrollField::rebuild_entire_field_on_thread()
{
	if (isThreadRunning() == false)
	{
		field			= cv::Mat();
		resized_image	= cv::Mat();
		cached_image	= juce::Image();

		startThread();
	}

	return;
}


void dm::ScrollField::run()
{
	field			= cv::Mat();
	resized_image	= cv::Mat();
	cached_image	= juce::Image();

	const size_t number_of_images = content.image_filenames.size();
	const size_t number_of_classes = content.names.size();

	if (number_of_images < 1 or number_of_classes < 1)
	{
		// things are still loading, so return now and we'll try again later
		need_to_rebuild_cache_image = true;
		return;
	}

	field = cv::Mat(number_of_images, 200, CV_8UC3, {0.0, 0.0, 0.0});
	line_width = static_cast<double>(field.cols) / static_cast<double>(number_of_classes);

	for (size_t idx = 0; idx < number_of_images; idx ++)
	{
		if (threadShouldExit())
		{
			break;
		}

		update_index(idx);
	}


	need_to_rebuild_cache_image = true;
	repaint();

	return;
}


void dm::ScrollField::update_index(const size_t idx)
{
	if (idx >= content.image_filenames.size())
	{
		// nothing we can do with an index that is out of range
		return;
	}

	if (field.empty())
	{
		// if we don't have an image to draw into, then return immediately
		return;
	}

	const std::string & filename = content.image_filenames.at(idx);
	File f = File(filename).withFileExtension(".json");
	if (f.existsAsFile() == false)
	{
		// nothing to show for this index, draw a blank line
		cv::line(field, cv::Point(0, idx), cv::Point(field.cols, idx), {32.0, 32.0, 32.0}, 1, cv::LINE_4);
		return;
	}

	try
	{
		json root = json::parse(f.loadFileAsString().toStdString());

		if (root.value("completely_empty", false) == true)
		{
			// create a fake entry so "empty" images show up as marked
			root["mark"][0]["class_idx"] = content.empty_image_name_index;
		}

		if (root["mark"].size() == 0)
		{
			// What is going on here?  Why do we have a JSON, but it isn't marked "empty" and doesn't have any marks?
			Log("ScrollField: error detected while processing " + filename + " (no marks, but non-empty image?)");
			cv::line(field, cv::Point(0, idx), cv::Point(field.cols, idx), {0.0, 0.0, 255.0}, 1, cv::LINE_4);
		}
		else
		{
			for (auto m : root["mark"])
			{
				const int class_idx = m["class_idx"];
				const cv::Point p1(line_width * class_idx, idx);
				const cv::Point p2 = p1 + cv::Point(line_width, 0);
				cv::line(field, p1, p2, content.annotation_colours.at(class_idx), 1, cv::LINE_4);
			}
		}
	}
	catch (const std::exception & e)
	{
		// the .json file is broken somehow, so use a pure red line
		Log("ScrollField: error detected at idx #" + std::to_string(idx) + " (" + filename + "): " + e.what());
		cv::line(field, cv::Point(0, idx), cv::Point(field.cols, idx), {0.0, 0.0, 255.0}, 1, cv::LINE_4);
	}
	catch (...)
	{
		// the .json file is broken somehow, so use a pure red line
		Log("ScrollField: error detected at idx #" + std::to_string(idx) + " (" + filename + ")");
		cv::line(field, cv::Point(0, idx), cv::Point(field.cols, idx), {0.0, 0.0, 255.0}, 1, cv::LINE_4);
	}

	return;
}


void dm::ScrollField::mouseUp(const MouseEvent & event)
{
	jump_to_location(event, true);

	CrosshairComponent::mouseUp(event);

	return;
}


void dm::ScrollField::mouseDown(const MouseEvent & event)
{
	CrosshairComponent::mouseDown(event);

	if (event.mods.isPopupMenu())
	{
		content.create_popup_menu().showMenuAsync(PopupMenu::Options());
	}
	else
	{
		jump_to_location(event);
	}

	return;
}


void dm::ScrollField::mouseDrag(const MouseEvent & event)
{
	jump_to_location(event);

	CrosshairComponent::mouseDrag(event);

	return;
}


void dm::ScrollField::mouseWheelMove(const MouseEvent & event, const MouseWheelDetails & wheel)
{
	#if 0
	Log("DX=" + std::to_string(wheel.deltaX) +
		", DY=" + std::to_string(wheel.deltaY) +
		", inertial=" + std::to_string(wheel.isInertial) +
		", reversed=" + std::to_string(wheel.isReversed) +
		", smooth=" + std::to_string(wheel.isSmooth));
	#endif

	const int idx = content.image_filename_index;

	// Not obvious:
	//
	// - When deltaY is negative, that means the wheel is being scrolled DOWN, so we need to INCREASE the image index.
	// - When deltaY is positive, that means the wheel is being scrolled UP, meaning we DECREASE the image index (but not past zero).
	//
	int change = 0;
	if (wheel.deltaY < 0.0f)
	{
		change = 1;
	}
	else if (wheel.deltaY > 0.0f and idx > 0)
	{
		change = -1;
	}

	if (change != 0)
	{
		content.load_image(idx + change, false);

		startTimer(500); // request a callback -- in milliseconds -- at which point in time we'll fully load this image
	}

	CrosshairComponent::mouseWheelMove(event, wheel);

	return;
}


void dm::ScrollField::timerCallback()
{
	// if we get called, then we've been sitting on the same image for some time so go ahead and load the full image including all the marks
	stopTimer();

	content.load_image(content.image_filename_index);

	return;
}


void dm::ScrollField::jump_to_location(const MouseEvent & event, const bool full_load)
{
	// If the mouse is dragged beyond the border of the window, the Y will be negative,
	// which makes us wrap back around which is confusing, so ignore negative values.
	const double y		= std::max(0, event.getPosition().getY());
	const double h		= getHeight();
	const double ratio	= y / h;
	const size_t idx	= std::round(ratio * (double)content.image_filenames.size());

//	Log("Y=" + std::to_string(y) + " H=" + std::to_string(h) + " ratio=" + std::to_string(ratio) + " idx=" + std::to_string(idx));

	if (full_load or idx != content.image_filename_index)
	{
		content.load_image(idx, full_load);
	}

	return;
}


void dm::ScrollField::rebuild_cache_image()
{
	resized_image	= cv::Mat();
	cached_image	= juce::Image();

	if (field.empty())
	{
		rebuild_entire_field_on_thread();
	}
	else
	{
		const int w = getWidth();
		const int h = getHeight();

		cv::resize(field, resized_image, cv::Size(w, h));

		draw_marker_at_current_image();

		need_to_rebuild_cache_image = false;
	}

	return;
}


void dm::ScrollField::draw_marker_at_current_image()
{
	if (resized_image.empty() == false)
	{
		cv::Mat mat = resized_image.clone();

		const double number_of_images	= static_cast<double>(content.image_filenames.size());
		const double current_image_idx	= static_cast<double>(content.image_filename_index);
		const double ratio				= current_image_idx / number_of_images;
		const int y						= std::round(ratio * mat.rows);

		cv::line(mat, cv::Point(0, y), cv::Point(mat.cols - 1, y), {255.0, 255.0, 255.0}, 1, cv::LINE_4);

		cached_image = convert_opencv_mat_to_juce_image(mat);

		repaint();
	}

	return;
}
