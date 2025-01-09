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

			/// Used to stop the counting thread in cases where the window is closed early.
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

			/** Set by the counting thread if any errors were found while looking through all of the annotations.  If there are
			 * errors, then prevent the user from saving modifications to the classes.  User must fix errors first, to prevent
			 * us making things worse by attempting to modify bad .txt annotation files.
			 */
			size_t error_count;

			Component canvas;

			TableListBox table;

			TextButton add_button;
			ArrowButton up_button;
			ArrowButton down_button;
			TextButton apply_button;
			TextButton cancel_button;

			/// This is the content of the "table".
			std::vector<Info> vinfo;

			/// Flag will be set to @p true once the counting thread has finished looking at all the image annotations.
			std::atomic<bool> done_looking_for_images;

			/** All images which were found to have YOLO annotations.  This is set by the counting thread, and will only be
			 * populated once the @ref done_looking_for_images flag has also been set.
			 */
			VStr all_images;

			bool names_file_rewritten;
			size_t number_of_annotations_deleted;
			size_t number_of_annotations_remapped;
			size_t number_of_txt_files_rewritten;
	};
}
