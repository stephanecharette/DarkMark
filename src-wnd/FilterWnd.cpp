// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"
#include "json.hpp"
using json = nlohmann::json;


class ButtonSelection : public ButtonPropertyComponent
{
	public:

		ButtonSelection(dm::FilterWnd & parent, const bool state) :
			ButtonPropertyComponent("", false),
			fw(parent),
			toggle_state(state)
		{
			return;
		}

		virtual void buttonClicked() override
		{
			for (auto & v : fw.value_for_each_class)
			{
				v = toggle_state;
			}
		}

		virtual String getButtonText () const override
		{
			return (toggle_state ? "select all" : "select none");
		}

		// This is where the configuration template filename needs to be stored.
		dm::FilterWnd & fw;
		const bool toggle_state;
};


dm::FilterWnd::FilterWnd(dm::DMContent & c) :
	DocumentWindow("DarkMark filters for project \"" + c.project_info.project_name + "\"", Colours::darkgrey, TitleBarButtons::allButtons),
	select_all(nullptr),
	select_none(nullptr),
	content(c),
	cancel_button("Cancel"),
	apply_button("Apply"),
	ok_button("OK"),
	worker_thread_needs_to_end(false)
{
	setContentNonOwned		(&canvas, true	);
	setUsingNativeTitleBar	(true			);
	setResizable			(true, false	);
	setDropShadowEnabled	(true			);

	canvas.addAndMakeVisible(pp				);
	canvas.addAndMakeVisible(cancel_button	);
	canvas.addAndMakeVisible(apply_button	);
	canvas.addAndMakeVisible(ok_button		);

	cancel_button.addListener(this);
	apply_button.addListener(this);
	ok_button.addListener(this);

	setIcon(DarkMarkLogo());
	ComponentPeer *peer = getPeer();
	if (peer)
	{
		peer->setIcon(DarkMarkLogo());
	}

	v_inclusion_regex = cfg().get_str(content.project_info.cfg_prefix + "inclusion_regex").c_str();
	v_exclusion_regex = cfg().get_str(content.project_info.cfg_prefix + "exclusion_regex").c_str();
	v_age_of_annotations			= 0;
	v_include_all_classes			= true;
	v_include_empty_images			= true;
	v_include_non_annotated_images	= true;

	v_inclusion_regex				.addListener(this);
	v_exclusion_regex				.addListener(this);
	v_age_of_annotations			.addListener(this);
	v_include_all_classes			.addListener(this);
	v_include_empty_images			.addListener(this);
	v_include_non_annotated_images	.addListener(this);

	Array<PropertyComponent*> properties;
	TextPropertyComponent		* t = nullptr;
	BooleanPropertyComponent	* b = nullptr;
	SliderPropertyComponent		* s = nullptr;

	t = new TextPropertyComponent(v_total_number_of_images, "total number of images", 1000, false, false);
	properties.add(t);
	t = new TextPropertyComponent(v_images_after_regex, "images after regex", 1000, false, false);
	properties.add(t);
	t = new TextPropertyComponent(v_images_after_filters, "images after filters", 1000, false, false);
	properties.add(t);
	t = new TextPropertyComponent(v_usable_images, "usable images", 1000, false, false);
	properties.add(t);

	pp.addSection("images", properties);
	properties.clear();

	t = new TextPropertyComponent(v_inclusion_regex, "inclusion regex", 1000, false, true);
	t->setTooltip("Inclusion regex is used to temporarily include a subset of images from the project. The regex will be applied individually to each image path and filename.\n\nYou cannot set both an inclusion regex and an exclusion regex.");
	properties.add(t);

	t = new TextPropertyComponent(v_exclusion_regex, "exclusion regex", 1000, false, true);
	t->setTooltip("Exclusion regex is used to temporarily exclude a subset of images from the project. The regex will be applied individually to each image path and filename.\n\nYou cannot set both an inclusion regex and an exclusion regex.");
	properties.add(t);

	pp.addSection("regex", properties);
	properties.clear();

	s = new SliderPropertyComponent(v_age_of_annotations, "age of annotations", 0.0, 63072000.0, 60.0, 0.1);
	s->setTooltip(
		"If annotations are older than this (in seconds) then the image and annotation will be excluded. Set to 0 to completely skip this filter.\n\n"
		"0=include all\n"
		"300=5 minutes\n"
		"600=10 minutes\n"
		"3600=1 hour\n"
		"7200=2 hours\n"
		"86400=1 day\n"
		"172800=2 days\n"
		"604800=1 week\n"
		"1209600=2 weeks\n"
		"2592000=1 month\n"
		"5270400=2 months\n"
		"15811200=6 months\n"
		"31536000=1 year\n"
		"63072000=2 years"
		);
	properties.add(s);

	b = new BooleanPropertyComponent(v_include_all_classes, "all classes", "all classes");
	b->setTooltip("Determines whether or not all classes are included or individually filtered. The default value is \"yes\" (include all classes).");
	properties.add(b);

	b = new BooleanPropertyComponent(v_include_empty_images, "empty images", "empty images");
	b->setTooltip("Determines whether or not empty images are included. The default value is \"yes\".");
	properties.add(b);

	b = new BooleanPropertyComponent(v_include_non_annotated_images, "non-annotated images", "non-annotated images");
	b->setTooltip("Determines whether or not images without any annotations are included. The default value is \"yes\".");
	properties.add(b);

	pp.addSection("included", properties);
	properties.clear();

	select_all	= new ButtonSelection(*this, true);
	select_none	= new ButtonSelection(*this, false);
	properties.add(select_all);
	properties.add(select_none);

	value_for_each_class.reserve(content.names.size());
	for (size_t idx = 0; idx < content.names.size(); idx ++)
	{
		if (idx == content.empty_image_name_index)
		{
			continue;
		}
		const auto & name = content.names[idx];
		value_for_each_class.emplace_back();
		auto & v = value_for_each_class.back();
		v.addListener(this);
		v = true;
		b = new BooleanPropertyComponent(v, "#" + std::to_string(idx) + ": \"" + name + "\"", name);
		b->setTooltip("Include images which contain the label \"" + name + "\".\n\nThe filter \"all classes\" must be turned off for this to work.");
		properties.add(b);

		// remember this control so we can enable/disable when the "all classes" checkbox is toggled
		checkbox_for_each_class.push_back(b);
	}
	pp.addSection("classes", properties, false);
	properties.clear();

	// force all the controls to be enabled/disabled correctly as if this has been clicked
	valueChanged(v_include_all_classes);

	auto r = dmapp().wnd->getBounds();
	r = r.withSizeKeepingCentre(400, 400);
	canvas.setBounds(r);
	setBounds(r);

	setVisible(true);

	return;
}


dm::FilterWnd::~FilterWnd()
{
	worker_thread_needs_to_end = true;
	if (worker_thread.joinable())
	{
		worker_thread.join();
	}

	return;
}


void dm::FilterWnd::closeButtonPressed()
{
	// close button

	worker_thread_needs_to_end = true;
	dmapp().filter_wnd.reset(nullptr);

	return;
}


void dm::FilterWnd::userTriedToCloseWindow()
{
	// ALT+F4

	worker_thread_needs_to_end = true;
	closeButtonPressed();

	return;
}


void dm::FilterWnd::resized()
{
	// get the document window to resize the canvas, then we'll deal with the rest of the components
	DocumentWindow::resized();

	const int margin_size = 5;

	FlexBox button_row;
	button_row.flexDirection = FlexBox::Direction::row;
	button_row.justifyContent = FlexBox::JustifyContent::flexEnd;
	button_row.items.add(FlexItem(cancel_button	).withWidth(100.0).withMargin(FlexItem::Margin(0, margin_size, 0, 0)));
	button_row.items.add(FlexItem(apply_button	).withWidth(100.0).withMargin(FlexItem::Margin(0, margin_size, 0, 0)));
	button_row.items.add(FlexItem(ok_button		).withWidth(100.0));

	FlexBox fb;
	fb.flexDirection = FlexBox::Direction::column;
	fb.items.add(FlexItem(pp).withFlex(1.0));
	fb.items.add(FlexItem(button_row).withHeight(30.0).withMargin(FlexItem::Margin(margin_size, 0, 0, 0)));

	auto r = getLocalBounds();
	r.reduce(margin_size, margin_size);
	fb.performLayout(r);

	return;
}


void dm::FilterWnd::buttonClicked(Button * button)
{
	worker_thread_needs_to_end = true;

	if (button == &ok_button or button == &apply_button)
	{
		content.scrollfield.field = cv::Mat(); // force the scrollfield to be recalculated
		content.image_filenames = filtered_image_filenames;
		content.load_image(0);
		content.sort_order = ESort::kInvalid; // force the full sort logic to run
		content.set_sort_order(ESort::kAlphabetical);
		content.filter_use_this_subset_of_class_ids = class_ids_to_include;
	}

	if (button != &apply_button)
	{
		closeButtonPressed();
	}

	return;
}


void dm::FilterWnd::valueChanged(Value & value)
{
	// force the worker thread to end since the filters have changed
	worker_thread_needs_to_end = true;
	apply_button.setEnabled(false);
	ok_button	.setEnabled(false);

	v_total_number_of_images	= "-";
	v_images_after_regex		= "-";
	v_images_after_filters		= "-";
	v_usable_images				= "-";

	if (value.refersToSameSourceAs(v_include_all_classes))
	{
		const bool toggle = not v_include_all_classes.getValue();

		select_all	->setEnabled(toggle);
		select_none	->setEnabled(toggle);

		for (auto & checkbox : checkbox_for_each_class)
		{
			checkbox->setEnabled(toggle);
		}
	}

	// request a callback -- in milliseconds -- at which point in time we'll start the worker thread to apply the new filters
	startTimer(500);

	return;
}


void dm::FilterWnd::timerCallback()
{
	// if we get called, then the filters are no longer changing, so tell the worker thread to restart

	stopTimer();

	worker_thread_needs_to_end = true;
	if (worker_thread.joinable())
	{
		Log("joining with previous worker thread");
		worker_thread.join();
	}
	worker_thread_needs_to_end = false;
	Log("starting a new worker thread to apply the updated filters");
	worker_thread = std::thread(&dm::FilterWnd::apply_filters_on_thread, this);

	return;
}


void dm::FilterWnd::apply_filters_on_thread()
{
	DarkMarkApplication::setup_signal_handling();

	std::string last_accessed_filename;

	try
	{
		VStr image_filenames;
		VStr json_filenames;
		VStr images_without_json;

		v_total_number_of_images	= "finding images...";
		v_images_after_regex		= "-";
		v_images_after_filters		= "-";
		v_usable_images				= "-";

		find_files(File(content.project_info.project_dir), image_filenames, json_filenames, images_without_json, worker_thread_needs_to_end);

		Log(std::string(__PRETTY_FUNCTION__) + ": number of images found in " + content.project_info.project_dir + ": " + std::to_string(image_filenames.size()));
		if (worker_thread_needs_to_end)
		{
			Log("cancelling out of worker thread while finding images");
			return;
		}

		v_total_number_of_images	= String(image_filenames.size());
		v_images_after_regex		= "applying regex...";

		const std::string inclusion_regex	= v_inclusion_regex.toString().toStdString();
		const std::string exclusion_regex	= v_exclusion_regex.toString().toStdString();
		const std::string regex_to_use		= inclusion_regex + exclusion_regex;

		if (regex_to_use.empty() == false)
		{
			Log(std::string(__PRETTY_FUNCTION__) + ": applying regex \"" + regex_to_use + "\"");

			try
			{
				const std::regex rx(regex_to_use);
				VStr v;
				for (auto && fn : image_filenames)
				{
					if (worker_thread_needs_to_end)
					{
						Log("cancelling out of worker thread while applying regex");
						return;
					}

					last_accessed_filename = fn;

					if (std::regex_search(fn, rx) == exclusion_regex.empty())
					{
						v.push_back(fn);
					}
				}
				last_accessed_filename.clear();


				if (v.size() != image_filenames.size())
				{
					v.swap(image_filenames);
				}
				Log(std::string(__PRETTY_FUNCTION__) + ": done applying regex: \"" + regex_to_use + "\"");
			}
			catch (...)
			{
				AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon, "DarkMark Filters", "The \"inclusion regex\" or \"exclusion regex\" has caused an error and has been skipped.");
			}
		}

		v_images_after_regex = String(image_filenames.size());

		if (v_include_empty_images == false)
		{
			Log(std::string(__PRETTY_FUNCTION__) + ": filtering empty images");
			v_images_after_filters	= "filter empty images...";
			VStr v;
			for (const auto & fn : image_filenames)
			{
				if (worker_thread_needs_to_end)
				{
					Log("cancelling out of worker thread while filtering empty images");
					return;
				}

				last_accessed_filename = fn;

				// if we get here then we want to *REMOVE* images which have been marked as empty
				File file = File(fn).withFileExtension(".txt");
				if (file.existsAsFile() and file.getSize() == 0)
				{
					// the zero size tells us this image is a negative sample
					continue;
				}

				v.push_back(fn);
			}

			last_accessed_filename.clear();
			v.swap(image_filenames);
		}

		if (v_include_non_annotated_images == false)
		{
			Log(std::string(__PRETTY_FUNCTION__) + ": filtering non annotated images");
			v_images_after_filters	= "filter non annotated images...";
			VStr v;
			for (const auto & fn : image_filenames)
			{
				if (worker_thread_needs_to_end)
				{
					Log("cancelling out of worker thread while filtering non annotated images");
					return;
				}

				last_accessed_filename = fn;

				// if we get here then we want to *REMOVE* images which have not yet been annotated
				File file = File(fn).withFileExtension(".txt");
				if (file.existsAsFile())
				{
					v.push_back(fn);
				}
			}

			last_accessed_filename.clear();
			v.swap(image_filenames);
		}

		double age = v_age_of_annotations.getValue();
		if (age  > 0.0)
		{
			Log(std::string(__PRETTY_FUNCTION__) + ": filtering annotations for age > " + std::to_string(age));

			const std::time_t now				= std::time(nullptr);
			const std::time_t timestamp_limit	= now - age; // files have to be this timestamp or newer

			VStr v;
			for (const auto & fn : image_filenames)
			{
				if (worker_thread_needs_to_end)
				{
					Log("cancelling out of worker thread while filtering for annotation age");
					return;
				}

				last_accessed_filename = fn;

				// if this is not annotated, then keep it and move to the next image
				const auto json_file = File(fn).withFileExtension(".json");
				if (json_file.existsAsFile() == false)
				{
					v.push_back(fn);
					continue;
				}

				// if we get here then we know we have some sort of annotation

				last_accessed_filename = json_file.getFullPathName().toStdString();
				auto root = json::parse(json_file.loadFileAsString().toStdString());

				const time_t timestamp = root.value("timestamp", 0);
				if (timestamp >= timestamp_limit)
				{
					v.push_back(fn);
				}
			}

			last_accessed_filename.clear();
			v.swap(image_filenames);
		}

		SId class_ids;
		if (v_include_all_classes == false)
		{
			Log(std::string(__PRETTY_FUNCTION__) + ": filtering classes");
			v_images_after_filters = "filter classes...";

			for (size_t idx = 0; idx < value_for_each_class.size(); idx ++)
			{
				if (value_for_each_class[idx].getValue())
				{
					class_ids.insert(idx);
				}
			}

			VStr v;
			for (const auto & fn : image_filenames)
			{
				if (worker_thread_needs_to_end)
				{
					Log("cancelling out of worker thread while filtering classes (looping through images)");
					return;
				}

				last_accessed_filename = fn;

				// if this is not annotated, then keep it and move to the next image
				const auto json_file = File(fn).withFileExtension(".json");
				if (json_file.existsAsFile() == false)
				{
					v.push_back(fn);
					continue;
				}

				last_accessed_filename = json_file.getFullPathName().toStdString();
				auto root = json::parse(json_file.loadFileAsString().toStdString());

				// if this is a negative sample, then keep it and move to the next image
				if (root.value("completely_empty", false))
				{
					v.push_back(fn);
					continue;
				}

				// otherwise, look through the annotations to see if it includes any of the classes we want
				for (const auto & j : root["mark"])
				{
					if (worker_thread_needs_to_end)
					{
						Log("cancelling out of worker thread while filtering classes (reading marks)");
						return;
					}

					if (class_ids.count(j["class_idx"]) > 0)
					{
						v.push_back(fn);
						break;
					}
				}
				last_accessed_filename.clear();
			}

			v.swap(image_filenames);
		}

		if (worker_thread_needs_to_end)
		{
			Log("cancelling out of worker thread (bottom of function)");
		}
		else
		{
			const String size(image_filenames.size());
			Log(std::string(__PRETTY_FUNCTION__) + ": worker thread is ending; filter size=" + size.toStdString());

			v_images_after_filters	= size;
			v_usable_images			= size;

			std::sort(image_filenames.begin(), image_filenames.end());
			filtered_image_filenames.swap(image_filenames);
			class_ids_to_include.swap(class_ids);

			if (filtered_image_filenames.size() > 0)
			{
				// cannot have both inclusion and exclusion regexes set at the same time
				if (v_inclusion_regex.toString().isEmpty() or v_exclusion_regex.toString().isEmpty())
				{
					Log(std::string(__PRETTY_FUNCTION__) + ": worker thread is done, enabling buttons");
					apply_button.setEnabled(true);
					ok_button	.setEnabled(true);
				}
			}
		}

		Log(std::string(__PRETTY_FUNCTION__) + ": worker thread has finished successfully");
	}
	catch (const std::exception & e)
	{
		Log(std::string(__PRETTY_FUNCTION__) + ": worker thread is ending due to exception: " + e.what());
	}
	catch (...)
	{
		Log(std::string(__PRETTY_FUNCTION__) + ": worker thread is ending due to unknown exception");
	}

	if (last_accessed_filename.empty() == false)
	{
		Log(std::string(__PRETTY_FUNCTION__) + ": last file accessed was " + last_accessed_filename);
	}

	return;
}
