// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"


#include "json.hpp"
using json = nlohmann::json;


dm::DMContentStatistics::DMContentStatistics(dm::DMContent & c) :
		ThreadWithProgressWindow("Gathering statistics...", true, true),
		content(c)
{
	return;
}


dm::DMContentStatistics::~DMContentStatistics()
{
	return;
}


void dm::DMContentStatistics::run()
{
	DarkMarkApplication::setup_signal_handling();

	const double max_work = content.image_filenames.size();
	double work_completed = 0.0;

	MStats m;

	// create a (blank) stats entry for every class we expect to find
	for (size_t idx = 0; idx < content.names.size(); idx ++)
	{
		m[idx].class_idx = idx;
		m[idx].name = content.names.at(idx);
	}

	for (const auto & fn : content.image_filenames)
	{
		if (threadShouldExit())
		{
			break;
		}

		setProgress(work_completed / max_work);
		work_completed ++;

		File f = File(fn).withFileExtension(".json");
		if (f.existsAsFile())
		{
			json root = json::parse(f.loadFileAsString().toStdString());

			// if the image is completely empty, then create a "fake" mark covering the entire image
			if (root.value("completely_empty", false))
			{
				root["mark"][0]["class_idx"] = content.empty_image_name_index;
				root["mark"][0]["rect"]["int_w"] = root["image"]["width"];
				root["mark"][0]["rect"]["int_h"] = root["image"]["height"];
			}

			std::map<size_t, size_t> mark_counter;

			for (auto mark : root["mark"])
			{
				const size_t class_idx = mark["class_idx"].get<size_t>();
				Stats & s = m[class_idx];
				s.count ++;
				s.filenames.insert(fn);

				const int w = mark["rect"]["int_w"].get<int>();
				const int h = mark["rect"]["int_h"].get<int>();
				const int a = w * h;

				s.sum_w += w;
				s.sum_h += h;
				s.sum_a += a;

				s.width_counts[w] ++;
				s.height_counts[h] ++;

				mark_counter[class_idx] ++;

				if (a < s.min_area)
				{
					s.min_area = a;
					s.min_size = cv::Size(w, h);
					s.min_filename = fn;
				}
				if (a > s.max_area)
				{
					s.max_area = a;
					s.max_size = cv::Size(w, h);
					s.max_filename = fn;
				}
			}

			// now go through the marks and see if we're beyond the minimum or maximum
			for (auto iter : mark_counter)
			{
				const size_t class_idx = iter.first;
				const size_t count = iter.second;

				Stats & s = m[class_idx];

				if (s.min_number_of_marks_per_image == 0 or s.min_number_of_marks_per_image > count)
				{
					// found new minimum
					s.min_number_of_marks_per_image = count;
					s.min_number_of_marks_filename = fn;

				}

				if (count > s.max_number_of_marks_per_image)
				{
					// found new maximum
					s.max_number_of_marks_per_image = count;
					s.max_number_of_marks_filename = fn;
				}
			}
		}
	}

	// remove the entry for "empty images" if it wasn't used
	if (m[content.empty_image_name_index].count == 0)
	{
		m.erase(content.empty_image_name_index);
	}

	// calculate the averages and standard deviations for each class
	for (auto iter : m)
	{
		if (threadShouldExit())
		{
			break;
		}

		const size_t idx = iter.first;
		Stats & s = m.at(idx);

		if (s.count > 0)
		{
			s.avg_w = double(s.sum_w) / double(s.count);
			s.avg_h = double(s.sum_h) / double(s.count);
			s.avg_a = double(s.sum_a) / double(s.count);

			double sum_of_diff_squared = 0.0;
			for (auto i : s.width_counts)
			{
				const double w				= i.first;
				const size_t count			= i.second;
				const double diff			= w - s.avg_w;
				const double diff_squared	= diff * diff;
				sum_of_diff_squared			+= (count * diff_squared);
			}
			s.standard_deviation_width = std::sqrt(sum_of_diff_squared / double(s.count));

			sum_of_diff_squared = 0.0;
			for (auto i : s.height_counts)
			{
				const double h				= i.first;
				const size_t count			= i.second;
				const double diff			= h - s.avg_h;
				const double diff_squared	= diff * diff;
				sum_of_diff_squared			+= (count * diff_squared);
			}
			s.standard_deviation_height = std::sqrt(sum_of_diff_squared / double(s.count));
		}
	}

	DMContent* contentPtr = &content;
	MessageManager::callAsync(
		[m = std::move(m), contentPtr]()
		mutable
		{
			auto & app = dmapp();

			if (!app.stats_wnd)
			{
				app.stats_wnd.reset(new DMStatsWnd(*contentPtr));
			}

			app.stats_wnd->m.swap(m);
			app.stats_wnd->tlb.updateContent();
			app.stats_wnd->tlb.repaint();
			app.stats_wnd->toFront(true);
        }
	);

	return;
}
