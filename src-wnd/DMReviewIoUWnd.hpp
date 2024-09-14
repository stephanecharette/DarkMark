// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	struct ReviewIoUInfo
	{
		size_t number;

		cv::Mat thumbnail;

		std::string image_filename;

		double minimum_iou;
		double average_iou;
		double maximum_iou;

		size_t number_of_annotations;
		size_t number_of_predictions;
		size_t number_of_matches;

		size_t number_of_predictions_without_annotations;
		size_t number_of_annotations_without_predictions;
		size_t number_of_differences;

		std::string predictions_without_annotations;
		std::string annotations_without_predictions;

		ReviewIoUInfo()
		{
			number = 0;
			image_filename = "?";
			minimum_iou = 100.0;
			average_iou = -1.0;
			maximum_iou = -1.0;
			number_of_annotations = 0;
			number_of_predictions = 0;
			number_of_matches = 0;

			number_of_predictions_without_annotations = 0;
			number_of_annotations_without_predictions = 0;
			number_of_differences = 0;

			return;
		}
	};
	using VIoUInfo = std::vector<ReviewIoUInfo>;

	class DMReviewIoUWnd : public DocumentWindow, TableListBoxModel
	{
		public:

			DMReviewIoUWnd(DMContent & c);

			virtual ~DMReviewIoUWnd();

			virtual void closeButtonPressed() override;
			virtual void userTriedToCloseWindow() override;

			virtual int getNumRows() override;
			virtual void cellDoubleClicked(int rowNumber, int columnId, const MouseEvent & event) override;
			virtual String getCellTooltip(int rowNumber, int columnId) override;
			virtual void paintRowBackground(Graphics & g, int rowNumber, int width, int height, bool rowIsSelected) override;
			virtual void paintCell(Graphics & g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
			virtual void sortOrderChanged(int newSortColumnId, bool isForwards) override;

			DMContent & content;
			TableListBox tlb;
			VIoUInfo v;
	};
}
