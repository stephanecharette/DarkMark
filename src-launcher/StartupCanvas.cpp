// DarkMark (C) 2019-2021 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"

#include "json.hpp"
using json = nlohmann::json;


dm::StartupCanvas::StartupCanvas(const std::string & key, const std::string & dir) :
	Component("Startup Notebook Canvas"),
	cfg_key(key),
	hide_some_weight_files("hide extra .weights files"),
	applying_filter(true),
	done(false)
{
	addAndMakeVisible(pp);
	addAndMakeVisible(table);
	addAndMakeVisible(thumbnail);
	addAndMakeVisible(hide_some_weight_files);

	table.getHeader().addColumn("filename"	, 1, 100, 30, -1, TableHeaderComponent::notSortable);
	table.getHeader().addColumn("type"		, 2, 100, 30, -1, TableHeaderComponent::notSortable);
	table.getHeader().addColumn("size"		, 3, 100, 30, -1, TableHeaderComponent::notSortable);
	table.getHeader().addColumn("timestamp"	, 4, 100, 30, -1, TableHeaderComponent::notSortable);
	// if changing columns, also update paintCell() below

	table.getHeader().setStretchToFitActive(true);
	table.getHeader().setPopupMenuActive(false);
	table.setModel(this);

	project_directory	= dir.c_str();

	exclusion_regex					.addListener(this);
	darknet_configuration_template	.addListener(this);
	darknet_configuration_filename	.addListener(this);
	darknet_weights_filename		.addListener(this);
	darknet_names_filename			.addListener(this);

	Array<PropertyComponent *> properties;

	properties.add(new TextPropertyComponent(project_directory				, "project directory"		, 1000, false, false));
	properties.add(new TextPropertyComponent(size_of_directory				, "size of directory"		, 1000, false, false));
	properties.add(new TextPropertyComponent(number_of_images				, "image files"				, 1000, false, false));
	properties.add(new TextPropertyComponent(number_of_json					, "markup files"			, 1000, false, false));
	properties.add(new TextPropertyComponent(number_of_classes				, "number of classes"		, 1000, false, false));
	properties.add(new TextPropertyComponent(number_of_marks				, "number of marks"			, 1000, false, false));
	properties.add(new TextPropertyComponent(newest_markup					, "newest markup"			, 1000, false, false));
	properties.add(new TextPropertyComponent(oldest_markup					, "oldest markup"			, 1000, false, false));

	auto tmp = new TextPropertyComponent(exclusion_regex					, "exclusion regex"			, 1000, false, true);
	tmp->setTooltip("Exclusion regex is used to temporarily exclude a subset of images from the project. The regex will be applied individually to each image path and filename.\n\nFor example, set to \"car|truck\" to exclude all images that contains either the text \"car\" or \"truck\" anywhere in the path or filename.\n\nNormally, this field should be left blank.");
	properties.add(tmp);

	tmp = new TextPropertyComponent(darknet_configuration_template			, "darknet template"		, 1000, false, true);
	tmp->setTooltip("The configuration template will be filled in once you select a configuration file within the Darknet Options window. It indicates which Darknet configuration file is used as a template when creating the project's configuration.");
	properties.add(tmp);

	properties.add(new TextPropertyComponent(darknet_configuration_filename	, "darknet configuration"	, 1000, false, true));
	properties.add(new TextPropertyComponent(darknet_weights_filename		, "darknet weights"			, 1000, false, true));
	properties.add(new TextPropertyComponent(darknet_names_filename			, "classes/names"			, 1000, false, true));

	pp.addProperties(properties);
	properties.clear();

	hide_some_weight_files.setToggleState(true, NotificationType::sendNotification);
	hide_some_weight_files.addListener(this);

	refresh();

	return;
}


dm::StartupCanvas::~StartupCanvas()
{
	done = true;
	t.join();

	return;
}


void dm::StartupCanvas::resized()
{
	const int margin_size		= 5;
	const int number_of_lines	= 13;
	const int height_per_line	= 25;
	const int total_pp_height	= number_of_lines * height_per_line;

	FlexBox bottom;
	bottom.flexDirection	= FlexBox::Direction::row;
	bottom.alignContent		= FlexBox::AlignContent::stretch;
	bottom.justifyContent	= FlexBox::JustifyContent::spaceBetween;
	bottom.items.add(FlexItem(thumbnail				).withFlex(3.0));
	bottom.items.add(FlexItem(hide_some_weight_files).withFlex(1.0));

	FlexBox fb;
	fb.flexDirection = FlexBox::Direction::column;
	fb.items.add(FlexItem(pp		).withHeight(total_pp_height));
	fb.items.add(FlexItem(/*spacer*/).withHeight(margin_size));
	fb.items.add(FlexItem(table		).withFlex(3.0));
	fb.items.add(FlexItem(/*spacer*/).withHeight(margin_size));
	fb.items.add(FlexItem(bottom	).withFlex(1.0));

	auto r = getLocalBounds();
	r.reduce(margin_size, margin_size);
	fb.performLayout(r);

	return;
}


void dm::StartupCanvas::initialize_on_thread()
{
	DarkMarkApplication::setup_signal_handling();

	applying_filter = true;
	v.clear();
	table.updateContent();
	table.repaint();

	// give the main window a chance to start up completely before we start pounding the drive looking for files
	std::this_thread::sleep_for(std::chrono::milliseconds(100 + std::rand() % 250));

	File dir(project_directory.toString());

	VStr image_filenames;
	VStr json_filenames;
	VStr images_without_json;
	find_files(dir, image_filenames, json_filenames, images_without_json, done);
	if (not done)
	{
		const size_t image_counter	= image_filenames.size();
		const size_t json_counter	= json_filenames.size();

		number_of_images	= String(image_counter);
		number_of_json		= String(json_counter) + " (" + String(std::round(100.0 * json_counter / (image_counter == 0 ? 1 : image_counter))) + "%)";
	}

	if (image_filenames.empty() == false)
	{
		auto image = ImageCache::getFromFile(File(image_filenames[std::rand() % image_filenames.size()]));
		thumbnail.setImage(image, RectanglePlacement::xLeft);
	}

	find_all_darknet_files();

	calculate_size_of_directory();

	try
	{
		// look for newest and oldest timestamp
		std::time_t oldest = 0;
		std::time_t newest = 0;
		std::size_t count = 0;

		for (const auto & filename : json_filenames)
		{
			if (done)
			{
				break;
			}

			json j = json::parse(File(filename).loadFileAsString().toStdString());
			count += j["mark"].size();
			std::time_t timestamp = j["timestamp"].get<std::time_t>();
			if (oldest == 0 || timestamp < oldest)
			{
				oldest = timestamp;
			}
			if (newest == 0 || timestamp > newest)
			{
				newest = timestamp;
			}
		}

		char buffer[50] = "";
		if (oldest > 0)
		{
			auto tm = localtime(&oldest);
			std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm);
		}
		oldest_markup = buffer;

		buffer[0] = '\0';
		if (newest > 0)
		{
			auto tm = localtime(&newest);
			std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm);
		}
		newest_markup = buffer;

		const int classes = number_of_classes.getValue();
		std::stringstream ss;
		ss << std::fixed << std::setprecision(1);
		ss << count;
		if (classes > 0 and count > 0)
		{
			const double average_marks_per_class = static_cast<double>(count) / static_cast<double>(classes);
			const double average_marks_per_image = static_cast<double>(count) / static_cast<double>(json_filenames.size());

			ss	<< " (average of "
				<< average_marks_per_class << " mark" << (average_marks_per_class == 1.0 ? "" : "s") << " per class or "
				<< average_marks_per_image << " mark" << (average_marks_per_image == 1.0 ? "" : "s") << " per image)";
		}
		number_of_marks = ss.str().c_str();
	}
	catch (const std::exception & e)
	{
		Log(dir.getFullPathName().toStdString() + ": error while reading JSON: " + e.what());
		oldest_markup	= "(error reading markup .json file)";
		newest_markup	= oldest_markup.toString();
		number_of_marks	= oldest_markup.toString();
	}

	return;
}


void dm::StartupCanvas::find_all_darknet_files()
{
	// find all the .cfg, .weight, and .names files

	DarkMarkApplication::setup_signal_handling();

	applying_filter = true;
	v.clear();
	table.updateContent();
	table.repaint();

	extra_weights_files.clear();

	for (const auto type : {DarknetFileInfo::EType::kCfg, DarknetFileInfo::EType::kWeight, DarknetFileInfo::EType::kNames})
	{
		if (done)
		{
			break;
		}

		const String extension =
				type == DarknetFileInfo::EType::kCfg	?	"*.cfg"		:
				type == DarknetFileInfo::EType::kWeight	?	"*.weights" :
															"*.name*"	;

		File dir(project_directory.toString());
		for (auto dir_entry : RangedDirectoryIterator(dir, false, extension))
		{
			if (done)
			{
				break;
			}

			File f = dir_entry.getFile();

			DarknetFileInfo dfi;
			dfi.type		= type;
			dfi.full_name	= f.getFullPathName().toStdString();
			dfi.short_name	= f.getFileName().toStdString();
			dfi.file_size	= File::descriptionOfSizeInBytes(f.getSize()).toStdString();
			dfi.timestamp	= f.getLastModificationTime().formatted("%Y-%m-%d %H:%M:%S").toStdString();
			v.push_back(dfi);
		}
	}

	/* Sort by type, then within each type sort by timestamp so the newest files appear at the top.  If the type and
	 * timestamps are the same (e.g., maybe the files were rsync'd without times being maintained) then compare the
	 * names of the files themselves alphabetically so that yolov3-tiny_39000.weights appears prior to
	 * yolov3-tiny_38000.weights.
	 */
	std::sort(v.begin(), v.end(),
			[](const DarknetFileInfo & lhs, const DarknetFileInfo & rhs)
			{
				if (lhs.type < rhs.type)
				{
					return true;
				}
				if (lhs.type == rhs.type)
				{
					// reverse this test so the NEWER files appear at the top prior to the older files
					if (rhs.timestamp < lhs.timestamp)
					{
						return true;
					}

					if (lhs.timestamp == rhs.timestamp)
					{
						return rhs.short_name < lhs.short_name;
					}
				}
				return false;
			} );

	if (hide_some_weight_files.getToggleState())
	{
		filter_out_extra_weight_files();
	}
	else
	{
		applying_filter = false;
		table.updateContent();
	}

	// if we don't have a .cfg, .weights, or .names file to use, then look through the list of files and choose the most recent one
	if (darknet_configuration_filename.toString().isEmpty())
	{
		for (DarknetFileInfo & info : v)
		{
			if (info.type == DarknetFileInfo::EType::kCfg)
			{
				darknet_configuration_filename = info.full_name.c_str();
				break;
			}
		}
	}
	if (darknet_weights_filename.toString().isEmpty())
	{
		for (DarknetFileInfo & info : v)
		{
			if (info.type == DarknetFileInfo::EType::kWeight)
			{
				darknet_weights_filename = info.full_name.c_str();
				break;
			}
		}
	}
	if (darknet_names_filename.toString().isEmpty())
	{
		for (DarknetFileInfo & info : v)
		{
			if (info.type == DarknetFileInfo::EType::kNames)
			{
				darknet_names_filename = info.full_name.c_str();
				break;
			}
		}
	}

	const String fn = darknet_names_filename.toString();
	if (File(fn).existsAsFile())
	{
		VStr v;
		std::ifstream ifs(fn.toStdString());
		std::string line;
		while (std::getline(ifs, line))
		{
			if (line.empty())
			{
				break;
			}
			v.push_back(line);
		}
		number_of_classes = String(v.size());
	}
	else
	{
		number_of_classes = "?";
	}

	return;
}


void dm::StartupCanvas::refresh()
{
	done = true;
	if (t.joinable())
	{
		t.join();
	}

	size_of_directory	= "...";
	number_of_images	= "...";
	number_of_json		= "...";
	number_of_classes	= "...";
	number_of_marks		= "...";
	newest_markup		= "...";
	oldest_markup		= "...";

	exclusion_regex					= cfg().getValue("project_" + cfg_key + "_exclusion_regex"	);
	darknet_configuration_template	= cfg().getValue("project_" + cfg_key + "_darknet_cfg_template"	);
	darknet_configuration_filename	= cfg().getValue("project_" + cfg_key + "_cfg"					);
	darknet_weights_filename		= cfg().getValue("project_" + cfg_key + "_weights"				);
	darknet_names_filename			= cfg().getValue("project_" + cfg_key + "_names"				);

	done = false;
	t = std::thread(&StartupCanvas::initialize_on_thread, this);

	return;
}


int dm::StartupCanvas::getNumRows()
{
	if (applying_filter)
	{
		return 1;
	}

	return v.size();
}


void dm::StartupCanvas::paintRowBackground(Graphics & g, int rowNumber, int width, int height, bool rowIsSelected)
{
	if (rowNumber < 0				or
		rowNumber >= (int)v.size()	)
	{
		// rows are 0-based, columns are 1-based
		return;
	}

	Colour colour = Colours::white;
	if (rowIsSelected)
	{
		colour = Colours::lightblue; // selected rows will have a blue background
	}
	else if (applying_filter)
	{
		colour = Colours::lightyellow;
	}
	else
	{
		// does this row match ones of the key selected lines (cfg, .weights, or .names)?
		const DarknetFileInfo & info = v.at(rowNumber);
		const String full_name = info.full_name;
		if (full_name == darknet_configuration_filename	or
			full_name == darknet_weights_filename		or
			full_name == darknet_names_filename			)
		{
			colour = Colours::lightgreen;
		}
	}

	g.fillAll( colour );

	// draw the cell bottom divider between rows
	g.setColour( Colours::black.withAlpha( 0.5f ) );
	g.drawLine( 0, height, width, height );

	return;
}


void dm::StartupCanvas::paintCell(Graphics & g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
	if (rowNumber < 0				or
		rowNumber >= (int)v.size()	or
		columnId < 1				or
		columnId > 4				)
	{
		// rows are 0-based, columns are 1-based
		return;
	}

	std::string str;
	if (applying_filter)
	{
		if (columnId == 1)
		{
			str = "Calculating MD5 checksums...";
		}
	}
	else
	{
		const DarknetFileInfo & info = v.at(rowNumber);

		/* columns:
		 *		1: filename
		 *		2: type
		 *		3: size
		 *		4: timestamp
		 */
		switch (columnId)
		{
			case 2: str = (	info.type == DarknetFileInfo::EType::kCfg		?	"configuration"		:
							info.type == DarknetFileInfo::EType::kWeight	?	"weights"			:
																				"names"				);	break;
			case 1: str = info.short_name;																break;
			case 3: str = info.file_size;																break;
			case 4: str = info.timestamp;																break;
		}
	}

	// draw the text and the right-hand-side dividing line between cells
	g.setColour( Colours::black );
	Rectangle<int> r(0, 0, width, height);
	g.drawFittedText(str, r.reduced(2), Justification::centredLeft, 1 );

	// draw the divider on the right side of the column
	g.setColour( Colours::black.withAlpha( 0.5f ) );
	g.drawLine( width, 0, width, height );

	return;
}


void dm::StartupCanvas::selectedRowsChanged(int rowNumber)
{
	if (rowNumber < 0				or
		rowNumber >= (int)v.size()	)
	{
		// rows are 0-based, columns are 1-based
		return;
	}

	const DarknetFileInfo & info = v.at(rowNumber);

	switch (info.type)
	{
		case DarknetFileInfo::EType::kCfg:
		{
			darknet_configuration_filename = info.full_name.c_str();
			break;
		}
		case DarknetFileInfo::EType::kWeight:
		{
			darknet_weights_filename = info.full_name.c_str();
			break;
		}
		case DarknetFileInfo::EType::kNames:
		{
			darknet_names_filename = info.full_name.c_str();
			break;
		}
	}

	table.repaint();

	return;
}


void dm::StartupCanvas::cellClicked(int rowNumber, int columnId, const MouseEvent & event)
{
	if (event.mods.isPopupMenu())
	{
		PopupMenu m;
		m.addItem("delete unused and duplicate .weights files", (extra_weights_files.size() > 0), false, std::function<void()>( [&]{ delete_extra_weights_files(); }));
		m.showMenuAsync(PopupMenu::Options());
	}

	return;
}


void dm::StartupCanvas::delete_extra_weights_files()
{
	for (const auto & filename : extra_weights_files)
	{
		Log("deleting extra weights file: " + filename);
		File(filename).deleteFile();
	}

	calculate_size_of_directory();

	if (extra_weights_files.empty() == false)
	{
		const size_t count = extra_weights_files.size();
		AlertWindow::showMessageBox(AlertWindow::AlertIconType::InfoIcon, "DarkMark", "Deleted " + std::to_string(count) + " unused or duplicate .weights file" + (count == 1 ? "" : "s") + " from " + project_directory.toString().toStdString() + ".");
		extra_weights_files.clear();
	}

	return;
}


void dm::StartupCanvas::valueChanged(Value & value)
{
	table.repaint();

	return;
}


void dm::StartupCanvas::buttonClicked(Button * button)
{
	if (button == &hide_some_weight_files)
	{
		// this can take a while, so start it on a thread
		// (and in the case where this toggle is set to FALSE, we have no way to get back the entries we deleted)
		done = false;
		t.join();
		t = std::thread(&StartupCanvas::find_all_darknet_files, this);
	}

	return;
}


void dm::StartupCanvas::filter_out_extra_weight_files()
{
	/* Darknet spits out many .weights files when learning ends, such as:
	 *
	 *		mailboxes_yolov3-tiny_40000.weights
	 *		mailboxes_yolov3-tiny_final.weights
	 *		mailboxes_yolov3-tiny_best.weights
	 *		mailboxes_yolov3-tiny_last.weights
	 *
	 * ...all of which may actually be exactly the same.  We want to filter out duplicates.
	 * So we'll store the MD5 hash of each .weights file, that way we'll know if this is
	 * a duplicate.
	 */
	SStr md5s;
	extra_weights_files.clear();

	VDarknetFileInfo new_files;

	applying_filter = true;
	table.updateContent();

	for (auto & info : v)
	{
		if (done)
		{
			break;
		}

		if (info.type != DarknetFileInfo::EType::kWeight)
		{
			new_files.push_back(info);
			continue;
		}

		// otherwise if we get here then we have a .weights file, so check to see if this is one we want to keep

		if (md5s.empty()													or
			info.short_name.find("_best.weights")	!= std::string::npos	or
			info.short_name.find("_last.weights")	!= std::string::npos	or
			info.short_name.find("_final.weights")	!= std::string::npos	)
		{
			Log("calculating MD5 checksum for " + info.full_name);
			const std::string md5 = MD5(File(info.full_name)).toHexString().toStdString();
			if (md5s.count(md5) == 0)
			{
				Log("keeping the file " + info.full_name + " (md5=" + md5 + ")");
				new_files.push_back(info);
				md5s.insert(md5);
			}
			else
			{
				Log("skipping the file due to duplicate MD5 sum: " + info.full_name);
				extra_weights_files.insert(info.full_name);
			}
		}
		else
		{
			Log("skipping the intermediate file " + info.full_name);
			extra_weights_files.insert(info.full_name);
		}
	}

	applying_filter = false;

	v.swap(new_files);
	table.updateContent();

	Log(project_directory.toString().toStdString() + ": number of .weights files to skip: " + std::to_string(extra_weights_files.size()));

	return;
}


void dm::StartupCanvas::calculate_size_of_directory()
{
	int64_t total_bytes = 0;
	int64_t image_cache_bytes = 0;

	File dir(project_directory.toString());
	for (auto dir_entry : RangedDirectoryIterator(dir, true))
	{
		if (done)
		{
			break;
		}

		total_bytes += dir_entry.getFileSize();

		if (dir_entry.getFile().getFullPathName().contains("/darkmark_image_cache/"))
		{
			image_cache_bytes += dir_entry.getFileSize();
		}
	}

	if (not done)
	{
		String str(File::descriptionOfSizeInBytes(total_bytes));
		if (image_cache_bytes > 0)
		{
			str += " (" + File::descriptionOfSizeInBytes(image_cache_bytes) + " of which is in the cache)";
		}

		size_of_directory = str;
	}

	return;
}
