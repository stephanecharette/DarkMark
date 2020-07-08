/* DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include "DarkMark.hpp"
#include "json.hpp"
using json = nlohmann::json;


dm::ScrollField::ScrollField(DMContent & c) :
	CrosshairComponent(c),
	Thread("scrollfield loading thread"),
	content(c),
	line_width(0),
	triangle_size(cfg().get_int("scrollfield_marker_size"))
{
	setName("ScrollField");

	mouse_drag_is_enabled = false;

	bubble.addToDesktop(ComponentPeer::StyleFlags::windowIsTemporary + ComponentPeer::StyleFlags::windowIgnoresKeyPresses);

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
		map_idx_imagesets.clear();

		startThread(); // see ScrollField::run()
	}

	return;
}


void dm::ScrollField::run()
{
	field			= cv::Mat();
	resized_image	= cv::Mat();
	cached_image	= juce::Image();
	map_idx_imagesets.clear();

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

	if (content.sort_order == dm::ESort::kAlphabetical)
	{
		File previous_parent;

		const std::string parent = content.project_info.project_dir;
		
		// find all of the different image sets so we can display the white "arrow"
		for (size_t idx = 0; idx < number_of_images; idx ++)
		{
			if (threadShouldExit())
			{
				break;
			}

			File fn(content.image_filenames.at(idx));
			File dir = fn.getParentDirectory();
			if (previous_parent != dir)
			{
				// this is a new image set!
				Log("starting a new image set at index=" + std::to_string(idx) + ", fn=" + content.image_filenames.at(idx));

				std::string name = dir.getFullPathName().toStdString();
				if (name.size() > parent.size() + 1)
				{
					name.erase(0, parent.size() + 1);
				}
				map_idx_imagesets[idx] = name;
				previous_parent = dir;
			}
		}
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
			cv::line(field, cv::Point(0, idx), cv::Point(field.cols, idx), {0.0, 0.0, 0.0}, 1, cv::LINE_4); // start with a black background
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


void dm::ScrollField::mouseMove(const MouseEvent & event)
{
	if (map_idx_imagesets.empty() == false)
	{
		const double height				= static_cast<double>(resized_image.rows);
		const double number_of_images	= static_cast<double>(content.image_filenames.size());

		for (auto iter : map_idx_imagesets)
		{
			const double idx = iter.first;
			const int y = std::round(idx / number_of_images * height);

			if (event.x <= triangle_size and std::abs(event.y - y) <= triangle_size)
			{
				AttributedString str;
				str.setText(iter.second);
				str.setColour(Colours::white);
				str.setWordWrap(AttributedString::WordWrap::none);

				const auto r = getScreenBounds();
				bubble.showAt(juce::Rectangle<int>(r.getX(), r.getY() + y, 1, 1), str, 2000);
				break;
			}
		}
	}

	CrosshairComponent::mouseMove(event);

	return;
}


void dm::ScrollField::jump_to_location(const MouseEvent & event, const bool full_load)
{
	const double number_of_images	= static_cast<double>(content.image_filenames.size());
	const double height				= static_cast<double>(resized_image.rows);
	const double y					= std::max(0, event.getPosition().getY()); // if user is dragging up past the window border then Y can be negative, so cap it at zero
	const double factor				= height / number_of_images;

	// if the user has clicked on one of the markers, then we should snap to that index, so check that first
	if (full_load and event.getPosition().getX() <= triangle_size)
	{
		for (auto iter : map_idx_imagesets)
		{
			const double marker_idx	= iter.first;
			const double marker_y	= marker_idx * factor;

			if (std::abs(y - marker_y) <= triangle_size)
			{
				content.load_image(marker_idx, full_load);
				return;
			}
		}
	}

	// if we get here then the user is still scrolling, or did *not* click on a marker

	const size_t idx = std::round(y / factor);

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

		draw_triangles_at_image_sets();
		draw_marker_at_current_image();

		need_to_rebuild_cache_image = false;
	}

	return;
}


void dm::ScrollField::draw_triangles_at_image_sets()
{
	if (map_idx_imagesets.empty() == false and triangle_size > 0)
	{
		const double height				= static_cast<double>(resized_image.rows);
		const double number_of_images	= static_cast<double>(content.image_filenames.size());
		VContours contours;

		for (auto iter : map_idx_imagesets)
		{
			const double idx = iter.first;
			const int y = std::round(idx / number_of_images * height);

			const Contour contour =
			{
				cv::Point(0, y - triangle_size),
				cv::Point(triangle_size, y),
				cv::Point(0, y + triangle_size)
			};

			contours.push_back(contour);
		}

		cv::fillPoly(resized_image, contours, {255.0, 255.0, 255.0});
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
