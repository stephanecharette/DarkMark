// DarkMark (C) 2019-2021 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"
#include "yolo_anchors.hpp"


class SaveTask : public ThreadWithProgressWindow
{
	public:

		SaveTask(dm::DarknetWnd & w) :
			ThreadWithProgressWindow("Saving Darknet Files...", true, false),
			wnd(w)
		{
			return;
		}

		void run()
		{
			try
			{
				wnd.info.rebuild();

				size_t number_of_files_train	= 0;
				size_t number_of_files_valid	= 0;
				size_t number_of_skipped_files	= 0;
				size_t number_of_marks			= 0;
				size_t number_of_images_resized	= 0;
				size_t number_of_tiles_created	= 0;
				size_t number_of_zooms_created	= 0;

				setStatusMessage("Creating training and validation files...");
				wnd.create_Darknet_training_and_validation_files(
					*this,
					number_of_files_train,
					number_of_files_valid,
					number_of_skipped_files,
					number_of_marks,
					number_of_images_resized,
					number_of_tiles_created,
					number_of_zooms_created);

				setStatusMessage("Creating configuration files and shell scripts...");
				setProgress(0.333);
				wnd.create_Darknet_configuration_file();
				setProgress(0.667);
				wnd.create_Darknet_shell_scripts();

				setStatusMessage("Done!");
				setProgress(1.0);

				const size_t number_of_images = (wnd.info.train_with_all_images ? number_of_files_train : (number_of_files_train + number_of_files_valid));

				const bool singular = (wnd.content.names.size() == 2); // two because the "empty" class is appended to the names, but it does not get output

				std::stringstream ss;
				ss	<< "The necessary files to run darknet have been saved to " << wnd.info.project_dir << "." << std::endl
					<< std::endl
					<< "There " << (singular ? "is " : "are ") << (wnd.content.names.size() - 1) << " class" << (singular ? "" : "es") << " with a total of "
					<< number_of_files_train << " training images and "
					<< number_of_files_valid << " validation images. The average is "
					<< std::fixed << std::setprecision(2) << double(number_of_marks) / double(number_of_images)
					<< " marks per image." << std::endl
					<< std::endl;

				if (number_of_images_resized)
				{
					ss	<< "The number of images resized to " << wnd.info.image_width << "x" << wnd.info.image_height << ": " << number_of_images_resized << "." << std::endl
						<< std::endl;
				}

				if (number_of_tiles_created)
				{
					ss	<< "The number of new image tiles created: " << number_of_tiles_created << "." << std::endl
						<< std::endl;
				}

				if (number_of_zooms_created)
				{
					ss	<< "The number of random crop & zoom images created: " << number_of_zooms_created << "." << std::endl
						<< std::endl;
				}

				if (number_of_skipped_files)
				{
					ss	<< "IMPORTANT: " << number_of_skipped_files << " images were skipped because they have not yet been annotated." << std::endl
						<< std::endl;
				}

				ss << "Run " << wnd.info.command_filename << " to start the training.";

				dm::Log(ss.str());
				AlertWindow::showMessageBox(AlertWindow::AlertIconType::InfoIcon, "DarkMark", ss.str());
			}
			catch (const std::exception & e)
			{
				std::stringstream ss;
				ss	<< "Exception caught while creating the darknet/YOLO files:" << std::endl
					<< std::endl
					<< e.what();
				dm::Log(ss.str());
				AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "DarkMark", ss.str());
			}
		}

		dm::DarknetWnd & wnd;
};


class CfgTemplateButton : public ButtonPropertyComponent
{
	public:

		virtual ~CfgTemplateButton()
		{
			return;
		}

		CfgTemplateButton(Value & v) : ButtonPropertyComponent("configuration template", false), value(v)
		{
			return;
		}

		virtual void buttonClicked()
		{
			dm::WndCfgTemplates wnd(value);
			const int result = wnd.runModalLoop();

			if (result == 0)
			{
				refresh(); // force the name change to show
			}
		}

		virtual String getButtonText () const
		{
			return value.toString();
		}

		// This is where the configuration template filename needs to be stored.
		Value & value;
};


dm::DarknetWnd::DarknetWnd(dm::DMContent & c) :
	DocumentWindow("DarkMark v" DARKMARK_VERSION " - Darknet Output", Colours::darkgrey, TitleBarButtons::closeButton),
	content(c),
	info(c.project_info),
	help_button("Help"),
	ok_button("OK"),
	cancel_button("Cancel")
{
	percentage_slider			= nullptr;
	recalculate_anchors_toggle	= nullptr;
	class_imbalance_toggle		= nullptr;

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
	v_cfg_template					= info.cfg_template.c_str();
	v_train_with_all_images			= info.train_with_all_images;
	v_training_images_percentage	= std::round(100.0 * info.training_images_percentage);
	v_image_width					= info.image_width;
	v_image_height					= info.image_height;
	v_batch_size					= info.batch_size;
	v_subdivisions					= info.subdivisions;
	v_iterations					= info.iterations;
	v_learning_rate					= info.learning_rate;
	v_max_chart_loss				= info.max_chart_loss;
	v_do_not_resize_images			= info.do_not_resize_images;
	v_resize_images					= info.resize_images;
	v_tile_images					= info.tile_images;
	v_zoom_images					= info.zoom_images;
	v_recalculate_anchors			= info.recalculate_anchors;
	v_anchor_clusters				= info.anchor_clusters;
	v_class_imbalance				= info.class_imbalance;
	v_restart_training				= info.restart_training;
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
	v_show_receptive_field			= false;

	// when the template changes, then we need to determine if the YOLO and anchors controls need to be modified
	v_cfg_template.addListener(this);

	// when this value is toggled, we need to enable/disable the image percentage slider
	v_train_with_all_images.addListener(this);

	// counters_per_class depends on this being enabled
	v_recalculate_anchors.addListener(this);

	// handle the strange interactions between the "resize" options
	v_do_not_resize_images	.addListener(this);
	v_resize_images			.addListener(this);
	v_tile_images			.addListener(this);
	v_zoom_images			.addListener(this);

	Array<PropertyComponent *> properties;
	TextPropertyComponent		* t = nullptr;
	BooleanPropertyComponent	* b = nullptr;
	SliderPropertyComponent		* s = nullptr;
	ButtonPropertyComponent		* u = nullptr;

	t = new TextPropertyComponent(v_darknet_dir, "darknet directory", 1000, false, true);
	t->setTooltip("The directory where darknet was built.  Should also contain a 'cfg' directory which contains the example .cfg files to use as templates.");
	properties.add(t);

	u = new CfgTemplateButton(v_cfg_template);
	u->setTooltip("The darknet configuration file to use as a template for this project.");
	properties.add(u);

	pp.addSection("darknet", properties, true);
	properties.clear();

	s = new SliderPropertyComponent(v_image_width, "network width", 32.0, 2048.0, 32.0, 0.5, false);
	s->setTooltip("Network width. Must be a multiple of 32. Default is 448.");
	properties.add(s);

	s = new SliderPropertyComponent(v_image_height, "network height", 32.0, 2048.0, 32.0, 0.5, false);
	s->setTooltip("Network height. Must be a multiple of 32. Default is 256.");
	properties.add(s);

	s = new SliderPropertyComponent(v_batch_size, "batch size", 1.0, 512.0, 1.0, 0.5, false);
	s->setTooltip("Batch size determines the number of images processed per iteration. Default is 64 images per iteration.");
	properties.add(s);

	s = new SliderPropertyComponent(v_subdivisions, "subdivisions", 1.0, 512.0, 1.0, 0.5, false);
	s->setTooltip("The number of images processed in parallel by the GPU is the batch size divided by subdivisions. Default is 8.");
	properties.add(s);

	s = new SliderPropertyComponent(v_iterations, "max_batches", 1000.0, 100000.0, 1.0, 0.5, false);
	s->setTooltip("The total number of iterations to run. As a general rule of thumb, at the very least this should be 2000x more than the number of classes defined.");
	properties.add(s);

	s = new SliderPropertyComponent(v_learning_rate, "learning_rate", 0.000001, 0.999999, 0.000001, 0.25, false);
	s->setTooltip("The learning rate determines the step size at each iteration while moving toward a minimum of a loss function. Since it influences to what extent newly acquired information overrides old information, it metaphorically represents the speed at which a machine learning model \"learns\".");
	properties.add(s);

	s = new SliderPropertyComponent(v_max_chart_loss, "max loss for chart.png", 1.0, 20.0, 0.1);
	s->setTooltip("The maximum amount of loss to display on the output image \"chart.png\". This sets the maximum value to use for the Y-axis, and is for display purposes only; it has zero impact on how the neural network is trained.");
	properties.add(s);

	pp.addSection("configuration", properties, true);
	properties.clear();

	b = new BooleanPropertyComponent(v_do_not_resize_images, "do not resize images", "do not resize images");
	b->setTooltip("Images will be left exactly as they are.  This means Darknet will be responsible for resizing them to match the network dimensions during training.");
	properties.add(b);

	b = new BooleanPropertyComponent(v_resize_images, "resize images", "resize images to match the network dimensions");
	b->setTooltip("DarkMark will automatically resize all the images to match the network dimensions. This speeds up training since Darknet doesn't have to dynamically resize the images while training. This may be combined with the 'tile images' option.");
	properties.add(b);

	b = new BooleanPropertyComponent(v_tile_images, "tile images", "tile images to match the network dimensions");
	b->setTooltip("DarkMark will create new image tiles using the network dimensions. Annotations will automatically be fixed up to match the tiles. This may be combined with the 'resize images' option.");
	properties.add(b);

	b = new BooleanPropertyComponent(v_zoom_images, "crop & zoom images", "random crop and zoom images");
	b->setTooltip("DarkMark will randomly crop and zoom larger images to obtain tiles that match the network dimensions. Annotations will automatically be fixed up to match the tiles. This may be combined with the 'resize images' option.");
	properties.add(b);

	b = new BooleanPropertyComponent(v_train_with_all_images, "train with all images", "train with all images");
	b->setTooltip("Enable this option to use the full list of images for both training and validation, otherwise use the percentage defined below. If you are training your own custom network then you probably want to enable this.");
	properties.add(b);

	s = new SliderPropertyComponent(v_training_images_percentage, "training images %", 50.0, 100.0, 1.0);
	s->setTooltip("Percentage of images to use for training. The remaining images will be used for validation. Default is to use 80% of the images for training, and 20% for validation.");
	percentage_slider = s; // remember this slider, because we need to enable/disable it based on the previous boolean toggle
	if (v_train_with_all_images.getValue())
	{
		s->setEnabled(false);
	}
	properties.add(s);

	pp.addSection("images", properties, true);
	properties.clear();

	b = new BooleanPropertyComponent(v_recalculate_anchors, "recalculate yolo anchors", "recalculate yolo anchors");
	b->setTooltip("Recalculate the best anchors to use given the images, bounding boxes, and network dimensions.");
	recalculate_anchors_toggle = b;
	properties.add(b);

	s = new SliderPropertyComponent(v_anchor_clusters, "number of anchor clusters", 0.0, 20.0, 1.0);
	s->setTooltip("Number of anchor clusters.  If you want to increase or decrease the number of clusters, then you'll need to manually edit the configuration file.");
	s->setValue(9);
	s->setEnabled(false);
	properties.add(s);

	b = new BooleanPropertyComponent(v_class_imbalance, "handle class imbalance", "compensate for class imbalance");
	b->setTooltip("Sets counters_per_class in YOLO sections to balance out the network.");
	class_imbalance_toggle = b;
	if (v_recalculate_anchors.getValue().operator bool() == false)
	{
		v_class_imbalance = false;
		b->setEnabled(false);
	}
	properties.add(b);

	pp.addSection("yolo", properties, false);
	properties.clear();

	std::string name;
	const std::string darknet_weights = cfg().get_str(content.cfg_prefix + "weights");
	if (darknet_weights.empty() == false)
	{
		File f(darknet_weights);
		if (f.existsAsFile())
		{
			name = f.getFileName().toStdString();
		}
	}
	b = new BooleanPropertyComponent(v_restart_training, "re-use existing weights", (name.empty() ? "weights file not found" : name));
	b->setTooltip("Restart training using an existing .weights file (normally *_best.weights). This clears the 'images seen' counter in the weights file to restart training at zero. WARNING: Do not use this if you've added, removed, or re-ordered lines in your .names file since you last trained the network.");
	if (name.empty())
	{
		b->setState(false);
		b->setEnabled(false);
	}
	properties.add(b);

	b = new BooleanPropertyComponent(v_delete_temp_weights, "delete temporary weights", "delete temporary weights");
	b->setTooltip("Delete the temporary weights (1000, 2000, 3000, etc) and keep only the best and final weights.");
	properties.add(b);

	pp.addSection("weights", properties, false);
	properties.clear();

	s = new SliderPropertyComponent(v_saturation, "saturation", 0.0, 10.0, 0.001);
	s->setTooltip("The intensity of the colour. Default value is 1.5.");
	properties.add(s);

	s = new SliderPropertyComponent(v_exposure, "exposure", 0.0, 10.0, 0.001);
	s->setTooltip("The amount of white or black added to a colour to create a new shade. Default value is 1.5.");
	properties.add(s);

	s = new SliderPropertyComponent(v_hue, "hue", 0.0, 1.0, 0.001);
	s->setTooltip("If the network you are training contains classes based on colour, then you'll want to disable this or use a very low value to prevent darknet from altering the hue during data augmentation. Set to 0.5 or higher if the colour does not matter. Default value is 0.1.");
	properties.add(s);

	pp.addSection("data augmentation [colour]", properties, false);
	properties.clear();

	b = new BooleanPropertyComponent(v_enable_flip, "enable flip", "flip (left-right)");
	b->setTooltip("If the network you are training contains objects that have different meaning when flipped/mirrored (left hand vs right hand, 'b' vs 'd', ...) then you'll want to disable this to prevent darknet from mirroring objects during data augmentation. Otherwise, this should be left 'on'.");
	properties.add(b);

	b = new BooleanPropertyComponent(v_mosaic, "enable mosaic", "mosaic");
	b->setTooltip("WARNING: Issue #6105: \"Mosaic degrades accuracy for tiny model.\"");
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

	pp.addSection("data augmentation [misc]", properties, false);
	properties.clear();

	b = new BooleanPropertyComponent(v_keep_augmented_images, "keep images", "keep images");
	b->setTooltip("Save augmented images to disk for review. This adds the \"show_imgs\" flag when training.");
	properties.add(b);

	b = new BooleanPropertyComponent(v_show_receptive_field, "show receptive field", "show receptive field");
	b->setTooltip("Display receptive field debug information on the console when using \"darknet detector test ...\"");
	properties.add(b);

	pp.addSection("darknet debug", properties, false);
	properties.clear();

	auto r = dmapp().wnd->getBounds();
	r = r.withSizeKeepingCentre(550, 600);
	setBounds(r);

	// force some of the handlers to run on the initial config
	valueChanged(v_cfg_template);

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
	button_row.items.add(FlexItem(cancel_button).withWidth(100.0).withMargin(FlexItem::Margin(0, margin_size, 0, margin_size)));
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
		URL url("https://www.ccoderun.ca/DarkMark/darknet_output.html");

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
	const String cfg_template	= v_cfg_template.getValue();
	const int image_width		= v_image_width	.getValue();
	const int image_height		= v_image_height.getValue();
	const int batch_size		= v_batch_size	.getValue();
	const int subdivisions		= v_subdivisions.getValue();

	if (darknet_dir.isEmpty() or File(darknet_dir).exists() == false or File(darknet_dir).getChildFile("cfg").exists() == false)
	{
		AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "DarkMark", "The specified darknet directory is not valid.");
		return;
	}

	if (cfg_template.isEmpty() or File(cfg_template).exists() == false)
	{
		AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "DarkMark", "The configuration template filename is not valid.");
		return;
	}

	if (image_width % 32)
	{
		AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "DarkMark", "The image width must be a multiple of 32.");
		return;
	}

	if (image_height % 32)
	{
		AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "DarkMark", "The image height must be a multiple of 32.");
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

	cfg().setValue(/* this one is a global value */ "darknet_dir"		, v_darknet_dir					);
	cfg().setValue(content.cfg_prefix + "darknet_cfg_template"			, v_cfg_template				);
	cfg().setValue(content.cfg_prefix + "darknet_train_with_all_images"	, v_train_with_all_images		);
	cfg().setValue(content.cfg_prefix + "darknet_training_percentage"	, v_training_images_percentage	);
	cfg().setValue(content.cfg_prefix + "darknet_image_width"			, v_image_width					);
	cfg().setValue(content.cfg_prefix + "darknet_image_height"			, v_image_height				);
	cfg().setValue(content.cfg_prefix + "darknet_batch_size"			, v_batch_size					);
	cfg().setValue(content.cfg_prefix + "darknet_subdivisions"			, v_subdivisions				);
	cfg().setValue(content.cfg_prefix + "darknet_iterations"			, v_iterations					);
	cfg().setValue(content.cfg_prefix + "darknet_learning_rate"			, v_learning_rate				);
	cfg().setValue(content.cfg_prefix + "darknet_max_chart_loss"		, v_max_chart_loss				);
	cfg().setValue(content.cfg_prefix + "darknet_do_not_resize_images"	, v_do_not_resize_images		);
	cfg().setValue(content.cfg_prefix + "darknet_resize_images"			, v_resize_images				);
	cfg().setValue(content.cfg_prefix + "darknet_tile_images"			, v_tile_images					);
	cfg().setValue(content.cfg_prefix + "darknet_zoom_images"			, v_zoom_images					);
	cfg().setValue(content.cfg_prefix + "darknet_recalculate_anchors"	, v_recalculate_anchors			);
	cfg().setValue(content.cfg_prefix + "darknet_anchor_clusters"		, v_anchor_clusters				);
	cfg().setValue(content.cfg_prefix + "darknet_class_imbalance"		, v_class_imbalance				);
	cfg().setValue(content.cfg_prefix + "darknet_restart_training"		, v_restart_training			);
	cfg().setValue(content.cfg_prefix + "darknet_delete_temp_weights"	, v_delete_temp_weights			);
	cfg().setValue(content.cfg_prefix + "darknet_saturation"			, v_saturation					);
	cfg().setValue(content.cfg_prefix + "darknet_exposure"				, v_exposure					);
	cfg().setValue(content.cfg_prefix + "darknet_hue"					, v_hue							);
	cfg().setValue(content.cfg_prefix + "darknet_enable_flip"			, v_enable_flip					);
	cfg().setValue(content.cfg_prefix + "darknet_angle"					, v_angle						);
	cfg().setValue(content.cfg_prefix + "darknet_mosaic"				, v_mosaic						);
	cfg().setValue(content.cfg_prefix + "darknet_cutmix"				, v_cutmix						);
	cfg().setValue(content.cfg_prefix + "darknet_mixup"					, v_mixup						);

	info.darknet_dir				= v_darknet_dir				.toString().toStdString();
	info.cfg_template				= v_cfg_template			.toString().toStdString();
	info.train_with_all_images		= v_train_with_all_images	.getValue();
	info.training_images_percentage	= static_cast<double>(v_training_images_percentage.getValue()) / 100.0;
	info.image_width				= v_image_width				.getValue();
	info.image_height				= v_image_height			.getValue();
	info.batch_size					= v_batch_size				.getValue();
	info.subdivisions				= v_subdivisions			.getValue();
	info.iterations					= v_iterations				.getValue();
	info.learning_rate				= v_learning_rate			.getValue();
	info.max_chart_loss				= v_max_chart_loss			.getValue();
	info.do_not_resize_images		= v_do_not_resize_images	.getValue();
	info.resize_images				= v_resize_images			.getValue();
	info.tile_images				= v_tile_images				.getValue();
	info.zoom_images				= v_zoom_images				.getValue();
	info.recalculate_anchors		= v_recalculate_anchors		.getValue();
	info.anchor_clusters			= v_anchor_clusters			.getValue();
	info.class_imbalance			= v_class_imbalance			.getValue();
	info.restart_training			= v_restart_training		.getValue();
	info.delete_temp_weights		= v_delete_temp_weights		.getValue();
	info.saturation					= v_saturation				.getValue();
	info.exposure					= v_exposure				.getValue();
	info.hue						= v_hue						.getValue();
	info.enable_mosaic				= v_mosaic					.getValue();
	info.enable_cutmix				= v_cutmix					.getValue();
	info.enable_mixup				= v_mixup					.getValue();
	info.enable_flip				= v_enable_flip				.getValue();
	info.angle						= v_angle					.getValue();

	SaveTask save_task(*this);
	save_task.runThread();

	closeButtonPressed();

	return;
}


void dm::DarknetWnd::valueChanged(Value & value)
{
	if (percentage_slider)
	{
		percentage_slider->setEnabled(not v_train_with_all_images.getValue());
	}

	// resize, do-not-resize, and tile need to behave like modified radio buttons
	if (value.refersToSameSourceAs(v_do_not_resize_images))
	{
		if (v_do_not_resize_images.getValue())
		{
			v_resize_images	= false;
			v_tile_images	= false;
			v_zoom_images	= false;
		}
		else if (not v_resize_images.getValue() and not v_tile_images.getValue() and not v_zoom_images.getValue())
		{
			v_resize_images	= true;
			v_tile_images	= true;
			v_zoom_images	= true;
		}
	}
	if (value.refersToSameSourceAs(v_resize_images) or value.refersToSameSourceAs(v_tile_images) or value.refersToSameSourceAs(v_zoom_images))
	{
		if (v_resize_images.getValue() or v_tile_images.getValue() or v_zoom_images.getValue())
		{
			v_do_not_resize_images	= false;
		}
		else
		{
			v_do_not_resize_images = true;
		}
	}

	if (value.refersToSameSourceAs(v_recalculate_anchors))
	{
		const bool should_be_enabled = v_recalculate_anchors.getValue();
		if (class_imbalance_toggle)
		{
			class_imbalance_toggle->setEnabled(should_be_enabled);
		}
		if (should_be_enabled == false)
		{
			v_class_imbalance = false;
		}
	}

	if (value.refersToSameSourceAs(v_cfg_template))
	{
		const std::string filename = v_cfg_template.toString().toStdString();
		cfg_handler.parse(filename);
		const int number_of_clusters = cfg_handler.number_of_anchors_in_yolo();
		if (number_of_clusters <= 1)
		{
			v_recalculate_anchors = false;
			v_anchor_clusters = 0;
			recalculate_anchors_toggle->setEnabled(false);
		}
		else
		{
			v_anchor_clusters = number_of_clusters;
			recalculate_anchors_toggle->setEnabled(true);
		}
	}

	return;
}


void dm::DarknetWnd::create_Darknet_configuration_file()
{
	const size_t number_of_classes		= content.names.size() - 1;
	const bool enable_mosaic			= info.enable_mosaic;
	const bool enable_cutmix			= info.enable_cutmix;
	const bool enable_mixup				= info.enable_mixup;
	const bool enable_flip				= info.enable_flip;
	const float learning_rate			= info.learning_rate;
	const float max_chart_loss			= info.max_chart_loss;
	const float saturation				= info.saturation;
	const float exposure				= info.exposure;
	const float hue						= info.hue;
	const int angle						= info.angle;
	const size_t number_of_iterations	= info.iterations;
	const size_t step1					= std::round(0.8 * number_of_iterations);
	const size_t step2					= std::round(0.9 * number_of_iterations);
	const size_t batch					= info.batch_size;
	const size_t subdivisions			= info.subdivisions;
//	const size_t filters				= number_of_classes * 3 + 15;
	const size_t width					= info.image_width;
	const size_t height					= info.image_height;
	const bool recalculate_anchors		= info.recalculate_anchors;
	const size_t anchor_clusters		= info.anchor_clusters;
	const bool class_imbalance			= info.class_imbalance;

	MStr m =
	{
		{"use_cuda_graph"	, "0"													},
		{"flip"				, enable_flip	? "1" : "0"								},
		{"mosaic"			, enable_mosaic	? "1" : "0"								},
		{"cutmix"			, enable_cutmix	? "1" : "0"								},
		{"mixup"			, enable_mixup	? "1" : "0"								},
		{"learning_rate"	, std::to_string(learning_rate)							},
		{"max_chart_loss"	, std::to_string(max_chart_loss)						},
		{"hue"				, std::to_string(hue)									},
		{"saturation"		, std::to_string(saturation)							},
		{"exposure"			, std::to_string(exposure)								},
		{"max_batches"		, std::to_string(number_of_iterations)					},
		{"steps"			, std::to_string(step1) + "," + std::to_string(step2)	},
		{"batch"			, std::to_string(batch)									},
		{"subdivisions"		, std::to_string(subdivisions)							},
		{"height"			, std::to_string(height)								},
		{"width"			, std::to_string(width)									},
		{"angle"			, std::to_string(angle)									}
	};

	if (v_show_receptive_field.getValue().operator bool())
	{
		m["show_receptive_field"] = "1";
	}

	cfg_handler.modify_all_sections("[net]", m);

	m.clear();
	if (recalculate_anchors)
	{
		std::string new_anchors;
		std::string counters_per_class;
		float avg_iou = 0.0f;

		calc_anchors(info.train_filename, anchor_clusters, info.image_width, info.image_height, number_of_classes, new_anchors, counters_per_class, avg_iou);
		if (avg_iou > 0.0f)
		{
			dm::Log("avg IoU: " + std::to_string(avg_iou));
			dm::Log("new anchors: " + new_anchors);
			dm::Log("new counters: " + counters_per_class);

			m["anchors"] = new_anchors;

			if (class_imbalance)
			{
				m["counters_per_class"] = counters_per_class;
			}

		}
	}

	m["classes"] = std::to_string(number_of_classes);

	cfg_handler.modify_all_sections("[yolo]", m);
	cfg_handler.fix_filters_before_yolo();
	cfg_handler.output(info);

	return;
}


void dm::DarknetWnd::create_Darknet_training_and_validation_files(
		ThreadWithProgressWindow & progress_window,
		size_t & number_of_files_train		,
		size_t & number_of_files_valid		,
		size_t & number_of_skipped_files	,
		size_t & number_of_marks			,
		size_t & number_of_resized_images	,
		size_t & number_of_tiles_created	,
		size_t & number_of_zooms_created	)
{
	if (true)
	{
		std::ofstream fs(info.data_filename);
		fs	<< "classes = "	<< content.names.size() - 1						<< std::endl
			<< "train = "	<< info.train_filename							<< std::endl
			<< "valid = "	<< info.valid_filename							<< std::endl
			<< "names = "	<< cfg().get_str(content.cfg_prefix + "names")	<< std::endl
			<< "backup = "	<< info.project_dir								<< std::endl;
	}

	number_of_files_train		= 0;
	number_of_files_valid		= 0;
	number_of_skipped_files		= 0;
	number_of_marks				= 0;
	number_of_resized_images	= 0;
	number_of_tiles_created		= 0;
	number_of_zooms_created		= 0;

	File dir = File(info.project_dir).getChildFile("darkmark_image_cache");
	dir.deleteRecursively();

	// these vectors will have the full path of the images we need to use (or which have been skipped)
	VStr annotated_images;
	VStr skipped_images;
	VStr all_output_images;
	find_all_annotated_images(progress_window, annotated_images, skipped_images, number_of_marks);
	number_of_skipped_files = skipped_images.size();

	Log("total number of annotated input images: " + std::to_string(annotated_images.size()));
	Log("total number of skipped input images: " + std::to_string(skipped_images.size()));

	if (info.do_not_resize_images)
	{
		Log("not resizing any images");
		all_output_images = annotated_images;
	}
	if (info.resize_images)
	{
		Log("resizing all images");
		resize_images(progress_window, annotated_images, all_output_images, number_of_resized_images);
		Log("number of images resized: " + std::to_string(number_of_resized_images));
	}
	if (info.tile_images)
	{
		Log("tiling all images");
		tile_images(progress_window, annotated_images, all_output_images, number_of_marks, number_of_tiles_created);
		Log("number of tiles created: " + std::to_string(number_of_tiles_created));
	}
	if (info.zoom_images)
	{
		Log("crop+zoom all images");
		random_zoom_images(progress_window, annotated_images, all_output_images, number_of_marks, number_of_zooms_created);
		Log("number of crop+zoom images created: " + std::to_string(number_of_zooms_created));
	}

	// now that we know the exact set of images (including resized and tiled images)
	// we can create the training and validation .txt files

	double work_done = 0.0;
	double work_to_do = all_output_images.size() + 1.0;
	progress_window.setProgress(0.0);
	progress_window.setStatusMessage("Writing training and validation files...");
	Log("total number of output images: " + std::to_string(all_output_images.size()));

	std::random_shuffle(all_output_images.begin(), all_output_images.end());
	const bool use_all_images = info.train_with_all_images;
	number_of_files_train = std::round(info.training_images_percentage * all_output_images.size());
	number_of_files_valid = all_output_images.size() - number_of_files_train;

	if (use_all_images)
	{
		number_of_files_train = all_output_images.size();
		number_of_files_valid = all_output_images.size();
	}

	Log("total number of training images: " + std::to_string(number_of_files_train) + " (" + info.train_filename + ")");
	Log("total number of validation images: " + std::to_string(number_of_files_valid) + " (" + info.valid_filename + ")");

	std::ofstream fs_train(info.train_filename);
	std::ofstream fs_valid(info.valid_filename);

	for (size_t idx = 0; idx < all_output_images.size(); idx ++)
	{
		work_done ++;
		progress_window.setProgress(work_done / work_to_do);

		if (use_all_images or idx < number_of_files_train)
		{
			fs_train << all_output_images[idx] << std::endl;
		}

		if (use_all_images or idx >= number_of_files_train)
		{
			fs_valid << all_output_images[idx] << std::endl;
		}
	}

	Log("training and validation files have been saved to disk");

	return;
}


void dm::DarknetWnd::create_Darknet_shell_scripts()
{
	std::string header;

	if (true)
	{
		std::stringstream ss;
		ss	<< "#!/bin/bash -e"				<< std::endl
			<< ""							<< std::endl
			<< "cd " << info.project_dir	<< std::endl
			<< ""							<< std::endl
			<< "# Warning: this file is automatically created/updated by DarkMark v" << DARKMARK_VERSION << "!" << std::endl
			<< "# Created on " << Time::getCurrentTime().formatted("%a %Y-%m-%d %H:%M:%S %Z").toStdString()
			<< " by " << SystemStats::getLogonName().toStdString()
			<< "@" << SystemStats::getComputerName().toStdString() << "." << std::endl;
		header = ss.str();
	}

	if (true)
	{
		std::string cmd = info.darknet_dir + "/darknet detector -map" + (v_keep_augmented_images.getValue() ? " -show_imgs" : "") + " -dont_show train " + info.data_filename + " " + info.cfg_filename;
		if (info.restart_training)
		{
			cmd += " " + cfg().get_str(content.cfg_prefix + "weights");
			cmd += " -clear";
		}

		std::stringstream ss;
		ss	<< header
			<< ""												<< std::endl
			<< "rm -f output.log"								<< std::endl
			<< "rm -f chart.png"								<< std::endl
			<< ""												<< std::endl
			<< "echo \"creating new log file\" > output.log"	<< std::endl
			<< "date >> output.log"								<< std::endl
			<< ""												<< std::endl
			<< "ts1=$(date)"									<< std::endl
			<< "ts2=$(date +%s)"								<< std::endl
			<< "echo \"initial ts1: ${ts1}\" >> output.log"		<< std::endl
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
			<< ""																								<< std::endl;

			if (info.delete_temp_weights)
			{
				ss	<< "	find " << info.project_dir << " -maxdepth 1 -regex \".+_[0-9]+\\.weights\" -print -delete" << std::endl
					<< "" << std::endl;
			}

		ss	<< "	if [ -e chart.png ]; then"																	<< std::endl
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

	return;
}
