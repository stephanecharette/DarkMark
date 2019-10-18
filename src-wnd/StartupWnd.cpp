/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include "DarkMark.hpp"


dm::StartupWnd::StartupWnd() :
		DocumentWindow("DarkMark v" DARKMARK_VERSION, Colours::darkgrey, TitleBarButtons::closeButton),
		add_button("Add..."),
		ok_button("OK"),
		cancel_button("Cancel")
{
	setContentNonOwned		(&canvas, true	);
	setUsingNativeTitleBar	(true			);
	setResizable			(true, false	);
	setDropShadowEnabled	(true			);

	canvas.addAndMakeVisible(notebook);
	canvas.addAndMakeVisible(add_button);
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
	ok_button		.addListener(this);
	cancel_button	.addListener(this);

	/// @todo add button not yet supported
	add_button.setEnabled(false);

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

	dmapp().startup_wnd.reset(nullptr);

	return;
}


void dm::StartupWnd::userTriedToCloseWindow()
{
	// ALT+F4

	dmapp().startup_wnd.reset(nullptr);

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
		canvas.setEnabled(false);
		closeButtonPressed();
		dmapp().quit();
	}
	else if (button == &add_button)
	{
		/// @todo
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

			const bool dir_exists			= File(notebook_canvas->project_directory.toString()).exists();
			const bool cfg_exists			= cfg_filename		.isEmpty()	or File(cfg_filename		).existsAsFile();
			const bool weights_exits		= weights_filename	.isEmpty()	or File(weights_filename	).existsAsFile();
			const bool names_exists			= names_filename	.isEmpty()	or File(names_filename		).existsAsFile();

			size_t error_count = 0;
			std::stringstream ss;
			ss << std::endl << std::endl;
			if (dir_exists		== false)	{ ss << "- The project directory does not exist."	<< std::endl;	error_count ++; }
			if (cfg_exists		== false)	{ ss << "- The .cfg file does not exist."			<< std::endl;	error_count ++; }
			if (weights_exits	== false)	{ ss << "- The .weights file does not exist."		<< std::endl;	error_count ++; }
			if (names_exists	== false)	{ ss << "- The .names file does not exist."			<< std::endl;	error_count ++; }
			if (error_count > 0)
			{
				ss << std::endl << "Continue to load this project?";
				const String title = "DarkMark Project Error" + String(error_count == 1 ? "" : "s") + " Detected";
				const String msg = "Please note the following error message" + String(error_count == 1 ? "" : "s") + ":" + ss.str();
				const int result = AlertWindow::showOkCancelBox(AlertWindow::AlertIconType::QuestionIcon, title, msg, "Load", "Cancel");
				load_project = (result == 1 ? true : false);
			}

			if (load_project)
			{
				cfg().setValue("darknet_config"					, notebook_canvas->darknet_configuration_filename	);
				cfg().setValue("darknet_weights"				, notebook_canvas->darknet_weights_filename			);
				cfg().setValue("darknet_names"					, notebook_canvas->darknet_names_filename			);
				cfg().setValue("image_directory"				, notebook_canvas->project_directory				);
				cfg().setValue("project_" + key + "_timestamp"	, static_cast<int>(std::time(nullptr))				);
				cfg().setValue("project_" + key + "_cfg"		, notebook_canvas->darknet_configuration_filename	);
				cfg().setValue("project_" + key + "_weights"	, notebook_canvas->darknet_weights_filename			);
				cfg().setValue("project_" + key + "_names"		, notebook_canvas->darknet_names_filename			);
				dmapp().wnd.reset(new DMWnd);
				closeButtonPressed();
			}
			else
			{
				canvas.setEnabled(true);
			}
		}
	}

	return;
}
