// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class ClassIdWnd : public DocumentWindow, public Button::Listener, public ComboBox::Listener, public ThreadWithProgressWindow, public TableListBoxModel
	{
		public:

			struct Info
			{
				int			original_id;
				std::string	original_name;
				bool		must_delete;
				int			merge_into_id; // the ID we're merging into is either -1 (meaning we don't merge) or the original ID since that one never changes
				int			modified_id;
				std::string	modified_name;
			};

			ClassIdWnd(const std::string & fn);

			virtual ~ClassIdWnd();

			void add_row(const std::string & name);

			virtual void closeButtonPressed()			override;
			virtual void userTriedToCloseWindow()		override;
			virtual void resized()						override;
			virtual void buttonClicked(Button * button)	override;
			virtual void comboBoxChanged(ComboBox * comboBoxThatHasChanged) override;
			virtual void run()							override;

			virtual int getNumRows() override;
			virtual void paintRowBackground(Graphics &g, int rowNumber, int width, int height, bool rowIsSelected) override;
			virtual void paintCell(Graphics & g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
			virtual void selectedRowsChanged(int rowNumber) override;
			virtual void cellClicked(int rowNumber, int columnId, const MouseEvent & event) override;
			virtual Component * refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component * existingComponentToUpdate) override;

			void rebuild_table();

			const std::string names_fn;

			Component canvas;

			TableListBox table;

			TextButton add_button;
			TextButton apply_button;
			TextButton cancel_button;

			std::vector<ToggleButton *> toggle_buttons;
			std::vector<ComboBox *> combo_boxes;

			/// The checkboxes in LookAndFeel V4 are hard to see, so use V3 instead.
			LookAndFeel_V3 button_look_and_feel;

			std::vector<Info> vinfo;
	};
}
