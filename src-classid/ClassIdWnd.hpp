// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class ClassIdWnd : public DocumentWindow, public Button::Listener, public ComboBox::Listener, public ThreadWithProgressWindow, public TableListBoxModel
	{
		public:

			enum class EAction
			{
				kInvalid,
				kNone,
				kMerge,
				kDelete,
			};

			struct Info
			{
				int			original_id;
				std::string	original_name;
				size_t		images;
				size_t		count;
				EAction		action;
				int			modified_id;
				std::string	modified_name;

				Info() :
					original_id(-1),
					images(0),
					count(0),
					action(EAction::kNone),
					modified_id(-1)
				{
					return;
				}
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
			ArrowButton up_button;
			ArrowButton down_button;
			TextButton apply_button;
			TextButton cancel_button;

//			std::vector<ToggleButton *> toggle_buttons;
			std::vector<ComboBox *> combo_boxes;

			std::vector<Info> vinfo;
	};
}
