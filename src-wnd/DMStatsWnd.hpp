// DarkMark (C) 2019-2023 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	struct Stats
	{
		size_t class_idx;
		std::string name;

		/// The total number of times this class shows up across all images.
		size_t count;

		/// The set of all filenames where this class shows up.
		dm::SStr filenames;

		/// The smallest area, in pixels.  @see @ref min_size
		int min_area;

		/// The size that corresponds to the smallest area.  @see @ref min_area
		cv::Size min_size;

		/// The largest area, in pixels.  @see @ref max_size
		int max_area;

		/// The size that corresponds to the largest area.  @see @ref max_area
		cv::Size max_size;

		/// The sum of widths, heights, and area which is then used to calculate the averages.
		int sum_w;
		int sum_h;
		int sum_a;

		/// The averages of widths, heights and area as calculated from @ref sum_w, @ref sum_h, and @ref sum_a.
		double avg_w;
		double avg_h;
		double avg_a;

		double standard_deviation_width;
		double standard_deviation_height;

		size_t min_number_of_marks_per_image;
		size_t max_number_of_marks_per_image;

		/// Keep track of all widths and all heights so we can calculate standard deviations.  @{
		std::map<int, size_t> width_counts;
		std::map<int, size_t> height_counts;
		/// @}

		std::string min_filename;
		std::string max_filename;

		std::string min_number_of_marks_filename;
		std::string max_number_of_marks_filename;

		Stats()
		{
			name						= "?";
			class_idx					= 0;
			count						= 0;
			standard_deviation_width	= 0.0;
			standard_deviation_height	= 0.0;

			min_area = INT_MAX;
			max_area = INT_MIN;

			sum_w = 0;
			sum_h = 0;
			sum_a = 0;

			avg_w = 0.0;
			avg_h = 0.0;
			avg_a = 0.0;

			min_number_of_marks_per_image = 0.0;
			max_number_of_marks_per_image = 0.0;
		}
	};

	/// Map where the key is the class id and the value is the full stats for that key.
	typedef std::map<size_t, Stats> MStats;

	class DMStatsWnd : public DocumentWindow, TableListBoxModel
	{
		public:

			DMStatsWnd(DMContent & c);

			virtual ~DMStatsWnd();

			virtual void closeButtonPressed();
			virtual void userTriedToCloseWindow();

			virtual int getNumRows();
			virtual void cellDoubleClicked(int rowNumber, int columnId, const MouseEvent & event);
			virtual String getCellTooltip(int rowNumber, int columnId);
			virtual void paintRowBackground(Graphics & g, int rowNumber, int width, int height, bool rowIsSelected);
			virtual void paintCell(Graphics & g, int rowNumber, int columnId, int width, int height, bool rowIsSelected);

			DMContent & content;
			TableListBox tlb;
			MStats m;
	};
}
