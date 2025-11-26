// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"
#include <magic.h>


dm::StartupWnd::StartupWnd() :
		DocumentWindow("DarkMark v" DARKMARK_VERSION " Launcher", Colours::darkgrey, TitleBarButtons::allButtons),
		add_button("Add..."),
		delete_button("Delete..."),
		import_video_button("Import video..."),
		import_pdf_button("Import PDF..."),
		open_folder_button("Open folder..."),
		class_id_button("Dataset..."),
		refresh_button("Refresh"),
		ok_button("Load..."),
		cancel_button("Cancel")
{
	setContentNonOwned		(&canvas, true	);
	setUsingNativeTitleBar	(true			);
	setResizable			(true, false	);
	setDropShadowEnabled	(true			);

	canvas.addAndMakeVisible(notebook);
	canvas.addAndMakeVisible(add_button);
	canvas.addAndMakeVisible(delete_button);
	canvas.addAndMakeVisible(import_video_button);
	canvas.addAndMakeVisible(import_pdf_button);
	canvas.addAndMakeVisible(open_folder_button);
	canvas.addAndMakeVisible(class_id_button);
	canvas.addAndMakeVisible(refresh_button);
	canvas.addAndMakeVisible(ok_button);
	canvas.addAndMakeVisible(cancel_button);

	std::multimap<size_t, std::string> m;

	const std::regex rgx("^project_(.+)_dir$");

	const auto all_cfg = cfg().getAllProperties();
	const auto all_keys = all_cfg.getAllKeys();
	for (const String & k : all_keys)
	{
		std::smatch matches;
		const std::string str = k.toStdString();
		const bool found = std::regex_match(str, matches, rgx);
		if (found)
		{
			// now that we know the name, get a timestamp which we'll use to order the projects by most-recently-used
			const std::string key = matches[1].str();

			const size_t timestamp = cfg().getIntValue("project_" + key + "_timestamp");
			Log("project key " + key + ", timestamp=" + std::to_string(timestamp));
			m.insert({timestamp, key});
		}
	}

	for (auto iter : m)
	{
		const std::string & key	= iter.second;
		const std::string dir	= cfg().getValue("project_" + key + "_dir", "error").toStdString();
		const std::string name	= cfg().getValue("project_" + key + "_name", File(dir).getFileNameWithoutExtension()).toStdString();
		notebook.addTab(name, Colours::darkgrey, new StartupCanvas(key, dir), true, 0);
	}

	add_button			.addListener(this);
	delete_button		.addListener(this);
	import_video_button	.addListener(this);
	import_pdf_button	.addListener(this);
	open_folder_button	.addListener(this);
	class_id_button		.addListener(this);
	refresh_button		.addListener(this);
	ok_button			.addListener(this);
	cancel_button		.addListener(this);

	const auto & options = dmapp().cli_options;

	if (options.count("add"))
	{
		removeFromDesktop();
		setVisible(false);
		add_button.triggerClick();
	}

	if (options.count("del"))
	{
		removeFromDesktop();
		setVisible(false);
	}

	bool found = false;
	if (options.count("project_key"))
	{
		removeFromDesktop();
		setVisible(false);
		for (int idx = 0; idx < notebook.getNumTabs(); idx ++)
		{
			StartupCanvas * startup_canvas = dynamic_cast<StartupCanvas*>(notebook.getTabContentComponent(idx));
			if (startup_canvas)
			{
				const auto & key = startup_canvas->cfg_key;
				if (key == options.at("project_key"))
				{
					notebook.setCurrentTabIndex(idx);
					ok_button.triggerClick();
					found = true;
					break;
				}
			}
		}
	}

	if (not found)
	{
		notebook.setCurrentTabIndex(0);
		updateButtons();

		setIcon(DarkMarkLogo());
		ComponentPeer *peer = getPeer();
		if (peer)
		{
			peer->setIcon(DarkMarkLogo());
		}

		if (cfg().containsKey("StartupWnd"))
		{
#if 0
			// Why does this not work consistently?
			// Window shrinks since JUCE 6, and sometimes gets resized to [0, 0] since JUCE 7.
			restoreWindowStateFromString(cfg().getValue("StartupWnd"));
#else
			auto r = getParentMonitorArea();
			r.reduce(50, 50);
			setBounds(r);

			// JUCE v7 still manages to get the size wrong every once in a while,
			// so set a limit to prevent the window from appearing as a tiny square
			// just a few pixels in size at the top-left corner of the screen.
			setResizeLimits(r.getWidth() / 2, r.getHeight() / 2, 999999, 999999);
#endif
		}
		else
		{
			centreWithSize(800, 600);
		}

#if JUCE_MAC
		// fullscreen=true causes an issue when AlertWindow appears underneath the Launcher window 
		// and the entire UI becomes blocked on mac
		setFullScreen(false);
#else
		setFullScreen(true);
#endif

		setVisible(true);
	}

	return;
}


dm::StartupWnd::~StartupWnd()
{
	stop_refreshing_all_notebook_tabs();

	if (dmapp().cli_options.count("project_key") == 0)
	{
		cfg().setValue("StartupWnd", getWindowStateAsString());
	}

	return;
}


void dm::StartupWnd::closeButtonPressed()
{
	// close button

	setVisible(false);
	dmapp().startup_wnd.reset(nullptr);
	dmapp().quit();

	return;
}


void dm::StartupWnd::userTriedToCloseWindow()
{
	// ALT+F4

	closeButtonPressed();

	return;
}


void dm::StartupWnd::resized()
{
	const auto & options = dmapp().cli_options;

	if (options.count("add"			) == 0 and
		options.count("del"			) == 0 and
		options.count("project_key"	) == 0 )
	{
		// get the document window to resize the canvas, then we'll deal with the rest of the components
		DocumentWindow::resized();

		const int margin_size = 5;

		FlexBox button_row;
		button_row.flexDirection = FlexBox::Direction::row;
		button_row.justifyContent = FlexBox::JustifyContent::flexEnd;
		button_row.items.add(FlexItem(add_button)			.withWidth(100.0).withMargin(FlexItem::Margin(0, margin_size, 0, 0)));
		button_row.items.add(FlexItem(delete_button)		.withWidth(100.0).withMargin(FlexItem::Margin(0, margin_size, 0, 0)));
		button_row.items.add(FlexItem()						.withFlex(1.0));
		button_row.items.add(FlexItem(import_video_button)	.withWidth(125.0).withMargin(FlexItem::Margin(0, margin_size, 0, 0)));
		button_row.items.add(FlexItem(import_pdf_button)	.withWidth(125.0).withMargin(FlexItem::Margin(0, margin_size, 0, 0)));
		button_row.items.add(FlexItem(open_folder_button)	.withWidth(125.0).withMargin(FlexItem::Margin(0, margin_size, 0, 0)));
		button_row.items.add(FlexItem(class_id_button)		.withWidth(125.0).withMargin(FlexItem::Margin(0, margin_size, 0, 0)));
		button_row.items.add(FlexItem()						.withFlex(1.0));
		button_row.items.add(FlexItem(refresh_button)		.withWidth(100.0).withMargin(FlexItem::Margin(0, margin_size, 0, 0)));
		button_row.items.add(FlexItem(ok_button)			.withWidth(100.0).withMargin(FlexItem::Margin(0, margin_size, 0, 0)));
		button_row.items.add(FlexItem(cancel_button)		.withWidth(100.0));

		FlexBox fb;
		fb.flexDirection = FlexBox::Direction::column;
		fb.items.add(FlexItem(notebook).withFlex(1.0));
		fb.items.add(FlexItem(button_row).withHeight(30.0).withMargin(FlexItem::Margin(margin_size, 0, 0, 0)));

		auto r = getLocalBounds();
		r.reduce(margin_size, margin_size);
		fb.performLayout(r);
	}

	return;
}


void dm::StartupWnd::buttonClicked(Button * button)
{
	if (button == &cancel_button)
	{
		stop_refreshing_all_notebook_tabs();
		closeButtonPressed();
	}
	else if (button == &open_folder_button)
	{
		StartupCanvas * notebook_canvas = dynamic_cast<StartupCanvas*>(notebook.getTabContentComponent(notebook.getCurrentTabIndex()));
		if (notebook_canvas)
		{
			File dir(notebook_canvas->project_directory.toString());
			dir.revealToUser();
		}
	}
	else if (button == &class_id_button)
	{
		StartupCanvas * notebook_canvas = dynamic_cast<StartupCanvas*>(notebook.getTabContentComponent(notebook.getCurrentTabIndex()));
		if (notebook_canvas)
		{
			const auto fn = notebook_canvas->darknet_names_filename.toString();
			const auto dir = notebook_canvas->project_directory.toString();
			if (dir.isEmpty() == false and fn.isEmpty() == false)
			{
				// modal windows are causing problems for some people, so convert this to an async window and
				// try and behave somewhat like a modal by disabling all the controls on the parent window

				setEnabled(false); // these will be re-enabled when the ClassIdWnd is destroyed

				dmapp().class_id_wnd.reset(new ClassIdWnd(dir, fn.toStdString()));
				dmapp().class_id_wnd->setVisible(true);
				dmapp().class_id_wnd->toFront(true);
			}
		}
	}
	else if (button == &refresh_button)
	{
		StartupCanvas * notebook_canvas = dynamic_cast<StartupCanvas*>(notebook.getTabContentComponent(notebook.getCurrentTabIndex()));
		if (notebook_canvas)
		{
			notebook_canvas->refresh();
		}
	}
	else if (button == &import_pdf_button)
	{
		StartupCanvas * notebook_canvas = dynamic_cast<StartupCanvas*>(notebook.getTabContentComponent(notebook.getCurrentTabIndex()));
		if (notebook_canvas)
		{
			setEnabled(false);
			const auto base_directory = notebook_canvas->project_directory.toString();
			FileChooser fc("Select one or more PDF document to import...", base_directory, "*.pdf");
			if (fc.browseForMultipleFilesToOpen())
			{
				bool ok_to_proceed = true;

				// check to make sure all the files selected are video files
				magic_t magic_cookie = magic_open(MAGIC_MIME_TYPE);
				magic_load(magic_cookie, nullptr);

				VStr v;
				for (auto && file : fc.getResults())
				{
					const std::string filename = file.getFullPathName().toStdString();
					const std::string filetype = magic_file(magic_cookie, filename.c_str());
					Log("pdf import mime type: " + filetype + " " + filename);
					if (filetype == "application/pdf")
					{
						v.push_back(filename);
					}
					else
					{
						AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon,
							"DarkMark",
							"The selected file \"" + file.getFileName() + "\" does not appear to be a valid PDF file.\n"
							"\n"
							"Expected the MIME type to be application/pdf, but the type appears to be \"" + filetype + "\".\n"
							"\n" + filename);
						ok_to_proceed = false;
						break;
					}
				}
				magic_close(magic_cookie);

				if (ok_to_proceed)
				{
					setEnabled(false);
					PdfImportWindow piw(base_directory.toStdString(), v);
					piw.runModalLoop();
					setEnabled(true);
					if (piw.number_of_imported_pages > 0)
					{
						// re-scan this project now that we have more image files
						notebook_canvas->refresh();
					}
				}
			}
		}
		setEnabled(true);
	}
	else if (button == &import_video_button)
	{
		StartupCanvas * notebook_canvas = dynamic_cast<StartupCanvas*>(notebook.getTabContentComponent(notebook.getCurrentTabIndex()));
		if (notebook_canvas)
		{
			setEnabled(false);
			const auto base_directory = notebook_canvas->project_directory.toString();
			FileChooser fc("Select one or more video to import...", base_directory, "*");
			if (fc.browseForMultipleFilesToOpen())
			{
				bool ok_to_proceed = true;

				// check to make sure all the files selected are video files
				magic_t magic_cookie = magic_open(MAGIC_MIME_TYPE);
				magic_load(magic_cookie, nullptr);

				VStr v;
				for (auto && file : fc.getResults())
				{
					const std::string filename = file.getFullPathName().toStdString();
					const std::string filetype = magic_file(magic_cookie, filename.c_str());
					Log("video import mime type: " + filetype + " " + filename);
					if (filetype.find("video/") != std::string::npos)
					{
						v.push_back(filename);
					}
					else
					{
						AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon,
							"DarkMark",
							"The selected file \"" + file.getFileName() + "\" does not appear to be a valid video file.\n"
							"\n"
							"Expected the MIME type to be video/*, but the type appears to be \"" + filetype + "\".\n"
							"\n" + filename);
						ok_to_proceed = false;
						break;
					}
				}
				magic_close(magic_cookie);

				if (ok_to_proceed)
				{
					setEnabled(false);
					VideoImportWindow viw(base_directory.toStdString(), v);
					viw.runModalLoop();
					setEnabled(true);
					if (viw.number_of_processed_frames > 0)
					{
						// re-scan this project now that we have more image files
						notebook_canvas->refresh();
					}
				}
			}
		}
		setEnabled(true);
	}
	else if (button == &add_button)
	{
		bool result = true;
		File dir;

		auto & options = dmapp().cli_options;
		if (options.count("add"))
		{
			dir = File(options["add"]);
		}
		else
		{
			setEnabled(false);
			dir = File::getSpecialLocation(File::SpecialLocationType::userHomeDirectory);

			// attempt to default to the parent directory of the active tab (if we have a tab)
			StartupCanvas * notebook_canvas = dynamic_cast<StartupCanvas*>(notebook.getTabContentComponent(notebook.getCurrentTabIndex()));
			if (notebook_canvas)
			{
				dir = File(notebook_canvas->project_directory.toString()).getParentDirectory();
			}

			FileChooser chooser("Select the new project directory...", dir);
			result = chooser.browseForDirectory();
			if (result)
			{
				dir = chooser.getResult();
			}
		}

		if (result)
		{
			// loop through the existing tabs and make sure this directory doesn't already show up
			bool ok = true;
			for (int idx = 0; idx < notebook.getNumTabs(); idx ++)
			{
				StartupCanvas * notebook_canvas = dynamic_cast<StartupCanvas*>(notebook.getTabContentComponent(idx));
				if (notebook_canvas)
				{
					File path(notebook_canvas->project_directory.toString());
					if (dir == path or dir.isAChildOf(path) or path.isAChildOf(dir))
					{
						notebook.setCurrentTabIndex(idx);
						AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "DarkMark Project", "The new project path " + dir.getFullPathName().toStdString() + " conflicts with an existing project located at " + path.getFullPathName().toStdString() + ".", "Cancel");
						ok = false;
						break;
					}
				}
			}

			if (ok)
			{
				if (dir == File::getSpecialLocation(File::SpecialLocationType::userHomeDirectory)				or
					dir == File::getSpecialLocation(File::SpecialLocationType::userDesktopDirectory)			or
					dir == File::getSpecialLocation(File::SpecialLocationType::userDocumentsDirectory)			or
					dir == File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory)	or
					dir == File::getSpecialLocation(File::SpecialLocationType::userPicturesDirectory)			)
				{
					AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "DarkMark Project", "The new project path " + dir.getFullPathName().toStdString() + " is a reserved path and shouldn't be used as a project directory.  Instead, you should create a new subdirectory for your project within this location.\n\nFor examples, please see \"https://www.ccoderun.ca/programming/darknet_faq/#directory_setup\".", "Cancel");
					ok = false;
				}
			}

			if (ok)
			{
				const std::string project_name = dir.getFileNameWithoutExtension().toStdString();
				std::string names = dir.getChildFile(project_name).withFileExtension(".names").getFullPathName().toStdString();

				if (File(names).existsAsFile() == false)
				{
					// see if we can find a .names file we can use
					auto type = File::TypesOfFileToFind::findFiles + File::TypesOfFileToFind::ignoreHiddenFiles;
					auto files = dir.findChildFiles(type, false, "*.names");
					if (not files.isEmpty())
					{
						names = files[0].getFullPathName().toStdString();
					}
				}

				const bool need_to_create_new_names_file = (File(names).existsAsFile() == false);
				if (need_to_create_new_names_file)
				{
					std::ofstream ofs(names);
					ofs	<< "bicycle"	<< std::endl
						<< "car"		<< std::endl
						<< "truck"		<< std::endl
						<< "person"		<< std::endl;
				}

				// find a key that isn't used yet
				String key;
				while (true)
				{
					key = String(std::rand());
					if (cfg().containsKey("project_" + key + "_dir") == false)
					{
						break;
					}
				}

				cfg().setValue("project_" + key + "_dir"		, dir.getFullPathName());
				cfg().setValue("project_" + key + "_timestamp"	, (int)std::time(nullptr));
				cfg().setValue("project_" + key + "_names"		, names.c_str());

				notebook.addTab(project_name, Colours::darkgrey, new StartupCanvas(key.toStdString(), dir.getFullPathName().toStdString()), true, 0);
				notebook.setCurrentTabIndex(0);

				if (options.count("add"))
				{
					options["load"]			= project_name;
					options["project_key"]	= key.toStdString();
					ok_button.triggerClick();
				}
				else
				{
					AlertWindow::showMessageBox(AlertWindow::AlertIconType::InfoIcon, "DarkMark Project", "Successfully added the new project " + dir.getFullPathName() + ".");
					if (need_to_create_new_names_file)
					{
						File(names).revealToUser();
					}
				}
			}
		}

		updateButtons();
		setEnabled(true);
	}
	else if (button == &delete_button)
	{
		const int tab_index = notebook.getCurrentTabIndex();
		StartupCanvas * notebook_canvas = dynamic_cast<StartupCanvas*>(notebook.getTabContentComponent(tab_index));
		if (notebook_canvas)
		{
			setEnabled(false);
			notebook_canvas->done = true;
			File dir(notebook_canvas->project_directory.toString());

			int result = AlertWindow::showYesNoCancelBox(AlertWindow::AlertIconType::QuestionIcon, "DarkMark Project Deletion",
					"Project directory: " + dir.getFullPathName().toStdString() + "\n"
					"\n"
					"Do you want to remove the project from DarkMark but keep the files, or remove the project and also delete all the files?",
					"Remove Project Only", "Remove Project And Delete Files", "Cancel");

			if (result > 0)
			{
				// need to remove the project from DarkMark, which means we must go through all of the keys
				// and delete all the ones that match the name "project_0123_..."

				const String name = "project_" + notebook_canvas->cfg_key + "_";
				notebook.removeTab(tab_index);
				notebook_canvas = nullptr;

				SStr keys_to_delete;
				for (const String & k : cfg().getAllProperties().getAllKeys())
				{
					if (k.startsWith(name))
					{
						keys_to_delete.insert(k.toStdString());
					}
				}
				for (const std::string & k : keys_to_delete)
				{
					Log(dir.getFullPathName().toStdString() + ": removing key from configuration: " + k);
					cfg().removeValue(k);
				}
				notebook.setCurrentTabIndex(std::max(0, tab_index - 1));
			}

			if (result == 2)
			{
				// delete all of the source files!?
				result = AlertWindow::showOkCancelBox(
						AlertWindow::AlertIconType::QuestionIcon,
						"DarkMark Project Deletion",
						"Project directory: " + dir.getFullPathName().toStdString() + "\n"
						"\n"
						"The project has been removed from DarkMark.\n"
						"\n"
						"Please confirm you also want to delete all the images, configuration, weights, etc., contained in this directory.",
						"Delete All Files", "Cancel");

				if (result == 1)
				{
					Log("user has chosen to delete all of the files in " + dir.getFullPathName().toStdString());
					dir.deleteRecursively();
				}
			}
		}
		setEnabled(true);
		updateButtons();
	}
	else if (button == &ok_button)
	{
		stop_refreshing_all_notebook_tabs();
		StartupCanvas * notebook_canvas = dynamic_cast<StartupCanvas*>(notebook.getTabContentComponent(notebook.getCurrentTabIndex()));
		if (notebook_canvas)
		{
			canvas.setEnabled(false);

			bool load_project				= true;
			const String key				= notebook_canvas->cfg_key;
			const String cfg_template		= notebook_canvas->darknet_configuration_template	.toString();
			const String cfg_filename		= notebook_canvas->darknet_configuration_filename	.toString();
			const String weights_filename	= notebook_canvas->darknet_weights_filename			.toString();
			const String names_filename		= notebook_canvas->darknet_names_filename			.toString();
			const int image_count			= notebook_canvas->number_of_images					.getValue();

			const String inclusion_regex	= notebook_canvas->inclusion_regex					.toString();
			const String exclusion_regex	= notebook_canvas->exclusion_regex					.toString();
			const bool both_regex_set		= inclusion_regex.isNotEmpty() and exclusion_regex.isNotEmpty();

			const bool dir_exists			= File(notebook_canvas->project_directory.toString()).exists();
			const bool cfg_exists			= cfg_filename		.isEmpty()	or File(cfg_filename		).existsAsFile();
			const bool weights_exits		= weights_filename	.isEmpty()	or File(weights_filename	).existsAsFile();
			const bool names_exists			= names_filename	.isEmpty()	or File(names_filename		).existsAsFile();

			size_t warning_count	= 0;
			size_t error_count		= 0;

			if (dmapp().cli_options.count("project_key") == 0)
			{
				std::stringstream ss;
				ss << std::endl << std::endl;
				if (image_count			< 1)		{ ss << "- There are no images in the project directory."	<< std::endl;	error_count		++; }
				if (dir_exists			== false)	{ ss << "- The project directory does not exist."			<< std::endl;	error_count		++; }
				if (cfg_exists			== false)	{ ss << "- The .cfg file does not exist."					<< std::endl;	warning_count	++; }
				if (weights_exits		== false)	{ ss << "- The .weights file does not exist."				<< std::endl;	warning_count	++; }
				if (names_exists		== false)	{ ss << "- The .names file does not exist."					<< std::endl;	error_count		++; }
				if (both_regex_set		== true)	{ ss << "- Cannot set both inclusion and exclusion regex."	<< std::endl;	error_count		++; }

				const size_t message_count = warning_count + error_count;
				if (error_count > 0)
				{
					const String title = "DarkMark Project Error" + String(message_count == 1 ? "" : "s") + " Detected";
					const String msg = "Please note the following error message" + String(message_count == 1 ? "" : "s") + ":" + ss.str();
					AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, title, msg);
					load_project = false;
				}
				else if (warning_count > 0)
				{
					ss << std::endl << "Continue to load this project?";
					const String title = "DarkMark Project Warning" + String(message_count == 1 ? "" : "s") + " Detected";
					const String msg = "Please note the following warning message" + String(message_count == 1 ? "" : "s") + ":" + ss.str();
					const int result = AlertWindow::showOkCancelBox(AlertWindow::AlertIconType::QuestionIcon, title, msg, "Load", "Cancel");
					load_project = (result == 1 ? true : false);
				}
			}

			if (load_project)
			{
				setVisible(false);

				const std::string prefix = "project_" + key.toStdString() + "_";

				cfg().setValue(prefix + "inclusion_regex"		, notebook_canvas->inclusion_regex					);
				cfg().setValue(prefix + "exclusion_regex"		, notebook_canvas->exclusion_regex					);
				cfg().setValue(prefix + "cfg"					, notebook_canvas->darknet_configuration_filename	);
				cfg().setValue(prefix + "weights"				, notebook_canvas->darknet_weights_filename			);
				cfg().setValue(prefix + "names"					, notebook_canvas->darknet_names_filename			);
				cfg().setValue(prefix + "darknet_cfg_template"	, notebook_canvas->darknet_configuration_template	);
				cfg().setValue(prefix + "timestamp"				, static_cast<int>(std::time(nullptr))				);

				// Prior to 2020-05-30, there were many settings in configuration that were "global".  Every time a project
				// was loaded, the settings were copied over to the global name.  This mess is because when it was first
				// written, DarkMark didn't manage multiple projects.  It only managed a single project.
				//
				// All settings are now per-project.  So if we have one of the original project .cfg files that we can parse
				// and obtain some of the key values, do this now so the user can re-save the darknet project and yet keep
				// the settings they previously used with this project.
				if (File(cfg_filename).existsAsFile())
				{
					// regex to find keyword/value pairs such as:   width=416
					const std::regex rgx("^(\\w+)[ \t]*=[ \t]*([0-9.]+)$");

					std::ifstream ifs(cfg_filename.toStdString());
					std::string line;
					size_t line_count = 0; // we'll only read the first few lines of the .cfg file
					while (line_count < 50 and std::getline(ifs, line))
					{
						line_count ++;
						std::smatch matches;
						if (std::regex_match(line, matches, rgx))
						{
							const std::string keyword	= matches[1].str();
							const float value_float		= std::atof(matches[2].str().c_str());
							const bool	value_bool		= (value_float == 0.0 ? false : true);
							const int	value_int		= static_cast<int>(value_float);

							if (keyword == "batch"			) cfg().insert_if_not_exist(prefix + "darknet_batch_size"	, value_int);
							if (keyword == "subdivisions"	) cfg().insert_if_not_exist(prefix + "darknet_subdivisions"	, value_int);
							if (keyword == "width"			) cfg().insert_if_not_exist(prefix + "darknet_image_width"	, value_int);
							if (keyword == "height"			) cfg().insert_if_not_exist(prefix + "darknet_image_height"	, value_int);
							if (keyword == "max_batches"	) cfg().insert_if_not_exist(prefix + "darknet_iterations"	, value_int);
							if (keyword == "learning_rate"	) cfg().insert_if_not_exist(prefix + "darknet_learning_rate", value_float);
							if (keyword == "saturation"		) cfg().insert_if_not_exist(prefix + "darknet_saturation"	, value_float);
							if (keyword == "exposure"		) cfg().insert_if_not_exist(prefix + "darknet_exposure"		, value_float);
							if (keyword == "hue"			) cfg().insert_if_not_exist(prefix + "darknet_hue"			, value_float);
							if (keyword == "flip"			) cfg().insert_if_not_exist(prefix + "darknet_enable_flip"	, value_bool);
							if (keyword == "angle"			) cfg().insert_if_not_exist(prefix + "darknet_angle"		, value_int);
							if (keyword == "mosaic"			) cfg().insert_if_not_exist(prefix + "darknet_mosaic"		, value_bool);
							if (keyword == "cutmix"			) cfg().insert_if_not_exist(prefix + "darknet_cutmix"		, value_bool);
							if (keyword == "mixup"			) cfg().insert_if_not_exist(prefix + "darknet_mixup"		, value_bool);
						}
					}
				}

				dmapp().wnd.reset(new DMWnd(prefix));
				dmapp().startup_wnd.reset(nullptr);
			}
			else
			{
				canvas.setEnabled(true);
			}
		}
	}

	return;
}


bool dm::StartupWnd::keyPressed(const KeyPress & key)
{
	if (key.getKeyCode() == KeyPress::F1Key)
	{
		if (not dmapp().about_wnd)
		{
			dmapp().about_wnd.reset(new AboutWnd);
		}
		dmapp().about_wnd->toFront(true);
		return true; // key has been handled
	}

	if (key.getKeyCode() == KeyPress::escapeKey)
	{
		stop_refreshing_all_notebook_tabs();
		closeButtonPressed();
		return true; // key has been handled
	}

	return false; // false == keystroke not handled
}


void dm::StartupWnd::updateButtons()
{
	const bool enabled = (notebook.getNumTabs() > 0);

	delete_button		.setEnabled(enabled);
	import_video_button	.setEnabled(enabled);
	import_pdf_button	.setEnabled(enabled);
	open_folder_button	.setEnabled(enabled);
	class_id_button		.setEnabled(enabled);
	refresh_button		.setEnabled(enabled);
	ok_button			.setEnabled(enabled);

	return;
}


void dm::StartupWnd::stop_refreshing_all_notebook_tabs()
{
	for (int idx = 0; idx < notebook.getNumTabs(); idx ++)
	{
		StartupCanvas * startup_canvas = dynamic_cast<StartupCanvas*>(notebook.getTabContentComponent(idx));
		if (startup_canvas)
		{
			startup_canvas->done = true;
		}
	}

	return;
}
