// DarkMark (C) 2019-2021 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class WndCfgTemplates : public DocumentWindow, public TableListBoxModel, public Thread, public Button::Listener
	{
		public:

			enum Fields
			{
				// columns in JUCE's TableListBox are 1-based, meaning the first column ("invalid") wont get used
				kInvalid		,
				kName			,
				kFullPath		,
				kLines			,
				kLayers			,
				kYoloLayers		,
				kNetworkSize	,
				kWeightsSize	,
				kRelated		,
				kLastModified	,
				kLastCommitName	,
				kNotes			,
				kLink1			,
				kUrl1			,
				kLink2			,
				kUrl2			,
				kNumberOfCols
			};

			struct Row
			{
				std::string field[Fields::kNumberOfCols];
				bool is_old;

				Row()
				{
					is_old = false;
				}
			};

			std::vector<Row> table_content_all;
			std::vector<Row> table_content_visible;

			WndCfgTemplates(Value & v);

			virtual ~WndCfgTemplates();

			virtual void userTriedToCloseWindow() override;

			virtual void resized() override;

			virtual void buttonClicked(Button * button) override;

			/// Inherited from Thread.  Used to find all of the .cfg files to use as templates.
			virtual void run() override;

			virtual int getNumRows() override;
			virtual void paintCell(Graphics & g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
			virtual void paintRowBackground(Graphics & g, int rowNumber, int width, int height, bool rowIsSelected) override;
			virtual void selectedRowsChanged(int lastRowSelected) override;
			virtual void cellClicked(int rowNumber, int columnId, const MouseEvent & event) override;

			/// Apply the "new" or "yolo" filters to the table contents
			void filter_rows();

			/// Store the full path+filename in this value if the user clicks on "ok".
			Value value;

			std::string selected_configuration_path_and_filename;

			Component canvas;
			Label filters;
			Label status;
			ToggleButton include_new_button;
			ToggleButton include_yolo_button;
			ToggleButton include_tiny_button;
			TableListBox table;
			TextButton help_button;
			TextButton ok_button;
			TextButton cancel_button;
	};
}
