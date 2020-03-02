/* DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include "DarkMark.hpp"


dm::DarknetWnd::DarknetWnd(dm::DMContent & c) :
	DocumentWindow("DarkMark v" DARKMARK_VERSION " - Darknet Output", Colours::darkgrey, TitleBarButtons::closeButton),
	content(c),
	info(c.project_info),
	help_button("Help"),
	ok_button("OK"),
	cancel_button("Cancel")
{
	percentage_slider = nullptr;

	setContentNonOwned		(&canvas, true	);
	setUsingNativeTitleBar	(true			);
	setResizable			(true, false	);
	setDropShadowEnabled	(true			);

	canvas.addAndMakeVisible(pp);
	canvas.addAndMakeVisible(help_button);
	canvas.addAndMakeVisible(ok_button);
	canvas.addAndMakeVisible(cancel_button);

	help_button		.addListener(this);
	ok_button		.addListener(this);
	cancel_button	.addListener(this);

	setIcon(DarkMarkLogo());
	ComponentPeer *peer = getPeer();
	if (peer)
	{
		peer->setIcon(DarkMarkLogo());
	}

	v_darknet_dir					= info.darknet_dir.c_str();
	v_enable_yolov3_tiny			= info.enable_yolov3_tiny;
	v_enable_yolov3_full			= info.enable_yolov3_full;
	v_train_with_all_images			= info.train_with_all_images;
	v_training_images_percentage	= std::round(100.0 * info.training_images_percentage);
	v_image_size					= info.image_size;
	v_batch_size					= info.batch_size;
	v_subdivisions					= info.subdivisions;
	v_iterations					= info.iterations;
	v_resume_training				= info.resume_training;
	v_delete_temp_weights			= info.delete_temp_weights;
	v_saturation					= info.saturation;
	v_exposure						= info.exposure;
	v_hue							= info.hue;
	v_enable_flip					= info.enable_flip;
	v_angle							= 0; // info.angle; -- see https://github.com/AlexeyAB/darknet/issues/4626
	v_mosaic						= info.enable_mosaic;
	v_cutmix						= info.enable_cutmix;
	v_mixup							= info.enable_mixup;
	v_keep_augmented_images			= false;

	// when this value is toggled, we need to enable/disable the image percentage slider
	v_train_with_all_images.addListener(this);

	Array<PropertyComponent *> properties;
	TextPropertyComponent		* t = nullptr;
	BooleanPropertyComponent	* b = nullptr;
	SliderPropertyComponent		* s = nullptr;

	t = new TextPropertyComponent(v_darknet_dir, "darknet directory", 1000, false, true);
	t->setTooltip("The directory where darknet was built.  Should also contain a 'cfg' directory which contains the example YOLO .cfg files.");
	properties.add(t);

	b = new BooleanPropertyComponent(v_enable_yolov3_tiny, "export YOLOv3-tiny", "YOLOv3-tiny");
	b->setTooltip("Using the configuration and data augmentation settings below combined with the YOLOv3-tiny template .cfg file, create a new YOLOv3-tiny configuration file which can be used to train a new network.");
	properties.add(b);

	b = new BooleanPropertyComponent(v_enable_yolov3_full, "export YOLOv3-full", "YOLOv3-full");
	b->setTooltip("Using the configuration and data augmentation settings below combined with the YOLOv3 template .cfg file, create a new YOLOv3-full configuration file which can be used to train a new network.");
	properties.add(b);

	pp.addSection("darknet", properties);
	properties.clear();

	b = new BooleanPropertyComponent(v_train_with_all_images, "train with all images", "train with all images");
	b->setTooltip("Enable this option to use the full list of images for both training and validation (recommended), otherwise use the percentage defined below.");
	properties.add(b);

	s = new SliderPropertyComponent(v_training_images_percentage, "training images %", 50.0, 100.0, 1.0);
	s->setTooltip("Percentage of images to use for training. The remaining images will be used for validation. Default is to use 80% of the images for training, and 20% for validation.");
	percentage_slider = s; // remember this slider, because we need to enable/disable it based on the previous boolean toggle
	if (v_train_with_all_images.getValue())
	{
		s->setEnabled(false);
	}
	properties.add(s);

	s = new SliderPropertyComponent(v_image_size, "image size", 32.0, 2048.0, 32.0);
	s->setTooltip("Image size. Must be a multiple of 32. Default is 416x416.");
	properties.add(s);

	s = new SliderPropertyComponent(v_batch_size, "batch size", 1.0, 512.0, 1.0);
	s->setTooltip("Batch size determines the number of images processed per iteration. Default is 64 images per iteration.");
	properties.add(s);

	s = new SliderPropertyComponent(v_subdivisions, "subdivisions", 1.0, 512.0, 1.0);
	s->setTooltip("The number of images processed in parallel by the GPU is the batch size divided by subdivisions. Default is 8.");
	properties.add(s);

	s = new SliderPropertyComponent(v_iterations, "max_batches", 1000.0, 100000.0, 1.0);
	s->setTooltip("The total number of iterations to run. As a general rule of thumb, at the very least this should be 2000x more than the number of classes defined.");
	properties.add(s);

	pp.addSection("configuration", properties);
	properties.clear();

	std::string name;
	const std::string darknet_weights = cfg().get_str("darknet_weights");
	if (darknet_weights.empty() == false)
	{
		File f(darknet_weights);
		if (f.existsAsFile())
		{
			name = f.getFileName().toStdString();
		}
	}
	b = new BooleanPropertyComponent(v_resume_training, "resume training", (name.empty() ? "weights file not found" : name));
	b->setTooltip("Resume training from an existing .weights file (normally *_best.weights)");
	if (name.empty())
	{
		b->setState(false);
		b->setEnabled(false);
	}
	properties.add(b);

	b = new BooleanPropertyComponent(v_delete_temp_weights, "delete temporary weights", "delete temporary weights");
	b->setTooltip("Delete the temporary weights (1000, 2000, 3000, etc) and keep only the best and final weights.");
	properties.add(b);

	pp.addSection("weights", properties);
	properties.clear();

	s = new SliderPropertyComponent(v_saturation, "saturation", 0.0, 10.0, 0.001);
	s->setTooltip("The intensity of the colour.");
	properties.add(s);

	s = new SliderPropertyComponent(v_exposure, "exposure", 0.0, 10.0, 0.001);
	s->setTooltip("The amount of white or black added to a colour to create a new shade.");
	properties.add(s);

	s = new SliderPropertyComponent(v_hue, "hue", 0.0, 1.0, 0.001);
	s->setTooltip("If the network you are training contains classes based on colour, then you'll want to disable this or use a very low value to prevent darknet from altering the hue during data augmentation. Set to 0.5 or higher if the colour does not matter.");
	properties.add(s);

	pp.addSection("data augmentation [colour]", properties);
	properties.clear();

	b = new BooleanPropertyComponent(v_enable_flip, "enable flip", "flip (left-right)");
	b->setTooltip("If the network you are training contains objects that have different meaning when flipped/mirrored (left hand vs right hand, 'b' vs 'd', ...) then you'll want to disable this to prevent darknet from mirroring objects during data augmentation. Otherwise, this should be left 'on'.");
	properties.add(b);

	b = new BooleanPropertyComponent(v_mosaic, "enable mosaic", "mosaic");
//	b->setTooltip("...?");
	properties.add(b);

	b = new BooleanPropertyComponent(v_cutmix, "enable cutmix", "cutmix");
//	b->setTooltip("...?");
	properties.add(b);

	b = new BooleanPropertyComponent(v_mixup, "enable mixup", "mixup");
//	b->setTooltip("...?");
	properties.add(b);

#if 0
	// This is not yet supported by Darknet.
	// https://github.com/AlexeyAB/darknet/issues/4626
	s = new SliderPropertyComponent(v_angle, "rotation angle", 0.0, 180.0, 1.0);
	s->setTooltip("The number of degrees (+/-) by which the image can be rotated.");
	properties.add(s);
#endif

	pp.addSection("data augmentation [misc]", properties);
	properties.clear();

	b = new BooleanPropertyComponent(v_keep_augmented_images, "keep images", "keep images");
	b->setTooltip("Save augmented images to disk for review. This adds the \"show_imgs\" flag when training.");
	properties.add(b);

	pp.addSection("data augmentation [debug]", properties, false);
	properties.clear();

	auto r = dmapp().wnd->getBounds();
	r = r.withSizeKeepingCentre(550, 650);
	setBounds(r);

	setVisible(true);

	return;
}


dm::DarknetWnd::~DarknetWnd()
{
	percentage_slider = nullptr;

	return;
}


void dm::DarknetWnd::closeButtonPressed()
{
	// close button

	dmapp().darknet_wnd.reset(nullptr);

	return;
}


void dm::DarknetWnd::userTriedToCloseWindow()
{
	// ALT+F4

	dmapp().darknet_wnd.reset(nullptr);

	return;
}


void dm::DarknetWnd::resized()
{
	// get the document window to resize the canvas, then we'll deal with the rest of the components
	DocumentWindow::resized();

	const int margin_size = 5;

	FlexBox button_row;
	button_row.flexDirection = FlexBox::Direction::row;
	button_row.justifyContent = FlexBox::JustifyContent::flexEnd;
	button_row.items.add(FlexItem(help_button).withWidth(100.0));
	button_row.items.add(FlexItem().withFlex(1.0));
	button_row.items.add(FlexItem(cancel_button).withWidth(100.0).withMargin(FlexItem::Margin(0, margin_size, 0, 0)));
	button_row.items.add(FlexItem(ok_button).withWidth(100.0));

	FlexBox fb;
	fb.flexDirection = FlexBox::Direction::column;
	fb.items.add(FlexItem(pp).withFlex(1.0));
	fb.items.add(FlexItem(button_row).withHeight(30.0));

	auto r = getLocalBounds();
	r.reduce(margin_size, margin_size);
	fb.performLayout(r);

	return;
}


void dm::DarknetWnd::buttonClicked(Button * button)
{
	if (button == &help_button)
	{
		URL url("https://www.ccoderun.ca/DarkMark/DataAugmentation.html");
		url.launchInDefaultBrowser();

		return;
	}

	if (button == &cancel_button)
	{
		closeButtonPressed();
		return;
	}

	// otherwise if we get here we need to validate the fields before we export the darknet files

	const String darknet_dir	= v_darknet_dir	.getValue();
	const int image_size		= v_image_size	.getValue();
	const int batch_size		= v_batch_size	.getValue();
	const int subdivisions		= v_subdivisions.getValue();

	if (darknet_dir.isEmpty() or File(darknet_dir).exists() == false or File(darknet_dir).getChildFile("cfg").exists() == false)
	{
		AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "DarkMark", "The specified darknet directory is not valid.");
		return;
	}

	if (image_size % 32)
	{
		AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "DarkMark", "The image size must be a multiple of 32.");
		return;
	}

	if (subdivisions > batch_size)
	{
		AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "DarkMark", "The subdivision must be less than or equal to the batch size.");
		return;
	}

	if (batch_size % subdivisions)
	{
		AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "DarkMark", "The batch size must be a multiple of subdivisions.");
		return;
	}

	canvas.setEnabled(false);

	cfg().setValue("darknet_dir"					, v_darknet_dir					);
	cfg().setValue("darknet_enable_yolov3_tiny"		, v_enable_yolov3_tiny			);
	cfg().setValue("darknet_enable_yolov3_full"		, v_enable_yolov3_full			);
	cfg().setValue("darknet_train_with_all_images"	, v_train_with_all_images		);
	cfg().setValue("darknet_training_percentage"	, v_training_images_percentage	);
	cfg().setValue("darknet_image_size"				, v_image_size					);
	cfg().setValue("darknet_batch_size"				, v_batch_size					);
	cfg().setValue("darknet_subdivisions"			, v_subdivisions				);
	cfg().setValue("darknet_iterations"				, v_iterations					);
	cfg().setValue("darknet_resume_training"		, v_resume_training				);
	cfg().setValue("darknet_delete_temp_weights"	, v_delete_temp_weights			);
	cfg().setValue("darknet_saturation"				, v_saturation					);
	cfg().setValue("darknet_exposure"				, v_exposure					);
	cfg().setValue("darknet_hue"					, v_hue							);
	cfg().setValue("darknet_enable_flip"			, v_enable_flip					);
	cfg().setValue("darknet_angle"					, v_angle						);

	info.darknet_dir				= v_darknet_dir.toString().toStdString();
	info.enable_yolov3_tiny			= v_enable_yolov3_tiny		.getValue();
	info.enable_yolov3_full			= v_enable_yolov3_full		.getValue();
	info.train_with_all_images		= v_train_with_all_images	.getValue();
	info.training_images_percentage	= static_cast<double>(v_training_images_percentage.getValue()) / 100.0;
	info.image_size					= v_image_size				.getValue();
	info.batch_size					= v_batch_size				.getValue();
	info.subdivisions				= v_subdivisions			.getValue();
	info.iterations					= v_iterations				.getValue();
	info.resume_training			= v_resume_training			.getValue();
	info.delete_temp_weights		= v_delete_temp_weights		.getValue();
	info.saturation					= v_saturation				.getValue();
	info.exposure					= v_exposure				.getValue();
	info.hue						= v_hue						.getValue();
	info.enable_mosaic				= v_mosaic					.getValue();
	info.enable_cutmix				= v_cutmix					.getValue();
	info.enable_mixup				= v_mixup					.getValue();
	info.enable_flip				= v_enable_flip				.getValue();
	info.angle						= v_angle					.getValue();

	try
	{
		info.rebuild();

		create_YOLO_configuration_files();
		create_Darknet_files();
	}
	catch (const std::exception & e)
	{
		AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "DarkMark", "Exception caught while creating the darknet/YOLO files:\n\n" + std::string(e.what()));
	}

	closeButtonPressed();

	return;
}


void dm::DarknetWnd::valueChanged(Value & value)
{
	if (percentage_slider)
	{
		percentage_slider->setEnabled(not v_train_with_all_images.getValue());
	}

	return;
}


void dm::DarknetWnd::create_YOLO_configuration_files()
{
	if (info.darknet_dir.empty())
	{
		// nothing we can do without the darknet directory
		return;
	}

	VStr files_to_modify;
	if (info.enable_yolov3_full)
	{
		File(info.darknet_full_cfg_template).copyFileTo(File(info.darknet_full_cfg_filename));
		files_to_modify.push_back(info.darknet_full_cfg_filename);
	}
	if (info.enable_yolov3_tiny)
	{
		File(info.darknet_tiny_cfg_template).copyFileTo(File(info.darknet_tiny_cfg_filename));
		files_to_modify.push_back(info.darknet_tiny_cfg_filename);
	}

	const bool enable_mosaic			= info.enable_mosaic;
	const bool enable_cutmix			= info.enable_cutmix;
	const bool enable_mixup				= info.enable_mixup;
	const bool enable_flip				= info.enable_flip;
	const float saturation				= info.saturation;
	const float exposure				= info.exposure;
	const float hue						= info.hue;
	const int angle						= info.angle;
	const size_t number_of_iterations	= info.iterations;
	const size_t step1					= std::round(0.8 * number_of_iterations);
	const size_t step2					= std::round(0.9 * number_of_iterations);
	const size_t batch					= info.batch_size;
	const size_t subdivisions			= info.subdivisions;
	const size_t filters				= (content.names.size() - 1) * 3 + 15;
	const size_t width					= info.image_size;
	const size_t height					= width;

	for (const std::string & cfg_filename : files_to_modify)
	{
		const VStr commands =
		{
			"sed --in-place \"/^hue *=/ c\\hue="					+ std::to_string(hue)									+ "\" "		+ cfg_filename,
			"sed --in-place \"/^hue *=/ a\\flip="					+ std::string(enable_flip ? "1" : "0")					+ "\" "		+ cfg_filename,
			"sed --in-place \"/^hue *=/ a\\mosaic="					+ std::string(enable_mosaic ? "1" : "0")				+ "\" "		+ cfg_filename,
			"sed --in-place \"/^hue *=/ a\\cutmix="					+ std::string(enable_cutmix ? "1" : "0")				+ "\" "		+ cfg_filename,
			"sed --in-place \"/^hue *=/ a\\mixup="					+ std::string(enable_mixup ? "1" : "0")					+ "\" "		+ cfg_filename,
			"sed --in-place \"/^saturation *=/ c\\saturation="		+ std::to_string(saturation)							+ "\" "		+ cfg_filename,
			"sed --in-place \"/^exposure *=/ c\\exposure="			+ std::to_string(exposure)								+ "\" "		+ cfg_filename,
			"sed --in-place \"/^classes *=/ c\\classes="			+ std::to_string(content.names.size() - 1)				+ "\" "		+ cfg_filename,
			"sed --in-place \"/^max_batches *=/ c\\max_batches="	+ std::to_string(number_of_iterations)					+ "\" "		+ cfg_filename,
			"sed --in-place \"/^steps *=/ c\\steps="				+ std::to_string(step1) + "," + std::to_string(step2)	+ "\" "		+ cfg_filename,
			"sed --in-place \"/^batch *=/ c\\batch="				+ std::to_string(batch)									+ "\" "		+ cfg_filename,
			"sed --in-place \"/^subdivisions *=/ c\\subdivisions="	+ std::to_string(subdivisions)							+ "\" "		+ cfg_filename,
			"sed --in-place \"/^filters *= *255/ c\\filters="		+ std::to_string(filters)								+ "\" "		+ cfg_filename,
			"sed --in-place \"/^height *=/ c\\height="				+ std::to_string(width)									+ "\" "		+ cfg_filename,
			"sed --in-place \"/^width *=/ c\\width="				+ std::to_string(height)								+ "\" "		+ cfg_filename,
			"sed --in-place \"/^angle *=/ c\\angle="				+ std::to_string(angle)									+ "\" "		+ cfg_filename
		};

		for (const std::string & cmd : commands)
		{
			const int rc = system(cmd.c_str());
			if (rc)
			{
				Log("failed to run command (rc=" + std::to_string(rc) + "): " + cmd);
				AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "DarkMark", "Command failed to run:\n\n" + cmd);
				break;
			}
		}
	}

	return;
}


void dm::DarknetWnd::create_Darknet_files()
{
	if (true)
	{
		std::ofstream fs(info.data_filename);
		fs	<< "classes = "	<< content.names.size() - 1			<< std::endl
			<< "train = "	<< info.train_filename				<< std::endl
			<< "valid = "	<< info.valid_filename				<< std::endl
			<< "names = "	<< cfg().get_str("darknet_names")	<< std::endl
			<< "backup = "	<< info.project_dir					<< std::endl;
	}

	size_t number_of_files_train = 0;
	size_t number_of_files_valid = 0;
	size_t number_of_skipped_files = 0;
	size_t number_of_marks = 0;
	if (true)
	{
		// only include the images for which we have at least 1 mark (or have been explicitly marked as empty)
		VStr v;
		VStr skipped;
		for (const auto & filename : content.image_filenames)
		{
			File f = File(filename).withFileExtension(".json");
			const size_t count = content.count_marks_in_json(f);
			if (count == 0)
			{
				number_of_skipped_files ++;
				skipped.push_back(filename);
			}
			else
			{
				number_of_marks += count;
				v.push_back(filename);
			}
		}

		std::random_shuffle(v.begin(), v.end());
		const bool use_all_images = info.train_with_all_images;
		number_of_files_train = std::round(info.training_images_percentage * v.size());
		number_of_files_valid = v.size() - number_of_files_train;

		if (use_all_images)
		{
			number_of_files_train = v.size();
			number_of_files_valid = v.size();
		}

		std::ofstream fs_train(info.train_filename);
		std::ofstream fs_valid(info.valid_filename);

		for (size_t idx = 0; idx < v.size(); idx ++)
		{
			if (use_all_images or idx < number_of_files_train)
			{
				fs_train << v[idx] << std::endl;
			}

			if (use_all_images or idx >= number_of_files_train)
			{
				fs_valid << v[idx] << std::endl;
			}
		}

		const std::string fn = File(info.valid_filename).getSiblingFile("skipped_images.txt").getFullPathName().toStdString();
		std::random_shuffle(skipped.begin(), skipped.end());
		std::ofstream fs_skipped(fn);
		for (const auto & image_filename : skipped)
		{
			fs_skipped << image_filename << std::endl;
		}
	}

	const std::string timestamp = Time::getCurrentTime().formatted("%a %Y-%m-%d %H:%M:%S %Z").toStdString();
	std::string header;

	if (true)
	{
		std::stringstream ss;
		ss	<< "#!/bin/bash -e"				<< std::endl
			<< ""							<< std::endl
			<< "cd " << info.project_dir	<< std::endl
			<< ""							<< std::endl
			<< "# Warning: this file is automatically created/updated by DarkMark v" << DARKMARK_VERSION << "!" << std::endl
			<< "# Created on " << timestamp << " by " << SystemStats::getLogonName().toStdString() << "@" << SystemStats::getComputerName().toStdString() << "." << std::endl;
		header = ss.str();
	}

	if (true)
	{
		std::string cmd = info.darknet_dir + "/darknet detector -map" + (v_keep_augmented_images.getValue() ? " -show_imgs" : "") + " -dont_show train " + info.data_filename + " " + (info.enable_yolov3_tiny ? info.darknet_tiny_cfg_filename : info.darknet_full_cfg_filename);
		if (info.resume_training)
		{
			cmd += " " + cfg().get_str("darknet_weights");
		}

		std::stringstream ss;
		ss	<< header
			<< ""												<< std::endl
			<< "#rm -f " << info.project_name << "*.weights"	<< std::endl
			<< "rm -f output.log"								<< std::endl
			<< "rm -f chart.png"								<< std::endl
			<< ""												<< std::endl
			<< "ts1=$(date)"									<< std::endl
			<< "ts2=$(date +%s)"								<< std::endl
			<< "echo \"initial ts1: ${ts1}\" > output.log"		<< std::endl
			<< "echo \"initial ts2: ${ts2}\" >> output.log"		<< std::endl
			<< "echo \"cmd: " << cmd << "\" >> output.log"		<< std::endl
			<< ""												<< std::endl
			<< "/usr/bin/time --verbose " << cmd << " 2>&1 | tee --append output.log" << std::endl
			<< ""												<< std::endl
			<< "ts3=$(date)"									<< std::endl
			<< "ts4=$(date +%s)"								<< std::endl
			<< "echo \"ts1: ${ts1}\" >> output.log"				<< std::endl
			<< "echo \"ts2: ${ts2}\" >> output.log"				<< std::endl
			<< "echo \"ts3: ${ts3}\" >> output.log"				<< std::endl
			<< "echo \"ts4: ${ts4}\" >> output.log"				<< std::endl
			<< ""												<< std::endl;

		if (info.delete_temp_weights)
		{
			ss	<< "find " << info.project_dir << " -maxdepth 1 -regex \".+_[0-9]+\\.weights\" -print -delete >> output.log" << std::endl
				<< "" << std::endl;
		}

		const std::string data = ss.str();
		File f(info.command_filename);
		f.replaceWithData(data.c_str(), data.size());	// do not use replaceWithText() since it converts the file to CRLF endings which confuses bash
		f.setExecutePermission(true);
	}

	if (true)
	{
		std::stringstream ss;
		ss	<< header
			<< "#"																								<< std::endl
			<< "# This script assumes you have 2 computers:"													<< std::endl
			<< "#"																								<< std::endl
			<< "# - the first is the desktop where you run DarkMark and this script,"							<< std::endl
			<< "# - the second has a decent GPU and is where you train the neural network."						<< std::endl
			<< "#"																								<< std::endl
			<< "# It also assumes the directory structure for where neural networks are saved"					<< std::endl
			<< "# on disk is identical between both computers."													<< std::endl
			<< "#"																								<< std::endl
			<< "# Running this script *FROM THE DESKTOP COMPUTER* will retrieve the results"					<< std::endl
			<< "# (the .weights files) from 'gpurig' where training took place."								<< std::endl
			<< ""																								<< std::endl
			<< "ping -c 1 -W 1 gpurig >/dev/null 2>&1"															<< std::endl
			<< "if [ $? -ne 0 ]; then"																			<< std::endl
			<< "	echo \"Make sure the hostname 'gpurig' can be resolved or exists in the /etc/hosts file!\""	<< std::endl
			<< "else"																							<< std::endl
			<< "#	rm -f " << info.project_name << "*.weights"													<< std::endl
			<< "#	rm -f output.log"																			<< std::endl
			<< "	rm -f chart.png"																			<< std::endl
			<< ""																								<< std::endl
			<< "	rsync --progress --times --compress gpurig:" << info.project_dir << "/\\* ."				<< std::endl
			<< ""																								<< std::endl
			<< "	if [ -e chart.png ]; then"																	<< std::endl
			<< "		eog chart.png"																			<< std::endl
			<< "	fi"																							<< std::endl
			<< "fi"																								<< std::endl
			<< ""																								<< std::endl;
		const std::string data = ss.str();
		File f = File(info.project_dir).getChildFile("get_results_from_gpu_rig.sh");
		f.replaceWithData(data.c_str(), data.size());
		f.setExecutePermission(true);
	}

	if (true)
	{
		std::stringstream ss;
		ss	<< header
			<< "#"																								<< std::endl
			<< "# This script assumes you have 2 computers:"													<< std::endl
			<< "#"																								<< std::endl
			<< "# - the first is the desktop where you run DarkMark and this script,"							<< std::endl
			<< "# - the second has a decent GPU and is where you train the neural network."						<< std::endl
			<< "#"																								<< std::endl
			<< "# It also assumes the directory structure for where neural networks are saved"					<< std::endl
			<< "# on disk is identical between both computers."													<< std::endl
			<< "#"																								<< std::endl
			<< "# Running this script *FROM THE DESKTOP COMPUTER* will copy all of the"							<< std::endl
			<< "# necessary files (images, .txt, .names, .cfg, etc) from the desktop computer"					<< std::endl
			<< "# to the rig with the decent GPU so you can then start the training process."					<< std::endl
			<< "#"																								<< std::endl
			<< "# After this script has finished running, ssh to the GPU rig and run this to train:"			<< std::endl
			<< "#"																								<< std::endl
			<< "#		" << info.command_filename																<< std::endl
			<< "#"																								<< std::endl
			<< ""																								<< std::endl
			<< "ping -c 1 -W 1 gpurig >/dev/null 2>&1"															<< std::endl
			<< "if [ $? -ne 0 ]; then"																			<< std::endl
			<< "	echo \"Make sure the hostname 'gpurig' can be resolved or exists in the /etc/hosts file!\""	<< std::endl
			<< "else"																							<< std::endl
			<< "	rsync --recursive --progress --times --compress . gpurig:" << info.project_dir				<< std::endl
			<< "fi"																								<< std::endl
			<< ""																								<< std::endl;
		const std::string data = ss.str();
		File f = File(info.project_dir).getChildFile("send_files_to_gpu_rig.sh");
		f.replaceWithData(data.c_str(), data.size());
		f.setExecutePermission(true);
	}

	if (true)
	{
		std::stringstream ss;
		ss	<< "The necessary files to run darknet have been saved to " << info.project_dir << "." << std::endl
			<< std::endl
			<< "There are " << content.names.size() - 1 << " classes with a total of "
			<< number_of_files_train << " training files and "
			<< number_of_files_valid << " validation files. The average is "
			<< std::fixed << std::setprecision(2) << double(number_of_marks) / double(number_of_files_train + number_of_files_valid)
			<< " marks per image." << std::endl
			<< std::endl;

		if (number_of_skipped_files)
		{
			ss	<< "IMPORTANT: " << number_of_skipped_files << " images were skipped because they have not yet been marked." << std::endl
				<< std::endl;
		}

		ss << "Run " << info.command_filename << " to start the training.";

		AlertWindow::showMessageBox(AlertWindow::AlertIconType::InfoIcon, "DarkMark", ss.str());
	}

	return;
}
