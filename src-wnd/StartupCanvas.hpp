/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	struct DarknetFileInfo
	{
		enum class EType
		{
			kCfg,
			kWeight,
			kNames
		};

		EType type;
		std::string full_name;
		std::string short_name;
		std::string file_size;
		std::string timestamp;
	};

	typedef std::vector<DarknetFileInfo> VDarknetFileInfo;


	class StartupCanvas : public Component, public TableListBoxModel, public Value::Listener, public Button::Listener
	{
		public:

		/// Constructor.
		StartupCanvas(const std::string & key, const std::string & dir);

		/// Destructor.
		virtual ~StartupCanvas();

		/// Sets the @ref need_to_rebuild_cache_image flag, which eventually results in a call to @ref rebuild_cache_image().
		virtual void resized();

		virtual int getNumRows();
		virtual void paintRowBackground(Graphics &g, int rowNumber, int width, int height, bool rowIsSelected);
		virtual void paintCell(Graphics & g, int rowNumber, int columnId, int width, int height, bool rowIsSelected);
		virtual void selectedRowsChanged(int lastRowSelected);
		virtual void cellClicked(int rowNumber, int columnId, const MouseEvent & event);

		virtual void valueChanged(Value & value);

		virtual void buttonClicked(Button * button);

		void delete_extra_weights_files();

		void initialize_on_thread();

		void find_all_darknet_files();
		void filter_out_extra_weight_files();

		std::string cfg_key;

		PropertyPanel pp;
		Value project_directory;	///< full path where this project is located
		Value number_of_images;
		Value number_of_json;
		Value number_of_classes;
		Value number_of_marks;
		Value newest_markup;
		Value oldest_markup;
		Value darknet_configuration_filename;
		Value darknet_weights_filename;
		Value darknet_names_filename;

		TableListBox table;

		ImageComponent thumbnail;

		SStr extra_weights_files;
		ToggleButton hide_some_weight_files;

		std::atomic<bool> applying_filter;
		std::atomic<bool> done;
		std::thread t;

		VDarknetFileInfo v;
	};
}
