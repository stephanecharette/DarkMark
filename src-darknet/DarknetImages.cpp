// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#include <random>
#include "DarkMark.hpp"
#include "json.hpp"
using json = nlohmann::json;


namespace
{
	/** Force the cache images to be of a certain format:
	 *		0 = both
	 *		1 = JPG
	 *		2 = PNG
	 */
	static int cache_image_format = 0;

	/** Split a vector of filenames (or strings) into several parts.  When pieces is set to 0 (the default) an attempt
	 * will be made to determine the number of available cores.
	 * @since 2025-12-08
	 */
	std::vector<dm::VStr> split(const dm::VStr & vstr, size_t pieces = 0)
	{
		if (pieces == 0)
		{
			pieces = std::max(2U, std::thread::hardware_concurrency());
		}

		std::vector<dm::VStr> output;

		dm::Log("splitting a vector of " + std::to_string(vstr.size()) + " strings into " + std::to_string(pieces) + " new vectors...");

		if (pieces < 2)
		{
			output.push_back(vstr);
		}
		else
		{
			output.resize(pieces);
			for (auto & v : output)
			{
				v.reserve((vstr.size() + pieces) / pieces);
			}

			// round-robin distribute the filenames between all the vectors
			for (size_t idx = 0; idx < vstr.size(); idx ++)
			{
				output[idx % pieces].push_back(vstr[idx]);
			}
		}

		return output;
	}


	std::atomic<size_t> output_image_index = 0;
	size_t get_next_output_image_index()
	{
		return output_image_index ++;
	}


	cv::InterpolationFlags rnd_resize_method(std::default_random_engine & rng)
	{
		/* With very repetitive networks, such as those based on text using black-and-white images, or close-ups of barcodes,
		* or close-ups of parts on a machine when looking for defects, always using the same resize method actually prevents
		* objects from being detected when the images are processed with one of the other resize methods.  Not sure what kind
		* of subtle image artifact was being introduced with INTER_LINEAR, but it was shown that if all images in the training
		* dataset were resized using method "X", then for intererence the image also had to be resized using "X" and not "Y".
		* (Debugged at Hank.ai on 2023-07-07.)
		*
		* To prevent this from happening again, DarkMark now randomly uses one of several OpenCV resize methods.
		*/
		const std::vector<cv::InterpolationFlags> resize_methods =
		{
			cv::InterpolationFlags::INTER_LINEAR,	// this is the default -- fast but lower quality
			cv::InterpolationFlags::INTER_AREA,		// better when shrinking an image
			cv::InterpolationFlags::INTER_CUBIC,	// better when zooming an image
			cv::InterpolationFlags::INTER_NEAREST,	// similar to LINEAR
		};

		std::uniform_int_distribution<int> distribution(0, resize_methods.size() - 1);

		return resize_methods[distribution(rng)];
	}


	std::string rnd_image_filename(std::default_random_engine & rng, const std::string & basename)
	{
		if (cache_image_format == 1)
		{
			return basename + ".jpg";
		}
		if (cache_image_format == 2)
		{
			return basename + ".png";
		}

		std::uniform_int_distribution<int> distribution(0, 1);

		return basename + (distribution(rng) ? ".jpg" : ".png");
	}


	int rnd_jpg_quality(std::default_random_engine & rng)
	{
		// Sweet spot is ~70.  See https://www.ccoderun.ca/programming/2021-02-08_imwrite/

		std::uniform_int_distribution<int> distribution(60, 80);

		return distribution(rng);
	}


	bool is_jpg(const std::string & filename)
	{
		// "abc.jpg" -> extension starts at 3, which is length() - 4
		if (filename.length() > 4)
		{
			if (filename.rfind(".jpg") == filename.length() - 4)
			{
				return true;
			}
		}

		return false;
	}


	void save_image(const std::string & filename, cv::Mat & mat, std::default_random_engine & rng)
	{
		if (filename.empty() == false and mat.empty() == false)
		{
			if (is_jpg(filename))
			{
				cv::imwrite(filename, mat, {cv::ImwriteFlags::IMWRITE_JPEG_QUALITY, rnd_jpg_quality(rng)});
			}
			else
			{
				cv::imwrite(filename, mat, {cv::ImwriteFlags::IMWRITE_PNG_COMPRESSION, 0});
			}
		}

		return;
	}
}


void dm::DarknetWnd::find_all_annotated_images(ThreadWithProgressWindow & progress_window, VStr & annotated_images, VStr & skipped_images, size_t & number_of_marks, size_t & number_of_empty_images)
{
	output_image_index = 0;
	double work_done = 0.0;
	double work_to_do = content.image_filenames.size() + 1.0;
	progress_window.setProgress(0.0);
	progress_window.setStatusMessage(getText("Finding all images and annotations..."));

	annotated_images.clear();
	skipped_images.clear();

	for (const auto & filename : content.image_filenames)
	{
		work_done ++;
		progress_window.setProgress(work_done / work_to_do);

		File f = File(filename).withFileExtension(".json");

		// note how count_marks_in_json() lies about negative samples and returns "1" for empty images
		size_t count = content.count_marks_in_json(f);
		if (count)
		{
			annotated_images.push_back(filename);

			// cheat -- we'd like to know if this is actually a negative samples
			if (File(f).withFileExtension(".txt").getSize() == 0)
			{
				number_of_empty_images ++;
				count = 0;
			}
		}
		else
		{
			skipped_images.push_back(filename);
		}
		number_of_marks += count;
	}

	work_done = 0.0;
	work_to_do = skipped_images.size() + 1.0;
	progress_window.setProgress(0.0);
	progress_window.setStatusMessage(getText("Listing skipped images..."));

	std::shuffle(skipped_images.begin(), skipped_images.end(), get_random_engine());
	const std::string fn = File(info.project_dir).getChildFile("skipped_images.txt").getFullPathName().toStdString();
	std::ofstream fs_skipped(fn);
	for (const auto & image_filename : skipped_images)
	{
		work_done ++;
		progress_window.setProgress(work_done / work_to_do);

		fs_skipped << image_filename << std::endl;
	}

	if (info.image_type == "JPG")
	{
		cache_image_format = 1;
	}
	else if (info.image_type == "PNG")
	{
		cache_image_format = 2;
	}
	else
	{
		cache_image_format = 0; // both
	}

	return;
}


void dm::DarknetWnd::resize_images(ThreadWithProgressWindow & progress_window, const VStr & annotated_images, VStr & all_output_images, size_t & number_of_resized_images, size_t & number_of_images_not_resized, size_t & number_of_marks, size_t & number_of_empty_images)
{
	std::atomic<size_t> work_done = 0;
	const size_t work_to_do = annotated_images.size();

	const String sizing = String(info.image_width) + "x" + String(info.image_height);
	String text = getText("Resizing images to");
	#if DARKNET_GEN_SIMPLIFIED
		text = sizing + " " + text + "...";
	#else
		text += " " + sizing + "...";
	#endif

	progress_window.setProgress(0.0);
	progress_window.setStatusMessage(text);

	File dir = File(info.project_dir).getChildFile("darkmark_image_cache").getChildFile("resize");
	const std::string dir_name = dir.getFullPathName().toStdString();
	dir.createDirectory();
	if (dir.isDirectory() == false)
	{
		throw std::runtime_error("Failed to create directory " + dir_name + ".");
	}

	auto & rng = get_random_engine();

	const cv::Size desired_image_size(info.image_width, info.image_height);

	std::ofstream resized_txt(dir_name + "/resized.txt");
	resized_txt << "WARNING: multiple threads write to this file at the same time." << std::endl;

	// need to protect "all_output_images" and several other items from being modified across multiple threads at the same time
	std::mutex resize_images_mutex;
	const auto & split_image_filenames = split(annotated_images);
	std::string error_detected;

	const auto resize_worker_lambda = [&, this](const size_t thread_idx)
	{
		std::string last_image_filename = "?";

		try
		{
			for (const auto & original_image : split_image_filenames[thread_idx])
			{
				if (not error_detected.empty())
				{
					break;
				}

				work_done ++;

				last_image_filename = original_image;

				std::stringstream ss;
				ss << dir_name << "/" << std::setfill('0') << std::setw(8) << get_next_output_image_index();
				const std::string output_base_name = ss.str();
				const std::string output_image = rnd_image_filename(rng, output_base_name);
				const std::string output_label = output_base_name + ".txt";

				// first we create the resized image file
				cv::Mat mat = cv::imread(original_image);
				if (mat.empty())
				{
					// something has gone *very* wrong if we cannot read the image
					Log(original_image + " (" + std::to_string(mat.cols) + "x" + std::to_string(mat.rows) + ")");
					throw std::runtime_error("failed to open or read the image " + original_image);
				}

				cv::Mat dst;
				if (mat.cols != desired_image_size.width or mat.rows != desired_image_size.height)
				{
					cv::resize(mat, dst, desired_image_size, 0, 0, rnd_resize_method(rng));
					number_of_resized_images ++;
				}
				else
				{
					dst = mat;
					number_of_images_not_resized ++;
				}

				save_image(output_image, dst, rng);

				// next we copy the annoations in the .txt file
				File txt = File(original_image).withFileExtension(".txt");
				const bool success = txt.copyFileTo(File(output_label));
				if (not success)
				{
					throw std::runtime_error("Failed to copy " + txt.getFullPathName().toStdString() + ".");
				}

				// beyond this point we update the things that must be protected by the mutex lock

				std::lock_guard lock(resize_images_mutex);
				all_output_images.push_back(output_image);
				resized_txt
					<< "#" << thread_idx << ": "
					<< original_image
					<< " [" << mat.cols << "x" << mat.rows << "] -> "
					<< output_image
					<< " [" << dst.cols << "x" << dst.rows << "]"
					<< std::endl;

				if (txt.getSize() == 0)
				{
					number_of_empty_images ++;
				}
				else
				{
					json root = json::parse(txt.withFileExtension(".json").loadFileAsString().toStdString());
					number_of_marks += root["mark"].size();
				}
			}
		}
		catch (const std::exception & e)
		{
			std::string msg = "error in resize thread #" + std::to_string(thread_idx) + " while processing \"" + last_image_filename + "\": " + e.what();
			Log(msg);
			if (error_detected.empty())
			{
				error_detected.swap(msg);
			}
		}
	};

	// start multiple threads running the resize worker lambda, then we wait for all of them to be done

	VThreads vthreads;
	for (size_t idx = 0; idx < split_image_filenames.size(); idx ++)
	{
		Log("creating image resize thread #" + std::to_string(idx) + " to handle " + std::to_string(split_image_filenames[idx].size()) + " images...");
		vthreads.emplace_back(resize_worker_lambda, idx);
	}

	while (work_done < work_to_do and error_detected.empty())
	{
		progress_window.setProgress(work_done / static_cast<double>(work_to_do));
		std::this_thread::sleep_for(std::chrono::milliseconds(750));
	}

	for (auto & t : vthreads)
	{
		t.join();
	}

	if (not error_detected.empty())
	{
		throw std::runtime_error(error_detected);
	}

	return;
}


void dm::DarknetWnd::tile_images(ThreadWithProgressWindow & progress_window, const VStr & annotated_images, VStr & all_output_images, size_t & number_of_marks, size_t & number_of_tiles_created, size_t & number_of_empty_images)
{
	std::atomic<size_t> work_done = 0;
	const size_t work_to_do = annotated_images.size();

	const String sizing = String(info.image_width) + "x" + String(info.image_height);
	String text = getText("Tiling images to");
	#if DARKNET_GEN_SIMPLIFIED
		text = sizing + " " + text + "...";
	#else
		text += " " + sizing + "...";
	#endif

	progress_window.setProgress(0.0);
	progress_window.setStatusMessage(text);

	File dir = File(info.project_dir).getChildFile("darkmark_image_cache").getChildFile("tiles");
	const std::string dir_name = dir.getFullPathName().toStdString();
	dir.createDirectory();
	if (dir.isDirectory() == false)
	{
		throw std::runtime_error("Failed to create directory " + dir_name + ".");
	}

	auto & rng = get_random_engine();

	const cv::Size desired_tile_size(info.image_width, info.image_height);

	std::ofstream tiles_txt(dir_name + "/tiles.txt");
	tiles_txt << "WARNING: multiple threads write to this file at the same time." << std::endl;

	std::mutex tile_images_mutex;
	const auto & split_image_filenames = split(annotated_images);
	std::string error_detected;

	const auto tile_worker_lambda = [&, this](const size_t thread_idx)
	{
		std::string last_image_filename = "?";

		try
		{
			for (const auto & original_image : split_image_filenames[thread_idx])
			{
				if (not error_detected.empty())
				{
					break;
				}

				work_done ++;

				last_image_filename = original_image;

				// first thing we'll do is read the annotations for this image
				json root = json::parse(File(original_image).withFileExtension(".json").loadFileAsString().toStdString());

				cv::Mat mat = cv::imread(original_image);
				if (mat.empty())
				{
					// something has gone *very* wrong if we cannot read the image
					Log(original_image + " (" + std::to_string(mat.cols) + "x" + std::to_string(mat.rows) + ")");
					throw std::runtime_error("failed to open or read the image " + original_image);
				}

				const double horizontal_factor		= static_cast<double>(mat.cols) / static_cast<double>(desired_tile_size.width);
				const double vertical_factor		= static_cast<double>(mat.rows) / static_cast<double>(desired_tile_size.height);
				const size_t horizontal_tiles_count	= std::round(std::max(1.0, horizontal_factor	));
				const size_t vertical_tiles_count	= std::round(std::max(1.0, vertical_factor		));
				const double cell_width				= static_cast<double>(mat.cols) / static_cast<double>(horizontal_tiles_count);
				const double cell_height			= static_cast<double>(mat.rows) / static_cast<double>(vertical_tiles_count);

				std::stringstream messages;
				messages
					<< "#" << thread_idx << ": "
					<< original_image << " [" << mat.cols << "x" << mat.rows << "]"
					<< " -> [" << horizontal_tiles_count << "x" << vertical_tiles_count << "]"
					<< " -> [" << cell_width << "x" << cell_height << "]"
					<< std::endl;

				if (info.resize_images and horizontal_tiles_count == 1 and vertical_tiles_count == 1)
				{
					// this image only has 1 tile, and we already have it since "resize" is enabled, so skip to the next image
					std::lock_guard lock(tile_images_mutex);
					tiles_txt
						<< messages.str()
						<< "#" << thread_idx << ": "
						<< "skipped (single tile)" << std::endl;
					continue;
				}

//				Log(original_image + " (" + std::to_string(mat.cols) + "x" + std::to_string(mat.rows) + ") needs to be tiled as " + std::to_string(horizontal_tiles_count) + "x" + std::to_string(vertical_tiles_count) + " tiles each measuring " + std::to_string(desired_tile_size.width) + "x" + std::to_string(desired_tile_size.height));

				for (size_t y_idx = 0; y_idx < vertical_tiles_count; y_idx ++)
				{
					for (size_t x_idx = 0; x_idx < horizontal_tiles_count; x_idx ++)
					{
						int tile_x = std::round(cell_width	* static_cast<double>(x_idx));
						int tile_y = std::round(cell_height	* static_cast<double>(y_idx));
						int tile_w = std::round(cell_width);
						int tile_h = std::round(cell_height);
//						Log("-> old tile: x=" + std::to_string(tile_x) + " y=" + std::to_string(tile_y) + " w=" + std::to_string(tile_w) + " h=" + std::to_string(tile_h));

						// if a cell is smaller than our desired tile, then we can grab a few more pixels to fill out the tile and get it closer to the desired network size
						int delta = desired_tile_size.width - tile_w;
						tile_x -= delta / 2;
						tile_w += delta;

						// if we moved beyond the right border then move the X coordinate back
						if (tile_x + tile_w >= mat.cols)
						{
							tile_x = mat.cols - tile_w;
						}

						// if we moved beyond the *left* border, then reset to zero
						if (tile_x < 0)
						{
							tile_x = 0;
						}

						// make sure the cell width doesn't extend beyond the right border
						if (tile_x + tile_w >= mat.cols)
						{
							tile_w = (mat.cols - tile_x);
						}

						delta = desired_tile_size.height - tile_h;
						tile_y -= delta / 2;
						tile_h += delta;

						// if we moved beyond the bottom border then move the Y coordinate back
						if (tile_y + tile_h >= mat.rows)
						{
							tile_y = mat.rows - tile_h;
						}

						// if we moved beyond the *top* border, then reset to zero
						if (tile_y < 0)
						{
							tile_y = 0;
						}

						// make sure the cell width doesn't extend beyond the bottom border
						if (tile_y + tile_h >= mat.rows)
						{
							tile_h = (mat.rows - tile_y);
						}
//						Log("-> new tile: x=" + std::to_string(tile_x) + " y=" + std::to_string(tile_y) + " w=" + std::to_string(tile_w) + " h=" + std::to_string(tile_h));

						const cv::Rect tile_rect(tile_x, tile_y, tile_w, tile_h);
						cv::Mat tile = mat(tile_rect);

						std::stringstream ss;
						ss << dir_name << "/" << std::setfill('0') << std::setw(8) << get_next_output_image_index();
						const std::string output_base_name = ss.str();
						const std::string output_image = rnd_image_filename(rng, output_base_name);
						const std::string output_label = output_base_name + ".txt";

						save_image(output_image, tile, rng);

						// now re-create the .txt file with the appropriate annotations for this new tile
						//
						// we know our tile is from (tile_x, tile_y, tile_w, tile_h), so include any annotations within those bounds
						std::ofstream fs_txt(output_label);
						fs_txt << std::fixed << std::setprecision(10);

						size_t number_of_annotations = 0;
						for (auto j : root["mark"])
						{
							const cv::Rect annotation_rect(j["rect"]["int_x"], j["rect"]["int_y"], j["rect"]["int_w"], j["rect"]["int_h"]);
							const cv::Rect intersection = annotation_rect & tile_rect;
							if (intersection.area() > 0)
							{
								const int class_idx = j["class_idx"];
								int x = j["rect"]["int_x"];
								int y = j["rect"]["int_y"];
								int w = j["rect"]["int_w"];
								int h = j["rect"]["int_h"];

								if (x < tile_rect.x)
								{
									// X is beyond the left border, we need to move it to the right
									const int delta_x = tile_rect.x - x;
									x += delta_x;
									w -= delta_x;
								}
								if (y < tile_rect.y)
								{
									const int delta_y = tile_rect.y - y;
									y += delta_y;
									h -= delta_y;
								}
								if (x + w > tile_rect.x + tile_rect.width)
								{
									w = tile_rect.x + tile_rect.width - x;
								}
								if (y + h > tile_rect.y + tile_rect.height)
								{
									h = tile_rect.y + tile_rect.height - y;
								}

								// ignore extremely tiny slices
								if (w >= 10 and h >= 10)
								{
									// bring all the coordinates back down to zero
									x -= tile_rect.x;
									y -= tile_rect.y;

									const double normalized_w = static_cast<double>(w) / static_cast<double>(tile.cols);
									const double normalized_h = static_cast<double>(h) / static_cast<double>(tile.rows);
									const double normalized_x = static_cast<double>(x) / static_cast<double>(tile.cols) + normalized_w / 2.0;
									const double normalized_y = static_cast<double>(y) / static_cast<double>(tile.rows) + normalized_h / 2.0;
									fs_txt << class_idx << " " << normalized_x <<  " " << normalized_y << " " << normalized_w << " " << normalized_h << std::endl;
									number_of_annotations ++;
								}
							}
						}

						std::lock_guard lock(tile_images_mutex);
						all_output_images.push_back(output_image);

						if (number_of_annotations == 0)
						{
							number_of_empty_images ++;
						}
						number_of_marks += number_of_annotations;
						number_of_tiles_created ++;

						tiles_txt
							<< messages.str()
							<< "#" << thread_idx << ": "
							<< output_image
							<< " [" << tile.cols << "x" << tile.rows << "]"
							<< " [" << number_of_annotations << "/" << root["mark"].size() << "]"
							<< std::endl;
					}
				}
			}
		}
		catch (const std::exception & e)
		{
			std::string msg = "error in tile thread #" + std::to_string(thread_idx) + " while processing \"" + last_image_filename + "\": " + e.what();
			Log(msg);
			if (error_detected.empty())
			{
				error_detected.swap(msg);
			}
		}
	};

	// start multiple threads running the tile lambda, then we wait for all of them to be done

	VThreads vthreads;
	for (size_t idx = 0; idx < split_image_filenames.size(); idx ++)
	{
		Log("creating tile thread #" + std::to_string(idx) + " to handle " + std::to_string(split_image_filenames[idx].size()) + " images...");
		vthreads.emplace_back(tile_worker_lambda, idx);
	}

	while (work_done < work_to_do and error_detected.empty())
	{
		progress_window.setProgress(work_done / static_cast<double>(work_to_do));
		std::this_thread::sleep_for(std::chrono::milliseconds(750));
	}

	for (auto & t : vthreads)
	{
		t.join();
	}

	if (not error_detected.empty())
	{
		throw std::runtime_error(error_detected);
	}

	return;
}


void dm::DarknetWnd::random_zoom_images(ThreadWithProgressWindow & progress_window, const VStr & annotated_images, VStr & all_output_images, size_t & number_of_marks, size_t & number_of_zooms_created, size_t & number_of_empty_images)
{
	std::atomic<size_t> work_done = 0;
	const size_t work_to_do = annotated_images.size();
	progress_window.setProgress(0.0);
	progress_window.setStatusMessage(getText("Random image crop and zoom..."));

	File dir = File(info.project_dir).getChildFile("darkmark_image_cache").getChildFile("zoom");
	const std::string dir_name = dir.getFullPathName().toStdString();
	dir.createDirectory();
	if (dir.isDirectory() == false)
	{
		throw std::runtime_error("Failed to create directory " + dir_name + ".");
	}

	std::ofstream zoom_txt(dir_name + "/zoom.txt");
	zoom_txt << "WARNING: multiple threads write to this file at the same time." << std::endl;

	const cv::Size desired_size(info.image_width, info.image_height);

	/* Images must be larger than the final desired size for us to "zoom in".
	 * This variable describes the minimum size we need for us to work with the image.
	 */
	const cv::Size large_size(
			std::round(1.25f * desired_size.width),
			std::round(1.25f * desired_size.height));

	auto & rng = get_random_engine();

	std::mutex random_zoom_mutex;
	const auto & split_image_filenames = split(annotated_images);
	std::string error_detected;

	const auto random_zoom_worker_lambda = [&, this](const size_t thread_idx)
	{
		std::string last_image_filename = "?";

		try
		{
			for (const auto & original_image : split_image_filenames[thread_idx])
			{
				if (not error_detected.empty())
				{
					break;
				}

				work_done ++;

				last_image_filename = original_image;

				cv::Mat original_mat = cv::imread(original_image);
				if (original_mat.empty())
				{
					// something has gone *very* wrong if we cannot read the image
					Log(original_image + " (" + std::to_string(original_mat.cols) + "x" + std::to_string(original_mat.rows) + ")");
					throw std::runtime_error("failed to open or read the image " + original_image);
				}

				if (original_mat.cols < large_size.width or original_mat.rows < large_size.height)
				{
					std::lock_guard lock(random_zoom_mutex);
					zoom_txt
						<< "#" << thread_idx << ": "
						<< original_image
						<< " [" << original_mat.cols << "x" << original_mat.rows << "]"
						<< " -> skipped (image too small)" << std::endl;
					continue;
				}

				const cv::Rect original_rect(0, 0, original_mat.cols, original_mat.rows);
				json root = json::parse(File(original_image).withFileExtension(".json").loadFileAsString().toStdString());
				std::vector<cv::Point> points_of_interest;
				for (auto j : root["mark"])
				{
					const int x = j["rect"]["int_x"];
					const int y = j["rect"]["int_y"];
					const int w = j["rect"]["int_w"];
					const int h = j["rect"]["int_h"];

					for (const cv::Point & p :
						{
							cv::Point(x + 0, y + 0),	// TL
							cv::Point(x + w, y + 0),	// TR
							cv::Point(x + w, y + h),	// BR
							cv::Point(x + 0, y + h),	// BL
							cv::Point(x + w/2, y + h/2)	// middle
						})
					{
						if (original_rect.contains(p))
						{
							points_of_interest.push_back(p);
						}
					}
				}

				// keep creating cropped/zoomed images as long as we're finding new parts of the image that we didn't previously cover
				std::vector<cv::Rect> all_previous_rectangles;
				size_t failed_consecutive_attempts = 0;
				std::stringstream messages;

				while (failed_consecutive_attempts < 5)
				{
					/* The amount we're going to "zoom in" depends on exactly how big the image is compared to the final size.
					 * This value is the "factor" by which we multiply the desired image size.  We need to make sure that both
					 * the horizontal and vertical values can be satisfied.
					 */
					const float horizontal_factor	= static_cast<float>(original_mat.cols) / static_cast<float>(desired_size.width);
					const float vertical_factor		= static_cast<float>(original_mat.rows) / static_cast<float>(desired_size.height);
					const float min_factor			= std::min(horizontal_factor, vertical_factor);

					std::uniform_real_distribution<float> uni_f(0.8f, min_factor);
					const float factor = uni_f(rng);

					// This describes the size of the RoI we're going to carve out of the original image mat.
					const cv::Size size(
							std::round(factor * desired_size.width),
							std::round(factor * desired_size.height));

					// Now that we know the size, we can create the rectangle which is used to carve out the RoI.
					cv::Rect roi(cv::Point(0, 0), size);

					// Now figure out how much room remains outside of the RoI, and randomly choose some spacing to assign.
					const int delta_h = original_mat.cols - roi.width;
					const int delta_v = original_mat.rows - roi.height;
					std::uniform_int_distribution<int> uni_h(0, delta_h);
					std::uniform_int_distribution<int> uni_v(0, delta_v);
					roi.x = uni_h(rng);
					roi.y = uni_v(rng);

					// See if the middle point of this RoI was already covered by a previous rectangle.
					bool continue_crop_and_zoom = true;
					const cv::Point middle_point(roi.x + roi.width/2, roi.y + roi.height/2);
					for (const auto & r : all_previous_rectangles)
					{
						if (r.contains(middle_point))
						{
							// we've already covered this point
							continue_crop_and_zoom = false;
							break;
						}
					}

					if (continue_crop_and_zoom == false)
					{
						// before we give up on this RoI, see if it covers one of the remaining points of interest
						for (const auto & p : points_of_interest)
						{
							if (roi.contains(p))
							{
								messages << "#" << thread_idx << ": " << original_image << "-> adding RoI because it includes point-of-interest x=" << p.x << " y=" << p.y << std::endl;
								continue_crop_and_zoom = true;
								break;
							}
						}
					}

					if (continue_crop_and_zoom == false)
					{
						messages
							<< "#" << thread_idx << ": "
							<< original_image
							<< " -> skipped RoI [x=" << roi.x << " y=" << roi.y << " w=" << roi.width << " h=" << roi.height << "] due to overlap" << std::endl;
						failed_consecutive_attempts ++;
						continue;
					}

					// ...otherise, if we get here then we seem to be covering a new part of the image
					failed_consecutive_attempts = 0;
					all_previous_rectangles.push_back(roi);
					messages
						<< "#" << thread_idx << ": "
						<< original_image
						<< " -> creating RoI from [x=" << roi.x << " y=" << roi.y << " w=" << roi.width << " h=" << roi.height << "]" << std::endl;

					// remove from "points-of-interest" any points located within the RoI we've just created
					auto iter = points_of_interest.begin();
					while (iter != points_of_interest.end())
					{
						const auto & p = *iter;
						if (roi.contains(p))
						{
							iter = points_of_interest.erase(iter);
						}
						else
						{
							iter ++;
						}
					}

					// Crop the original image, and at the same time resize it to be the exact dimensions we need.
					cv::Mat output_mat;
					cv::resize(original_mat(roi), output_mat, desired_size, 0.0, 0.0, rnd_resize_method(rng));

					std::stringstream ss;
					ss << dir_name << "/" << std::setfill('0') << std::setw(8) << get_next_output_image_index();
					const std::string output_base_name = ss.str();
					const std::string output_image = rnd_image_filename(rng, output_base_name);
					const std::string output_label = output_base_name + ".txt";

					save_image(output_image, output_mat, rng);

					// crop the annotations to match the image, and re-calculate the values for the .txt file.

					std::ofstream fs_txt(output_label);
					fs_txt << std::fixed << std::setprecision(10);
					size_t number_of_annotations = 0;
					for (auto j : root["mark"])
					{
						int x = j["rect"]["int_x"];
						int y = j["rect"]["int_y"];
						int w = j["rect"]["int_w"];
						int h = j["rect"]["int_h"];
						const cv::Rect annotation_rect(x, y, w, h);
						const cv::Rect intersection = annotation_rect & roi;
						if (intersection.area() == 0)
						{
							// this annotation does not appear in our new image
							continue;
						}

						const int class_idx = j["class_idx"];

						if (x < roi.x)
						{
							// X is beyond the left border, we need to move it to the right
							int delta = roi.x - x;
							x += delta;
							w -= delta;
						}
						if (y < roi.y)
						{
							// Y is beyond the top border, we need to move it down
							int delta = roi.y - y;
							y += delta;
							h -= delta;
						}
						if (x + w > roi.x + roi.width)
						{
							// width is beyond the right border
							w = roi.x + roi.width - x;
						}
						if (y + h > roi.y + roi.height)
						{
							// height is beyond the bottom border
							h = roi.y + roi.height - y;
						}

						if (w < 5 or h < 5)
						{
							// ignore extremely tiny slices of annotations
							continue;
						}

						// bring all the coordinates back down to zero
						x -= roi.x;
						y -= roi.y;

						const double normalized_w = static_cast<double>(w) / static_cast<double>(roi.width	);
						const double normalized_h = static_cast<double>(h) / static_cast<double>(roi.height	);
						const double normalized_x = static_cast<double>(x) / static_cast<double>(roi.width	) + normalized_w / 2.0;
						const double normalized_y = static_cast<double>(y) / static_cast<double>(roi.height	) + normalized_h / 2.0;

						fs_txt << class_idx << " " << normalized_x <<  " " << normalized_y << " " << normalized_w << " " << normalized_h << std::endl;
						number_of_annotations ++;
					}

					// beyond this point we update the things that must be protected by the mutex lock

					std::lock_guard lock(random_zoom_mutex);
					all_output_images.push_back(output_image);

					zoom_txt
						<< messages.str()
						<< "#" << thread_idx << ": "
						<< original_image
						<< " [" << original_mat.cols << "x" << original_mat.rows << "]"
						<< " -> " << output_image
						<< " [f=" << factor
						<< " x=" << roi.x
						<< " y=" << roi.y
						<< " w=" << roi.width
						<< " h=" << roi.height
						<< "]"
						<< " -> [" << output_mat.cols << "x" << output_mat.rows << "]"
						<< " [" << number_of_annotations << "/" << root["mark"].size() << "]"
						<< std::endl;

					if (number_of_annotations == 0)
					{
						number_of_empty_images ++;
					}
					number_of_marks += number_of_annotations;
					number_of_zooms_created ++;
				}

				if (points_of_interest.empty() == false)
				{
					std::lock_guard lock(random_zoom_mutex);
					zoom_txt << "#" << thread_idx << ": " << original_image << " -> still had " << points_of_interest.size() << " items remaining in the points-of-interest" << std::endl;
				}
			}
		}
		catch (const std::exception & e)
		{
			std::string msg = "error in crop & zoom thread #" + std::to_string(thread_idx) + " while processing \"" + last_image_filename + "\": " + e.what();
			Log(msg);
			if (error_detected.empty())
			{
				error_detected.swap(msg);
			}
		}
	};

	// start multiple threads running the crop and zoom worker lambda, then we wait for all of them to be done

	VThreads vthreads;
	for (size_t idx = 0; idx < split_image_filenames.size(); idx ++)
	{
		Log("creating random zoom thread #" + std::to_string(idx) + " to handle " + std::to_string(split_image_filenames[idx].size()) + " images...");
		vthreads.emplace_back(random_zoom_worker_lambda, idx);
	}

	while (work_done < work_to_do and error_detected.empty())
	{
		progress_window.setProgress(work_done / static_cast<double>(work_to_do));
		std::this_thread::sleep_for(std::chrono::milliseconds(750));
	}

	for (auto & t : vthreads)
	{
		t.join();
	}

	if (not error_detected.empty())
	{
		throw std::runtime_error(error_detected);
	}

	return;
}


void dm::DarknetWnd::drop_small_annotations(ThreadWithProgressWindow & progress_window, const VStr & all_output_images, size_t & number_of_annotations_dropped)
{
	std::atomic<size_t> work_done = 0;
	const size_t work_to_do = all_output_images.size();

	progress_window.setProgress(0.0);
	progress_window.setStatusMessage("Looking for annotations too small to detect...");

	const double image_width	= info.image_width;
	const double image_height	= info.image_height;

	std::atomic<size_t> total_annotations_dropped = 0;
	const auto & split_image_filenames = split(all_output_images);
	std::string error_detected;

	const auto drop_small_annotations_worker_lambda = [&, this](const size_t thread_idx)
	{
		std::string last_image_filename = "?";

		try
		{
			for (const auto & fn : split_image_filenames[thread_idx])
			{
				if (not error_detected.empty())
				{
					break;
				}

				work_done ++;

				last_image_filename = fn;

				const auto txt = std::filesystem::path(fn).replace_extension(".txt");
				if (std::filesystem::file_size(txt) == 0)
				{
					continue;
				}

				std::stringstream ss;
				ss << std::fixed << std::setprecision(10);

				std::ifstream ifs(txt.string());

				size_t lines_dropped_in_this_file	= 0;
				size_t lines_kept_in_this_file		= 0;
				int class_id						= -1;
				double cx							= -1.0;
				double cy							= -1.0;
				double w							= -1.0;
				double h							= -1.0;

				while (ifs.good())
				{
					ifs >> class_id >> cx >> cy >> w >> h;
					if (class_id >= 0 and cx > 0.0 and cy > 0.0 and w > 0.0 and h > 0.0)
					{
						const int annotation_width	= std::round(image_width * w);
						const int annotation_height	= std::round(image_height * h);
						const int area = annotation_width * annotation_height;

						if (area <= info.annotation_area_size)
						{
							// this annotation is too small, so don't bother remembering it (we'll re-write the .txt file without this line)
							Log(txt.string() + ": dropping annotation (too small): class #" + std::to_string(class_id) + " w=" + std::to_string(annotation_width) + " h=" + std::to_string(annotation_height) + " area=" + std::to_string(area) + " limit=" + std::to_string(info.annotation_area_size));
							total_annotations_dropped ++;
							lines_dropped_in_this_file ++;
							continue;
						}

						// otherwise, remember this annotation
						lines_kept_in_this_file ++;
						ss << class_id << " " << cx << " " << cy << " " << w << " " << h << std::endl;
					}
				}
				ifs.close();

				if (lines_dropped_in_this_file > 0)
				{
					// we must have decided to drop an annotation, so re-write the .txt file
					std::ofstream ofs(txt.string());
					ofs << ss.str();
				}
			}
		}
		catch (const std::exception & e)
		{
			std::string msg = "error in \"too-small\" thread #" + std::to_string(thread_idx) + " while processing \"" + last_image_filename + "\": " + e.what();
			Log(msg);
			if (error_detected.empty())
			{
				error_detected.swap(msg);
			}
		}
	};

	// start multiple threads running the "too-small" worker lambda, then we wait for all of them to be done

	VThreads vthreads;
	for (size_t idx = 0; idx < split_image_filenames.size(); idx ++)
	{
		Log("creating \"too-small\" thread #" + std::to_string(idx) + " to handle " + std::to_string(split_image_filenames[idx].size()) + " images...");
		vthreads.emplace_back(drop_small_annotations_worker_lambda, idx);
	}

	while (work_done < work_to_do and error_detected.empty())
	{
		progress_window.setProgress(work_done / static_cast<double>(work_to_do));
		std::this_thread::sleep_for(std::chrono::milliseconds(750));
	}

	for (auto & t : vthreads)
	{
		t.join();
	}

	number_of_annotations_dropped += total_annotations_dropped;

	if (not error_detected.empty())
	{
		throw std::runtime_error(error_detected);
	}

	return;
}
