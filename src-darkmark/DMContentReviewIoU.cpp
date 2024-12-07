// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"

#include "json.hpp"
using json = nlohmann::json;


double IoU(const cv::Rect r1, const cv::Rect r2)
{
	// see: https://stackoverflow.com/questions/9324339/how-much-do-two-rectangles-overlap/9325084

	const auto tl1 = r1.tl();	// blue_triangle
	const auto tl2 = r2.tl();	// orange_triangle
	const auto br1 = r1.br();	// blue_circle
	const auto br2 = r2.br();	// orange_circle

	const double intersecting_area = std::max(0, std::min(br2.x, br1.x) - std::max(tl2.x, tl1.x)) * std::max(0, std::min(br2.y, br1.y) - std::max(tl2.y, tl1.y));
	const double a1 = r1.area();
	const double a2 = r2.area();
	const double intersection_over_union = intersecting_area / std::max(1.0, (a1 + std::min(a2, intersecting_area) - intersecting_area));

	return intersection_over_union;
}


dm::DMContentReviewIoU::DMContentReviewIoU(dm::DMContent & c) :
	ThreadWithProgressWindow("Predicting With Darknet/YOLO...", true, true),
	content(c)
{
	return;
}


dm::DMContentReviewIoU::~DMContentReviewIoU()
{
	return;
}


void dm::DMContentReviewIoU::run()
{
	DarkMarkApplication::setup_signal_handling();

	const float max_work = content.image_filenames.size();
	float work_completed = 0.0f;

	cv::Size thumbnail_size(30, 30);
	const int row_height = cfg().get_int("review_table_row_height");
	if (row_height >= 10)
	{
		thumbnail_size.width = row_height;
		thumbnail_size.height = row_height;
	}

	VIoUInfo v;
	v.reserve(content.image_filenames.size());

	for (const auto & fn : content.image_filenames)
	{
		if (threadShouldExit())
		{
			break;
		}

		setProgress(work_completed / max_work);
		work_completed ++;

		File f = File(fn).withFileExtension(".json");
		if (f.existsAsFile() == false)
		{
			// nothing we can do with this file since we don't have a corresponding .json
			continue;
		}

		json root;
		cv::Mat mat;
		try
		{
			Log("IoU: loading " + fn);
			root = json::parse(f.loadFileAsString().toStdString());
			mat = cv::imread(fn);
		}
		catch(const std::exception & e)
		{
			Log("failed to read image " + fn + " or parse json " + f.getFullPathName().toStdString() + ": " + e.what());
			continue;
		}

		if (mat.empty())
		{
			Log("failed to load image " + fn);
			continue;
		}

		ReviewIoUInfo info;
		info.image_filename = fn;
		info.number_of_annotations = root["mark"].size();

		const float size_factor = static_cast<float>(mat.rows) / mat.cols;
		const cv::Size desired_size(std::round(size_factor * row_height), row_height);
		info.thumbnail = DarkHelp::fast_resize_ignore_aspect_ratio(mat, desired_size);

		const auto results = dmapp().darkhelp_nn->predict(mat);
		info.number_of_predictions = results.size();

		SId classes_annotations_without_predictions;
		SId classes_predictions_without_annotations;
		SId prediction_index_consumed;
		double total_iou = 0.0;

		// markup annotations are considered "official" against which we'll compare the predictions
		for (const auto & mark : root["mark"])
		{
			if (threadShouldExit())
			{
				break;
			}

			const int class_idx = mark["class_idx"];
			const int x = mark["rect"]["int_x"].get<int>();
			const int y = mark["rect"]["int_y"].get<int>();
			const int w = mark["rect"]["int_w"].get<int>();
			const int h = mark["rect"]["int_h"].get<int>();
			const cv::Rect mark_r(x, y, w, h);

			// key is the IoU, val is the prediction index
			std::multimap<double, size_t> mm;

			// look through the predictions and see if we can find a match
			for (size_t idx = 0; idx < results.size(); idx ++)
			{
				if (threadShouldExit())
				{
					break;
				}

				if (prediction_index_consumed.count(idx))
				{
					// this prediction was already consumed by a previous annotation
					continue;
				}

				const auto & pred = results.at(idx);

				if (pred.all_probabilities.count(class_idx) == 0)
				{
					// this prediction has 0% chance to match the class_idx so look for something else
					continue;
				}

				// if we get here then the class index matches, so now compare the IoU
				const double iou = IoU(mark_r, pred.rect);
				if (iou > 0.0)
				{
					mm.insert({iou, idx});
				}
			}

			if (mm.empty())
			{
				// zero Darknet/YOLO predictions were found to match this annotation
				info.minimum_iou = 0.0;
				classes_annotations_without_predictions.insert(class_idx);
				info.number_of_annotations_without_predictions ++;
			}
			else
			{
				// take the best IoU and remove that index from the available results
				auto iter = mm.rbegin();
				const double iou = iter->first;
				const size_t idx = iter->second;

				prediction_index_consumed.insert(idx);
				total_iou += iou;

				info.number_of_matches ++;

				if (iou < info.minimum_iou)
				{
					info.minimum_iou = iou;
				}

				if (iou > info.maximum_iou)
				{
					info.maximum_iou = iou;
				}
			}
		}

		// once we get here we're done looking at all the markup and predictions for this image

		if (info.number_of_annotations == 0 and info.number_of_predictions == 0)
		{
			// this is an empty image (negative sample)
			info.minimum_iou = 1.0;
			info.maximum_iou = 1.0;
			info.average_iou = 1.0;
		}
		else
		{
			info.average_iou = total_iou / std::max(info.number_of_annotations, info.number_of_predictions);
		}

		if (info.minimum_iou < 0.0 or info.minimum_iou > 1.0)
		{
			info.minimum_iou = 0.0;
		}
		if (info.maximum_iou < 0.0 or info.maximum_iou > 1.0)
		{
			info.maximum_iou = 0.0;
		}
		if (info.average_iou < 0.0 or info.average_iou > 1.0)
		{
			info.average_iou = 0.0;
		}

		for (size_t idx = 0; idx < results.size(); idx ++)
		{
			if (prediction_index_consumed.count(idx) == 0)
			{
				const auto best_class = results.at(idx).best_class;
				classes_predictions_without_annotations.insert(best_class);
				info.number_of_predictions_without_annotations ++;
			}
		}

		for (const size_t idx : classes_predictions_without_annotations)
		{
			if (not info.predictions_without_annotations.empty())
			{
				info.predictions_without_annotations += ", ";
			}
			info.predictions_without_annotations += content.names.at(idx);
		}

		for (const size_t idx : classes_annotations_without_predictions)
		{
			if (not info.annotations_without_predictions.empty())
			{
				info.annotations_without_predictions += ", ";
			}
			info.annotations_without_predictions += content.names.at(idx);
		}

		info.number_of_differences = info.number_of_predictions_without_annotations + info.number_of_annotations_without_predictions;

		info.number = v.size() + 1;

		v.push_back(info);

		// update the JSON with the IoU information for this image; these values are then used when sorting
		Log("IoU: updating " + f.getFullPathName().toStdString());
		root["predictions"]["IoU"]["min"]						= info.minimum_iou;
		root["predictions"]["IoU"]["avg"]						= info.average_iou;
		root["predictions"]["IoU"]["max"]						= info.maximum_iou;
		root["predictions"]["count"]							= info.number_of_predictions;
		root["predictions"]["matches"]							= info.number_of_matches;
		root["predictions"]["predictions_without_annotations"]	= info.number_of_predictions_without_annotations;
		root["predictions"]["annotations_without_predictions"]	= info.number_of_annotations_without_predictions;
		root["predictions"]["number_of_differences"]			= info.number_of_differences;

		std::ofstream fs(f.getFullPathName().toStdString());
		fs << root.dump(1, '\t') << std::endl;
	}

	if (not dmapp().review_iou_wnd)
	{
		dmapp().review_iou_wnd.reset(new DMReviewIoUWnd(content));
	}
	dmapp().review_iou_wnd->v.swap(v);
	dmapp().review_iou_wnd->toFront(true);
	dmapp().review_iou_wnd->tlb.updateContent();

	content.IoU_info_found = true;

	return;
}
