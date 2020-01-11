/* DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include "DarkMark.hpp"


dm::StartupWnd::StartupWnd() :
		DocumentWindow("DarkMark v" DARKMARK_VERSION " Launcher", Colours::darkgrey, TitleBarButtons::allButtons),
		add_button("Add..."),
		delete_button("Delete..."),
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
	notebook.setCurrentTabIndex(0);

	add_button		.addListener(this);
	delete_button	.addListener(this);
	ok_button		.addListener(this);
	cancel_button	.addListener(this);

	setIcon(DarkMarkLogo());
	ComponentPeer *peer = getPeer();
	if (peer)
	{
		peer->setIcon(DarkMarkLogo());
	}

	if (cfg().containsKey("StartupWnd"))
	{
		restoreWindowStateFromString(cfg().getValue("StartupWnd"));
	}
	else
	{
		centreWithSize(800, 600);
	}

	setVisible(true);

	return;
}


dm::StartupWnd::~StartupWnd()
{
	for (int idx = 0; idx < notebook.getNumTabs(); idx ++)
	{
		StartupCanvas * canvas = dynamic_cast<StartupCanvas*>(notebook.getTabContentComponent(idx));
		if (canvas)
		{
			canvas->done = true;
		}
	}

	cfg().setValue("StartupWnd", getWindowStateAsString());

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
	// get the document window to resize the canvas, then we'll deal with the rest of the components
	DocumentWindow::resized();

	const int margin_size = 5;

	FlexBox button_row;
	button_row.flexDirection = FlexBox::Direction::row;
	button_row.justifyContent = FlexBox::JustifyContent::flexEnd;
	button_row.items.add(FlexItem(add_button)	.withWidth(100.0).withMargin(FlexItem::Margin(0, margin_size, 0, 0)));
	button_row.items.add(FlexItem(delete_button).withWidth(100.0).withMargin(FlexItem::Margin(0, margin_size, 0, 0)));
	button_row.items.add(FlexItem()				.withFlex(1.0));
	button_row.items.add(FlexItem(cancel_button).withWidth(100.0).withMargin(FlexItem::Margin(0, margin_size, 0, 0)));
	button_row.items.add(FlexItem(ok_button)	.withWidth(100.0));

	FlexBox fb;
	fb.flexDirection = FlexBox::Direction::column;
	fb.items.add(FlexItem(notebook).withFlex(1.0));
	fb.items.add(FlexItem(button_row).withHeight(30.0).withMargin(FlexItem::Margin(margin_size, 0, 0, 0)));

	auto r = getLocalBounds();
	r.reduce(margin_size, margin_size);
	fb.performLayout(r);

	return;
}


void dm::StartupWnd::buttonClicked(Button * button)
{
	if (button == &cancel_button)
	{
		closeButtonPressed();
	}
	else if (button == &add_button)
	{
		File home = File::getSpecialLocation(File::SpecialLocationType::userHomeDirectory);
		File dir = home;

		// attempt to default to the parent directory of the active tab (if we have a tab)
		StartupCanvas * notebook_canvas = dynamic_cast<StartupCanvas*>(notebook.getTabContentComponent(notebook.getCurrentTabIndex()));
		if (notebook_canvas)
		{
			dir = File(notebook_canvas->project_directory.toString()).getParentDirectory();
		}

		FileChooser chooser("Select the new project directory...", dir);
		bool result = chooser.browseForDirectory();
		if (result)
		{
			dir = chooser.getResult();

			// loop through the existing tabs and make sure this directory doesn't already show up
			bool ok = true;
			for (int idx = 0; idx < notebook.getNumTabs(); idx ++)
			{
				notebook_canvas = dynamic_cast<StartupCanvas*>(notebook.getTabContentComponent(idx));
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
				const std::string project_name = dir.getFileNameWithoutExtension().toStdString();
				const std::string names = dir.getChildFile(project_name).withFileExtension(".names").getFullPathName().toStdString();
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

				AlertWindow::showMessageBox(AlertWindow::AlertIconType::InfoIcon, "DarkMark Project", "Successfully added the new project " + dir.getFullPathName() + ".");
				if (need_to_create_new_names_file)
				{
					File(names).revealToUser();
				}
			}
		}
	}
	else if (button == &delete_button)
	{
		const int tab_index = notebook.getCurrentTabIndex();
		StartupCanvas * notebook_canvas = dynamic_cast<StartupCanvas*>(notebook.getTabContentComponent(tab_index));
		if (notebook_canvas)
		{
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
				for (const String k : cfg().getAllProperties().getAllKeys())
				{
					if (k.startsWith(name))
					{
						keys_to_delete.insert(k.toStdString());
					}
				}
				for (const std::string k : keys_to_delete)
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
	}
	else if (button == &ok_button)
	{
		StartupCanvas * notebook_canvas = dynamic_cast<StartupCanvas*>(notebook.getTabContentComponent(notebook.getCurrentTabIndex()));
		if (notebook_canvas)
		{
			canvas.setEnabled(false);

			bool load_project				= true;
			const String key				= notebook_canvas->cfg_key;
			const String cfg_filename		= notebook_canvas->darknet_configuration_filename	.toString();
			const String weights_filename	= notebook_canvas->darknet_weights_filename			.toString();
			const String names_filename		= notebook_canvas->darknet_names_filename			.toString();
			const int image_count			= notebook_canvas->number_of_images					.getValue();

			const bool dir_exists			= File(notebook_canvas->project_directory.toString()).exists();
			const bool cfg_exists			= cfg_filename		.isEmpty()	or File(cfg_filename		).existsAsFile();
			const bool weights_exits		= weights_filename	.isEmpty()	or File(weights_filename	).existsAsFile();
			const bool names_exists			= names_filename	.isEmpty()	or File(names_filename		).existsAsFile();

			size_t warning_count	= 0;
			size_t error_count		= 0;

			std::stringstream ss;
			ss << std::endl << std::endl;
			if (image_count		< 1)		{ ss << "- There are no images in the project directory."	<< std::endl;	error_count		++; }
			if (dir_exists		== false)	{ ss << "- The project directory does not exist."			<< std::endl;	error_count		++; }
			if (cfg_exists		== false)	{ ss << "- The .cfg file does not exist."					<< std::endl;	warning_count	++; }
			if (weights_exits	== false)	{ ss << "- The .weights file does not exist."				<< std::endl;	warning_count	++; }
			if (names_exists	== false)	{ ss << "- The .names file does not exist."					<< std::endl;	error_count		++; }

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

			if (load_project)
			{
				setVisible(false);

				cfg().setValue("darknet_config"					, notebook_canvas->darknet_configuration_filename	);
				cfg().setValue("darknet_weights"				, notebook_canvas->darknet_weights_filename			);
				cfg().setValue("darknet_names"					, notebook_canvas->darknet_names_filename			);
				cfg().setValue("image_directory"				, notebook_canvas->project_directory				);
				cfg().setValue("project_" + key + "_timestamp"	, static_cast<int>(std::time(nullptr))				);
				cfg().setValue("project_" + key + "_cfg"		, notebook_canvas->darknet_configuration_filename	);
				cfg().setValue("project_" + key + "_weights"	, notebook_canvas->darknet_weights_filename			);
				cfg().setValue("project_" + key + "_names"		, notebook_canvas->darknet_names_filename			);

				// see if we can import a few of the critical .cfg settings so that when the user re-creates the darknet
				// configuration files we can do it with the exact same values (or as close as conveniently possible)
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

							if (keyword == "batch"			) cfg().setValue("darknet_batch_size"	, value_int);
							if (keyword == "subdivisions"	) cfg().setValue("darknet_subdivisions"	, value_int);
							if (keyword == "width"			) cfg().setValue("darknet_image_size"	, value_int);
							if (keyword == "max_batches"	) cfg().setValue("darknet_iterations"	, value_int);
							if (keyword == "saturation"		) cfg().setValue("darknet_saturation"	, value_float);
							if (keyword == "exposure"		) cfg().setValue("darknet_exposure"		, value_float);
							if (keyword == "hue"			) cfg().setValue("darknet_hue"			, value_float);
//							if (keyword == "hue"			) cfg().setValue("darknet_enable_hue"	, value_bool);
							if (keyword == "flip"			) cfg().setValue("darknet_enable_flip"	, value_bool);
							if (keyword == "angle"			) cfg().setValue("darknet_angle"		, value_int);
							if (keyword == "mosaic"			) cfg().setValue("darknet_mosaic"		, value_bool);
							if (keyword == "cutmix"			) cfg().setValue("darknet_cutmix"		, value_bool);
							if (keyword == "mixup"			) cfg().setValue("darknet_mixup"		, value_bool);
						}
					}
				}

				dmapp().wnd.reset(new DMWnd);
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
			dmapp().about_wnd.reset(new DMAboutWnd);
		}
		dmapp().about_wnd->toFront(true);
		return true; // key has been handled
	}

	if (key.getKeyCode() == KeyPress::escapeKey)
	{
		closeButtonPressed();
		return true; // key has been handled
	}

	return false; // false == keystroke not handled
}
