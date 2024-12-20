// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class ClassIdWnd : public DocumentWindow, public Button::Listener, public ThreadWithProgressWindow, public TableListBoxModel
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
				EAction		action;
				std::string	merge_to_name; // merge to the class that has this name
				int			modified_id;
				std::string	modified_name;

				Info() :
					original_id(-1),
					action(EAction::kNone),
					modified_id(-1)
				{
					return;
				}
			};

			ClassIdWnd(File project_dir, const std::string & fn);

			virtual ~ClassIdWnd();

			void add_row(const std::string & name);

			virtual void closeButtonPressed()			override;
			virtual void userTriedToCloseWindow()		override;
			virtual void resized()						override;
			virtual void buttonClicked(Button * button)	override;
			virtual void run()							override;

			virtual int getNumRows() override;
			virtual void paintRowBackground(Graphics &g, int rowNumber, int width, int height, bool rowIsSelected) override;
			virtual void paintCell(Graphics & g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
			virtual void selectedRowsChanged(int rowNumber) override;
			virtual void cellClicked(int rowNumber, int columnId, const MouseEvent & event) override;
			virtual Component * refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component * existingComponentToUpdate) override;
			virtual String getCellTooltip(int rowNumber, int columnId) override;

			void rebuild_table();

			std::atomic<bool> done;
			std::thread counting_thread;
			void count_images_and_marks();

			/// Root of the project, where images and .txt files can be found
			File dir;

			/// The filename that contains all of the class names.
			const std::string names_fn;

			/// The key is the class ID, the val is the number of files found with that class.
			std::map<int, size_t> count_files_per_class;

			/// The key is the class ID, the val is the number of annotations found with that class.
			std::map<int, size_t> count_annotations_per_class;

			size_t error_count;

			Component canvas;

			TableListBox table;

			TextButton add_button;
			ArrowButton up_button;
			ArrowButton down_button;
			TextButton apply_button;
			TextButton cancel_button;

			std::vector<Info> vinfo;
	};
}
