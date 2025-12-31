// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"
#include "yolo_anchors.hpp"
#include <random>


// Sponsored change:  simplified interface + Japanese translation.
#if DARKNET_GEN_SIMPLIFIED
constexpr bool simplified_interface = true;
#else
constexpr bool simplified_interface = false;
#endif
constexpr bool normal_interface = not simplified_interface;


void setTooltip(PropertyComponent * component, const String & msg)
{
	if (component and normal_interface)
	{
		component->setTooltip(msg);
	}

	return;
}


class SaveTask : public ThreadWithProgressWindow
{
	public:

		SaveTask(dm::DarknetWnd & w) :
			ThreadWithProgressWindow(dm::getText("TITLE3"), true, false),
			wnd(w)
		{
			return;
		}

		void run()
		{
			try
			{
				wnd.info.rebuild();

				size_t number_of_files_train			= 0;
				size_t number_of_files_valid			= 0;
				size_t number_of_annotated_images		= 0;
				size_t number_of_skipped_files			= 0;
				size_t number_of_marks					= 0;
				size_t number_of_empty_images			= 0;
				size_t number_of_dropped_empty_images	= 0;
				size_t number_of_images_resized			= 0;
				size_t number_of_images_not_resized		= 0;
				size_t number_of_tiles_created			= 0;
				size_t number_of_zooms_created			= 0;
				size_t number_of_dropped_annotations	= 0;

				setStatusMessage(dm::getText("Creating training and validation files..."));
				wnd.create_Darknet_training_and_validation_files(
					*this,
					number_of_files_train,
					number_of_files_valid,
					number_of_annotated_images,
					number_of_skipped_files,
					number_of_marks,
					number_of_empty_images,
					number_of_dropped_empty_images,
					number_of_images_resized,
					number_of_images_not_resized,
					number_of_tiles_created,
					number_of_zooms_created,
					number_of_dropped_annotations);

				setStatusMessage(dm::getText("Creating configuration files and shell scripts..."));
				setProgress(0.333);
				wnd.create_Darknet_configuration_file(*this);
				setProgress(1.0);
				wnd.create_Darknet_shell_scripts();

				setStatusMessage(dm::getText("Done!"));
				setProgress(1.0);

				const bool singular = (wnd.content.names.size() == 2); // two because the "empty" class is appended to the names, but it does not get output

				std::stringstream ss;
				ss	<< "The necessary files to run darknet have been saved to " << wnd.info.project_dir << "." << std::endl
					<< std::endl
					<< "There " << (singular ? "is " : "are ") << (wnd.content.names.size() - 1) << " class" << (singular ? "" : "es") << " with a total of "
					<< number_of_files_train << " training images and "
					<< number_of_files_valid << " validation images. The average is "
					<< std::fixed << std::setprecision(2) << double(number_of_marks) / double(number_of_annotated_images)
					<< " marks per image across a total of " << number_of_annotated_images << " annotated images." << std::endl
					<< std::endl;

				if (number_of_empty_images)
				{
					ss	<< "The number of negative samples (empty images): " << number_of_empty_images << "." << std::endl;
					if (number_of_dropped_empty_images)
					{
						ss << "Additional negative samples dropped/ignored: " << number_of_dropped_empty_images << "." << std::endl;
					}
					ss << std::endl;
				}

				if (number_of_images_resized)
				{
					ss	<< "The number of images resized to " << wnd.info.image_width << "x" << wnd.info.image_height << ": " << number_of_images_resized << "." << std::endl;
					if (number_of_images_not_resized)
					{
						ss << "The number of images already at " << wnd.info.image_width << "x" << wnd.info.image_height << ": " << number_of_images_not_resized << "." << std::endl;
					}
					ss << std::endl;
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

				const double percentage = double(number_of_empty_images) / double(number_of_annotated_images + number_of_empty_images);
				if (percentage < 0.2)
				{
					ss	<< "WARNING: The number of negative samples (empty images) seems unusually low: " << (int)std::round(100.0 * percentage) << "%." << std::endl
						<< std::endl;
				}
				if (percentage > 0.7)
				{
					ss	<< "NOTE: The number of negative samples (empty images) seems unusually high: " << (int)std::round(100.0 * percentage) << "%." << std::endl
						<< std::endl;
				}

				if (wnd.info.remove_small_annotations)
				{
					if (number_of_dropped_annotations == 0)
					{
						ss << "No annotations were dropped." << std::endl << std::endl;
					}
					else
					{
						ss << "WARNING: The number of dropped lines (annotations are too small): " << number_of_dropped_annotations << "." << std::endl << std::endl;
					}
				}

				ss << "Run " << wnd.info.command_filename << " to start the training.";

				dm::Log(ss.str());

				if (normal_interface and dm::dmapp().cli_options["darknet"] != "run")
				{
					AlertWindow::showMessageBox(AlertWindow::AlertIconType::InfoIcon, "DarkMark", ss.str());
				}
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
			dm::dmapp().darknet_wnd->setEnabled(false);

			dm::dmapp().cfg_template_wnd.reset(new dm::WndCfgTemplates(value));
			dm::dmapp().cfg_template_wnd->setVisible(true);
			dm::dmapp().cfg_template_wnd->toFront(true);
		}

		virtual String getButtonText () const
		{
			return value.toString();
		}

		// This is where the configuration template filename needs to be stored.
		Value & value;
};


dm::DarknetWnd::DarknetWnd(dm::DMContent & c) :
	DocumentWindow(getText("TITLE"), Colours::darkgrey, TitleBarButtons::closeButton),
	content(c),
	info(c.project_info),
	help_button(getText("Read Me!")),
	youtube_button("YouTube", DrawableButton::ButtonStyle::ImageOnButtonBackground),
	ok_button(getText("OK")),
	cancel_button(getText("Cancel"))
{
	template_button				= nullptr;
	percentage_slider			= nullptr;
	recalculate_anchors_toggle	= nullptr;
	class_imbalance_toggle		= nullptr;

	setContentNonOwned		(&canvas, true	);
	setUsingNativeTitleBar	(true			);
	setResizable			(true, false	);
	setDropShadowEnabled	(true			);

	addAndMakeVisible(bubble);
	bubble.setColour(BubbleMessageComponent::ColourIds::backgroundColourId, Colours::yellow);
	bubble.setColour(BubbleMessageComponent::ColourIds::outlineColourId, Colours::black);

	canvas.addAndMakeVisible(pp);
	canvas.addAndMakeVisible(help_button);
	canvas.addAndMakeVisible(youtube_button);
	canvas.addAndMakeVisible(ok_button);
	canvas.addAndMakeVisible(cancel_button);

	help_button		.addListener(this);
	youtube_button	.addListener(this);
	ok_button		.addListener(this);
	cancel_button	.addListener(this);

	setIcon(DarkMarkLogo());
	ComponentPeer *peer = getPeer();
	if (peer)
	{
		peer->setIcon(DarkMarkLogo());
	}

	DrawablePath path;

	Path p;
	// the rounded-corner red + white YouTube logo
//	p.addRoundedRectangle(Rectangle<int>(6, 80, 500, 350), 70);
	p.addTriangle(207, 183, 207, 332, 334, 257);
	path.setPath(p);
	path.setFill(Colours::white);

	youtube_button.setImages(&path);
	youtube_button.setTooltip("Watch the tutorial on YouTube.");
	youtube_button.setColour(TextButton::ColourIds::buttonColourId, Colours::red);

	help_button.setTooltip("Read the documentation on the web site.");

	if (info.batch_size <= 1)
	{
		info.batch_size = 64;
	}

	v_cfg_template					= info.cfg_template.c_str();
	v_extra_flags					= info.extra_flags.c_str();
	v_train_with_all_images			= info.train_with_all_images;
	v_training_images_percentage	= std::round(100.0 * info.training_images_percentage);
	v_limit_validation_images		= info.limit_validation_images;
	v_image_width					= info.image_width;
	v_image_height					= info.image_height;
	v_batch_size					= info.batch_size;
	v_subdivisions					= info.subdivisions;
	v_iterations					= info.iterations;
	v_learning_rate					= info.learning_rate;
	v_max_chart_loss				= info.max_chart_loss;
	v_image_type					= info.image_type.c_str();
	v_do_not_resize_images			= info.do_not_resize_images;
	v_resize_images					= info.resize_images;
	v_tile_images					= info.tile_images;
	v_zoom_images					= info.zoom_images;
	v_limit_negative_samples		= info.limit_negative_samples;
	v_remove_small_annotations		= info.remove_small_annotations;
	v_annotation_area_size			= info.annotation_area_size;
	v_recalculate_anchors			= info.recalculate_anchors;
	v_anchor_clusters				= info.anchor_clusters;
	v_class_imbalance				= info.class_imbalance;
	v_restart_training				= info.restart_training;
	v_delete_temp_weights			= info.delete_temp_weights;
	v_save_weights					= info.save_weights;
	v_saturation					= info.saturation;
	v_exposure						= info.exposure;
	v_hue							= info.hue;
	v_enable_flip					= info.enable_flip;
	v_angle							= 0; // info.angle; -- see https://github.com/AlexeyAB/darknet/issues/4626
	v_mosaic						= info.enable_mosaic;
	v_cutmix						= info.enable_cutmix;
	v_mixup							= info.enable_mixup;
	// debug options are not stored in config
	v_verbose_output				= false;
	v_keep_augmented_images			= false;
	v_show_receptive_field			= false;

	// when the template changes, then we need to determine if the YOLO and anchors controls need to be modified
	v_cfg_template			.addListener(this);

	// when this value is toggled, we need to enable/disable the image percentage slider
	v_train_with_all_images	.addListener(this);

	v_remove_small_annotations.addListener(this);
	v_annotation_area_size.addListener(this);

	// counters_per_class depends on this being enabled
	v_recalculate_anchors	.addListener(this);

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
	ChoicePropertyComponent		* dd = nullptr; // drop-down menu

	if (normal_interface)
	{
		u = new CfgTemplateButton(v_cfg_template);
		setTooltip(u, "The darknet configuration file to use as a template for this project.");
		properties.add(u);

		template_button = u;

		t = new TextPropertyComponent(v_extra_flags, getText("extra flags"), 50, false, true);
		setTooltip(t, "Other parameters to pass to Darknet when training.");
		properties.add(t);

		pp.addSection("darknet", properties, true);
		properties.clear();
	}

	s = new SliderPropertyComponent(v_image_width, getText("network width"), 32.0, 2048.0, 32.0, 0.5, false);
	setTooltip(s, "Network width. Must be a multiple of 32. Default is 448.");
	properties.add(s);

	s = new SliderPropertyComponent(v_image_height, getText("network height"), 32.0, 2048.0, 32.0, 0.5, false);
	setTooltip(s, "Network height. Must be a multiple of 32. Default is 256.");
	properties.add(s);

	s = new SliderPropertyComponent(v_batch_size, getText("batch size"), 1.0, 512.0, 1.0, 0.5, false);
	setTooltip(s, "Batch size determines the number of images processed per iteration. Default is 64 images per iteration.");
	properties.add(s);
	highlight_conditions_and_messages.push_back(BubbleInfo(s, 64, -1, "The batch size when training a neural network should usually be set to 64."));
	v_batch_size.addListener(this);

	s = new SliderPropertyComponent(v_subdivisions, getText("subdivisions"), 1.0, 512.0, 1.0, 0.5, false);
	setTooltip(s, "The number of images processed in parallel by the GPU is the batch size divided by subdivisions. Default is 8.");
	properties.add(s);
	highlight_conditions_and_messages.push_back(BubbleInfo(s, 1, 8, "For best results during training, the subdivisions should be as near to \"1\" as possible. The amount of available memory on your GPU may require you to set this higher. The higher the value, the less efficient training will be. See the YOLO FAQ for details."));
	v_subdivisions.addListener(this);

	s = new SliderPropertyComponent(v_iterations, getText("max_batches"), 1000.0, 1000000.0, 1.0, 0.5, false);
	setTooltip(s, "The total number of iterations to run. As a general rule of thumb, at the very least this should be 2000x more than the number of classes defined.");
	properties.add(s);
	highlight_conditions_and_messages.push_back(BubbleInfo(s, 500 * (content.names.size() - 1), 5000 * (content.names.size() - 1), "Max batches should be set to approximately 2000 times the number of classes defined, or until the neural network reaches a reasonable plateau while training."));
	v_iterations.addListener(this);

	s = new SliderPropertyComponent(v_learning_rate, getText("learning_rate"), 0.000001, 0.999999, 0.000001, 0.25, false);
	setTooltip(s, "The learning rate determines the step size at each iteration while moving toward a minimum of a loss function. Since it influences to what extent newly acquired information overrides old information, it metaphorically represents the speed at which a machine learning model \"learns\".");
	properties.add(s);

	if (normal_interface)
	{
		s = new SliderPropertyComponent(v_max_chart_loss, "max loss for chart.png", 1.0, 20.0, 0.1);
		setTooltip(s, "The maximum amount of loss to display on the output image \"chart.png\". This sets the maximum value to use for the Y-axis, and is for display purposes only; it has zero impact on how the neural network is trained.");
		properties.add(s);
	}

	pp.addSection(getText("configuration"), properties, true);
	properties.clear();

	const StringArray choices = {"JPG: real-world images, lossy compression", "PNG: text docs or sharp edges, non-lossy", "both: use a random mix of JPG and PNG"};
	const Array<var> values = {"JPG", "PNG", "both"};
	dd = new ChoicePropertyComponent(v_image_type, "format for cache images", choices, values);
	setTooltip(dd, "Which format to use when saving training images in the DarkMark image cache?  If training a network to detect text or images with very clear sharp edges and lines where JPEG artifacts may cause problems, you likely want \"PNG\" or \"both\".  If using real world images, then you likely want JPG.  If in doubt, choose \"both\" which will work in all situations but saving the cache images takes slightly longer.");
	properties.add(dd);

	b = new BooleanPropertyComponent(v_do_not_resize_images, getText("do not resize images"), getText("do not resize images"));
	setTooltip(b, "Images will be left exactly as they are. This means Darknet will be responsible for resizing them to match the network dimensions during training. This is not recommended.");
	properties.add(b);
	highlight_conditions_and_messages.push_back(BubbleInfo(b, true, "Images should generally be resized prior to training. If you select this option, the original images will be used to train the neural network which will cause training to take much longer than usual to complete."));
	v_do_not_resize_images.addListener(this);

	b = new BooleanPropertyComponent(v_resize_images, getText("resize images"), getText("resize images to match the network dimensions"));
	setTooltip(b, "DarkMark will automatically resize all the images to match the network dimensions. This speeds up training since Darknet doesn't have to dynamically resize the images while training. This may be combined with the 'tile images' option.");
	properties.add(b);

	b = new BooleanPropertyComponent(v_tile_images, getText("tile images"), getText("tile images to match the network dimensions"));
	setTooltip(b, "DarkMark will create new image tiles using the network dimensions. Annotations will automatically be fixed up to match the tiles. This may be combined with the 'resize images' option.");
	properties.add(b);
	highlight_conditions_and_messages.push_back(BubbleInfo(b, true, "Tiling is an advanced feature. If you select this option, you MUST also use DarkHelp for inference. Remember to enable \"tiling\" in DarkHelp, otherwise inference may not work as expected."));
	v_tile_images.addListener(this);

	b = new BooleanPropertyComponent(v_zoom_images, getText("crop & zoom images"), getText("random crop and zoom images"));
	setTooltip(b, "DarkMark will randomly crop and zoom larger images to obtain tiles that match the network dimensions. Annotations will automatically be fixed up to match the tiles. This may be combined with the 'resize images' option.");
	properties.add(b);

	b = new BooleanPropertyComponent(v_limit_negative_samples, getText("limit negative samples"), getText("limit negative samples"));
	setTooltip(b, "Limit the number of negative samples included in the training and validation sets to 50% of the images. This should be enabled.");
	properties.add(b);

	b = new BooleanPropertyComponent(v_train_with_all_images, getText("train with all images"), getText("train with all images"));
	setTooltip(b, "Enable this option to use the full list of images for both training and validation, otherwise use the percentage defined below. If you are training your own custom network then you probably want to enable this.");
	properties.add(b);
	highlight_conditions_and_messages.push_back(BubbleInfo(b, false, "It is highly recommended that this be kept \"on\". Turning it off likely doesn't work the way you think! See the YOLO FAQ for details."));
	v_train_with_all_images.addListener(this);

	s = new SliderPropertyComponent(v_training_images_percentage, getText("training images %"), 50.0, 100.0, 1.0);
	setTooltip(s, "Percentage of images to use for training. The remaining images will be used for validation. Default is to use 80% of the images for training, and 20% for validation.");
	percentage_slider = s; // remember this slider, because we need to enable/disable it based on the previous boolean toggle
	if (v_train_with_all_images.getValue())
	{
		s->setEnabled(false);
	}
	properties.add(s);

	b = new BooleanPropertyComponent(v_remove_small_annotations, getText("remove small annotations"), getText("remove small annotations"));
	setTooltip(b, "Remove annoations which have an *area* (multiply width & height) equal to or less than the size described below. This should be enabled.");
	highlight_conditions_and_messages.push_back(BubbleInfo(b, false, "For best results during training, annotations smaller than 100 square pixels (e.g., 10x10) should be removed. The value that works best for your network is not an exact number but depends on factors such as the configuration used and the amount of contrast in your images & objects."));
	remove_small_annotations_toggle = b;
	properties.add(b);

	s = new SliderPropertyComponent(v_annotation_area_size, getText("annotations to remove"), 16.0, 400.0, 1.0);
	setTooltip(s, "Annotations with an area (multiply width & height) equal to or less than this will be removed. For example, to remove annotations smaller than 10x10 pixels, select \"100\".");
	highlight_conditions_and_messages.push_back(BubbleInfo(s, 64, -1, "For best results during training, annotations smaller than 100 square pixels (e.g., 10x10) should be removed. The value that works best for your network is not an exact number but depends on factors such as the configuration used and the amount of contrast in your images & objects."));
	annotation_area_size_slider = s;
	properties.add(s);

	b = new BooleanPropertyComponent(v_limit_validation_images, getText("limit validation images"), getText("limit validation images"));
	setTooltip(b, "Limit the number of validation images to a maximum based on the number of classes. This should be disabled unless you are running into problems related to an extreme number of validation images.");
	properties.add(b);
	highlight_conditions_and_messages.push_back(BubbleInfo(b, true, "It is highly recommended that this be kept \"off\". This is an advanced feature meant to be used when encountering a specific problem while training. It can negatively impact the training of a neural network if enabled needlessly."));
	v_limit_validation_images.addListener(this);

	pp.addSection(getText("images"), properties, true);
	properties.clear();

	b = new BooleanPropertyComponent(v_recalculate_anchors, getText("recalculate yolo anchors"), getText("recalculate yolo anchors"));
	setTooltip(b, "Recalculate the best anchors to use given the images, bounding boxes, and network dimensions. This should be enabled.");
	recalculate_anchors_toggle = b;
	properties.add(b);
	highlight_conditions_and_messages.push_back(BubbleInfo(b, false, "It is highly recommended that this be kept \"on\"."));
	v_recalculate_anchors.addListener(this);

	if (normal_interface)
	{
		s = new SliderPropertyComponent(v_anchor_clusters, getText("number of anchor clusters"), 0.0, 20.0, 1.0);
		setTooltip(s, "Number of anchor clusters.  If you want to increase or decrease the number of clusters, then you'll need to manually edit the configuration file.");
		s->setValue(9);
		s->setEnabled(false);
		properties.add(s);

		b = new BooleanPropertyComponent(v_class_imbalance, getText("handle class imbalance"), getText("compensate for class imbalance"));
		setTooltip(b, "Sets counters_per_class in YOLO sections to balance out the network. This option problematic and should normally be disabled.");
		class_imbalance_toggle = b;
		if (v_recalculate_anchors.getValue().operator bool() == false)
		{
			v_class_imbalance = false;
			b->setEnabled(false);
		}
		properties.add(b);
		highlight_conditions_and_messages.push_back(BubbleInfo(b, true, "This is an advanced feature, and may have a significant impact on how the neural network is trained. It is recommended that this be kept \"off\"."));
		v_class_imbalance.addListener(this);
	}
	else
	{
		v_class_imbalance = false;
	}

	pp.addSection(getText("yolo"), properties, false);
	properties.clear();

	if (normal_interface)
	{
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
		b = new BooleanPropertyComponent(v_restart_training, getText("re-use existing weights"), (name.empty() ? getText("weights file not found") : name));
		setTooltip(b, "Restart training using an existing .weights file (normally *_best.weights). This clears the 'images seen' counter in the weights file to restart training at zero. WARNING: Do not use this if you've added, removed, or re-ordered lines in your .names file since you last trained the network.");
		if (name.empty())
		{
			b->setState(false);
			b->setEnabled(false);
		}
		properties.add(b);

		b = new BooleanPropertyComponent(v_delete_temp_weights, getText("delete temporary weights"), getText("delete temporary weights"));
		setTooltip(b, "Delete the temporary weights (1000, 2000, 3000, etc) and keep only the best and final weights.");
		properties.add(b);

		s = new SliderPropertyComponent(v_save_weights, getText("how often to save weights"), 0.0, 10000.0, 1.0);
		setTooltip(s, "Number of iterations to elapse before weights are saved to disk.  When set to zero, Darknet will automatically calculate a value to use.  This feature requires Darknet V4 or newer.");
		properties.add(s);

		pp.addSection(getText("weights"), properties, false);
		properties.clear();
	}
	else
	{
		v_restart_training = false;
		v_delete_temp_weights = false;
		v_save_weights = 0;
	}

	s = new SliderPropertyComponent(v_saturation, getText("saturation"), 0.0, 10.0, 0.001);
	setTooltip(s, "The intensity of the colour. Default value is 1.5.");
	properties.add(s);

	s = new SliderPropertyComponent(v_exposure, getText("exposure"), 0.0, 10.0, 0.001);
	setTooltip(s, "The amount of white or black added to a colour to create a new shade. Default value is 1.5.");
	properties.add(s);

	s = new SliderPropertyComponent(v_hue, getText("hue"), 0.0, 1.0, 0.001);
	setTooltip(s, "If the network you are training contains classes based on colour, then you'll want to disable this or use a very low value to prevent darknet from altering the hue during data augmentation. Set to 0.5 or higher if the colour does not matter. Default value is 0.1.");
	properties.add(s);

	pp.addSection(getText("data augmentation [colour]"), properties, false);
	properties.clear();

	b = new BooleanPropertyComponent(v_enable_flip, getText("enable flip"), getText("flip (left-right)"));
	setTooltip(b, "If the network you are training contains objects that have different meaning when flipped/mirrored (left hand vs right hand, 'b' vs 'd', ...) then you'll want to disable this to prevent darknet from mirroring objects during data augmentation. Otherwise, this should be left 'on'.");
	properties.add(b);

	b = new BooleanPropertyComponent(v_mosaic, getText("enable mosaic"), getText("mosaic"));
	setTooltip(b, "WARNING: Issue #6105: \"Mosaic degrades accuracy for tiny model.\"");
	properties.add(b);
	highlight_conditions_and_messages.push_back(BubbleInfo(b, true, "Enabling this option may degrade the accuracy of the neural network."));
	v_mosaic.addListener(this);

	if (normal_interface)
	{
		b = new BooleanPropertyComponent(v_cutmix, getText("enable cutmix"), getText("cutmix"));
//		setTooltip(b, "...?");
		properties.add(b);

		b = new BooleanPropertyComponent(v_mixup, getText("enable mixup"), getText("mixup"));
//		setTooltip(b, "...?");
		properties.add(b);
	}
	else
	{
		v_cutmix = false;
		v_mixup = false;
	}

#if 0
	// This is not yet supported by Darknet.
	// https://github.com/AlexeyAB/darknet/issues/4626
	s = new SliderPropertyComponent(v_angle, "rotation angle", 0.0, 180.0, 1.0);
	setTooltip(s, "The number of degrees (+/-) by which the image can be rotated.");
	properties.add(s);
#endif

	pp.addSection(getText("data augmentation [misc]"), properties, false);
	properties.clear();

	if (normal_interface)
	{
		b = new BooleanPropertyComponent(v_verbose_output, "verbose output", "verbose output");
		setTooltip(b, "This adds the \"verbose\" flag when training.");
		properties.add(b);

		b = new BooleanPropertyComponent(v_keep_augmented_images, "keep images", "keep images");
		setTooltip(b, "Save augmented images to disk for review. This adds the \"show_imgs\" flag when training.");
		properties.add(b);
		highlight_conditions_and_messages.push_back(BubbleInfo(b, true, "This is meant for debug purpose only, not for regular use. This will cause an extreme number of new images to be saved to disk."));
		v_keep_augmented_images.addListener(this);

		b = new BooleanPropertyComponent(v_show_receptive_field, "show receptive field", "show receptive field");
		setTooltip(b, "Display receptive field debug information on the console when using \"darknet detector test ...\"");
		properties.add(b);
		highlight_conditions_and_messages.push_back(BubbleInfo(b, true, "This is meant for debug purpose only, not for regular use."));
		v_show_receptive_field.addListener(this);

		pp.addSection("darknet debug", properties, false);
		properties.clear();
	}

	auto r = Rectangle<int>();
	if (dmapp().wnd->show_window)
	{
		r = dmapp().wnd->getBounds();
	}
	else
	{
		auto * display = Desktop::getInstance().getDisplays().getPrimaryDisplay();
		if (display)
		{
			r = display->userArea;
		}
	}

	if (normal_interface)
	{
		r = r.withSizeKeepingCentre(550, 750);
	}
	else
	{
		help_button.setVisible(false);
		youtube_button.setVisible(false);
		r = r.withSizeKeepingCentre(550, 550);
	}

	setBounds(r);

	// force some of the handlers to run on the initial config
	valueChanged(v_cfg_template);

	setVisible(true);

	if (dmapp().cli_options["darknet"] == "run")
	{
		ok_button.triggerClick();
	}

	return;
}


dm::DarknetWnd::~DarknetWnd()
{
	percentage_slider = nullptr;

	dmapp().cfg_template_wnd.reset(nullptr);

	if (dmapp().cli_options["editor"] == "gen-darknet")
	{
		// since our sole purpose was to run this window, we can completely exit from DarkMark
		dmapp().systemRequestedQuit();
	}

	return;
}


void dm::DarknetWnd::closeButtonPressed()
{
	// close button

	dmapp().cfg_template_wnd.reset(nullptr);
	dmapp().darknet_wnd.reset(nullptr);

	return;
}


void dm::DarknetWnd::userTriedToCloseWindow()
{
	// ALT+F4

	dmapp().darknet_wnd.reset(nullptr);
	dmapp().cfg_template_wnd.reset(nullptr);

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
	button_row.items.add(FlexItem().withWidth(margin_size));
	button_row.items.add(FlexItem(youtube_button).withWidth(30.0));
	button_row.items.add(FlexItem().withFlex(1.0));
	button_row.items.add(FlexItem(cancel_button).withWidth(100.0));
	button_row.items.add(FlexItem().withWidth(margin_size));
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


bool dm::DarknetWnd::keyPressed(const KeyPress & key)
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
		cancel_button.triggerClick();
		return true; // key has been handled
	}

	return false;
}


void dm::DarknetWnd::buttonClicked(Button * button)
{
	if (button == &help_button)
	{
		URL("https://www.ccoderun.ca/DarkMark/darknet_output.html").launchInDefaultBrowser();
		return;
	}

	if (button == &youtube_button)
	{
		URL("https://youtu.be/8Ms9T9Ue2g8").launchInDefaultBrowser();
		return;
	}

	if (button == &cancel_button)
	{
		closeButtonPressed();
		return;
	}

	// otherwise if we get here we need to validate the fields before we export the darknet files

	const String cfg_template	= v_cfg_template.getValue();
	const int image_width		= v_image_width	.getValue();
	const int image_height		= v_image_height.getValue();
	const int batch_size		= v_batch_size	.getValue();
	const int subdivisions		= v_subdivisions.getValue();

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

	cfg().setValue(content.cfg_prefix + "darknet_cfg_template"			, v_cfg_template				);
	cfg().setValue(content.cfg_prefix + "darknet_extra_flags"			, v_extra_flags					);
	cfg().setValue(content.cfg_prefix + "darknet_train_with_all_images"	, v_train_with_all_images		);
	cfg().setValue(content.cfg_prefix + "darknet_training_percentage"	, v_training_images_percentage	);
	cfg().setValue(content.cfg_prefix + "darknet_limit_validation_images",v_limit_validation_images		);
	cfg().setValue(content.cfg_prefix + "darknet_image_width"			, v_image_width					);
	cfg().setValue(content.cfg_prefix + "darknet_image_height"			, v_image_height				);
	cfg().setValue(content.cfg_prefix + "darknet_batch_size"			, v_batch_size					);
	cfg().setValue(content.cfg_prefix + "darknet_subdivisions"			, v_subdivisions				);
	cfg().setValue(content.cfg_prefix + "darknet_iterations"			, v_iterations					);
	cfg().setValue(content.cfg_prefix + "darknet_learning_rate"			, v_learning_rate				);
	cfg().setValue(content.cfg_prefix + "darknet_max_chart_loss"		, v_max_chart_loss				);
	cfg().setValue(content.cfg_prefix + "darknet_image_type"			, v_image_type					);
	cfg().setValue(content.cfg_prefix + "darknet_do_not_resize_images"	, v_do_not_resize_images		);
	cfg().setValue(content.cfg_prefix + "darknet_resize_images"			, v_resize_images				);
	cfg().setValue(content.cfg_prefix + "darknet_tile_images"			, v_tile_images					);
	cfg().setValue(content.cfg_prefix + "darknet_zoom_images"			, v_zoom_images					);
	cfg().setValue(content.cfg_prefix + "darknet_remove_small_annotations", v_remove_small_annotations	);
	cfg().setValue(content.cfg_prefix + "darknet_annotation_area_size"	, v_annotation_area_size		);
	cfg().setValue(content.cfg_prefix + "darknet_limit_negative_samples", v_limit_negative_samples		);
	cfg().setValue(content.cfg_prefix + "darknet_recalculate_anchors"	, v_recalculate_anchors			);
	cfg().setValue(content.cfg_prefix + "darknet_anchor_clusters"		, v_anchor_clusters				);
	cfg().setValue(content.cfg_prefix + "darknet_class_imbalance"		, v_class_imbalance				);
	cfg().setValue(content.cfg_prefix + "darknet_restart_training"		, v_restart_training			);
	cfg().setValue(content.cfg_prefix + "darknet_delete_temp_weights"	, v_delete_temp_weights			);
	cfg().setValue(content.cfg_prefix + "darknet_save_weights"			, v_save_weights				);
	cfg().setValue(content.cfg_prefix + "darknet_saturation"			, v_saturation					);
	cfg().setValue(content.cfg_prefix + "darknet_exposure"				, v_exposure					);
	cfg().setValue(content.cfg_prefix + "darknet_hue"					, v_hue							);
	cfg().setValue(content.cfg_prefix + "darknet_enable_flip"			, v_enable_flip					);
	cfg().setValue(content.cfg_prefix + "darknet_angle"					, v_angle						);
	cfg().setValue(content.cfg_prefix + "darknet_mosaic"				, v_mosaic						);
	cfg().setValue(content.cfg_prefix + "darknet_cutmix"				, v_cutmix						);
	cfg().setValue(content.cfg_prefix + "darknet_mixup"					, v_mixup						);

	info.cfg_template				= v_cfg_template			.toString().toStdString();
	info.extra_flags				= v_extra_flags				.toString().toStdString();
	info.train_with_all_images		= v_train_with_all_images	.getValue();
	info.training_images_percentage	= static_cast<double>(v_training_images_percentage.getValue()) / 100.0;
	info.limit_validation_images	= v_limit_validation_images	.getValue();
	info.image_width				= v_image_width				.getValue();
	info.image_height				= v_image_height			.getValue();
	info.batch_size					= v_batch_size				.getValue();
	info.subdivisions				= v_subdivisions			.getValue();
	info.iterations					= v_iterations				.getValue();
	info.learning_rate				= v_learning_rate			.getValue();
	info.max_chart_loss				= v_max_chart_loss			.getValue();
	info.image_type					= v_image_type				.toString().toStdString();
	info.do_not_resize_images		= v_do_not_resize_images	.getValue();
	info.resize_images				= v_resize_images			.getValue();
	info.tile_images				= v_tile_images				.getValue();
	info.zoom_images				= v_zoom_images				.getValue();
	info.limit_negative_samples		= v_limit_negative_samples	.getValue();
	info.remove_small_annotations	= v_remove_small_annotations.getValue();
	info.annotation_area_size		= v_annotation_area_size	.getValue();
	info.recalculate_anchors		= v_recalculate_anchors		.getValue();
	info.anchor_clusters			= v_anchor_clusters			.getValue();
	info.class_imbalance			= v_class_imbalance			.getValue();
	info.restart_training			= v_restart_training		.getValue();
	info.delete_temp_weights		= v_delete_temp_weights		.getValue();
	info.save_weights				= v_save_weights			.getValue();
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
//			v_tile_images	= true;
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

	if (not v_do_not_resize_images.getValue())
	{
		remove_small_annotations_toggle->setEnabled(true);

		annotation_area_size_slider->setEnabled(v_remove_small_annotations.getValue());
	}
	else
	{
		v_remove_small_annotations = false;
		remove_small_annotations_toggle->setEnabled(false);
		annotation_area_size_slider->setEnabled(false);
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

	// go through all of the necessary property buttons and highlight any which are set opposite to what we expect
	for (const auto & entry : highlight_conditions_and_messages)
	{
		bool must_be_highlited = false;
		const bool has_colour = entry.component->isColourSpecified(PropertyComponent::ColourIds::backgroundColourId);

		// see if this is a boolean toggle box
		BooleanPropertyComponent * button = dynamic_cast<BooleanPropertyComponent *>(entry.component);
		if (button)
		{
			const bool state = button->getState();

			if (state == entry.highlight_if_enabled)
			{
				must_be_highlited = true;
			}
		}

		// next we check the slider controls
		SliderPropertyComponent * slider = dynamic_cast<SliderPropertyComponent *>(entry.component);
		if (slider)
		{
			const int value = slider->getValue();

			if ((entry.highlight_if_less_than > -1 and value < entry.highlight_if_less_than) or
				(entry.highlight_if_more_than > -1 and value > entry.highlight_if_more_than))
			{
				must_be_highlited = true;
			}
		}

		// if a control is disabled, then get rid of any highlighting
		if (not entry.component->isEnabled())
		{
			must_be_highlited = false;
		}

		if (must_be_highlited and not has_colour)
		{
			entry.component->setColour(PropertyComponent::ColourIds::backgroundColourId, Colours::red.withAlpha(0.75f));
			repaint();

			AttributedString str;
			str.setText(entry.msg);
			str.setColour(Colours::black);
			str.setWordWrap(AttributedString::WordWrap::byWord);

			bubble.showAt(entry.component, str, 10000, true, false);
		}
		else if (not must_be_highlited and has_colour)
		{
			entry.component->removeColour(PropertyComponent::ColourIds::backgroundColourId);
			repaint();
		}
	}

	return;
}


void dm::DarknetWnd::create_Darknet_configuration_file(ThreadWithProgressWindow & progress_window)
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
		{"scales"			, "0.2,0.1"												},
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

	std::set<size_t> steps =
	{
		static_cast<size_t>(std::round(0.8f * number_of_iterations)),
		static_cast<size_t>(std::round(0.9f * number_of_iterations))
	};

	if (v_restart_training.getValue().operator bool())
	{
		steps.insert(std::round(0.5f * number_of_iterations));
		steps.insert(std::round(0.6f * number_of_iterations));
		steps.insert(std::round(0.7f * number_of_iterations));
		m["scales"] = "0.5,0.4,0.3,0.2,0.1";
		m["burn_in"] = "0";
	}

	std::string str;
	for (const auto s : steps)
	{
		if (str.size())
		{
			str += ",";
		}
		str += std::to_string(s);
	}
	m["steps"] = str;

	cfg_handler.modify_all_sections("[net]", m);

	m.clear();
	if (recalculate_anchors)
	{
		progress_window.setStatusMessage(dm::getText("Recalculating anchors..."));
		progress_window.setProgress(0.0);

		/* Make many attempts at figuring out the best anchors.  In tests, I've seen the best anchors found as high
		 * as the 98th attempt!  Also keep track of the time in case it is taking too long and we want to abort.
		 */
		std::time_t now = std::time(nullptr);
		const std::time_t end_time = now + 60;
		const size_t max_attempts = 100;
		float best_avg_iou = 0.0f;

		for (size_t attempt = 0; attempt < max_attempts and std::time(nullptr) < end_time; attempt ++)
		{
			progress_window.setProgress(double(attempt) / double(max_attempts));

			std::string counters_per_class;
			std::string anchors;
			float avg_iou = 0.0f;

			calc_anchors(info.train_filename, anchor_clusters, info.image_width, info.image_height, number_of_classes, anchors, counters_per_class, avg_iou);
			if (avg_iou > best_avg_iou)
			{
				dm::Log("attempt #" + std::to_string(attempt) + ": avg IoU ........ " + std::to_string(avg_iou));
				dm::Log("attempt #" + std::to_string(attempt) + ": new anchors .... " + anchors);
				dm::Log("attempt #" + std::to_string(attempt) + ": new counters ... " + counters_per_class);

				best_avg_iou = avg_iou;
				m["anchors"] = anchors;

				if (class_imbalance)
				{
					m["counters_per_class"] = counters_per_class;
				}
			}
		}

		/* In YOLOv3-tiny and YOLOv4-tiny, there is a typo in the masks.  It
		 * should be 0,1,2 but instead appears as 1,2,3.  Fix this when the
		 * user has chosen to re-calculate the anchors.
		 *
		 * https://github.com/AlexeyAB/darknet/issues/7856#issuecomment-874147909
		 */
		for (auto section_idx : cfg_handler.find_section("yolo"))
		{
			const auto idx = cfg_handler.find_key_in_section(section_idx, "mask");

			if (idx != std::string::npos)
			{
				std::string & line = cfg_handler.cfg.at(idx);
				if (line == "mask = 1,2,3")
				{
					Log("fixing YOLO masks at index " + std::to_string(idx) + ": " + line);
					line = "mask = 0,1,2";
				}
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
		size_t & number_of_files_train			,
		size_t & number_of_files_valid			,
		size_t & number_of_annotated_images		,
		size_t & number_of_skipped_files		,
		size_t & number_of_marks				,
		size_t & number_of_empty_images			,
		size_t & number_of_dropped_empty_images	,
		size_t & number_of_resized_images		,
		size_t & number_of_images_not_resized	,
		size_t & number_of_tiles_created		,
		size_t & number_of_zooms_created		,
		size_t & number_of_dropped_annotations	)
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

	number_of_files_train			= 0;
	number_of_files_valid			= 0;
	number_of_annotated_images		= 0;
	number_of_skipped_files			= 0;
	number_of_marks					= 0;
	number_of_empty_images			= 0;
	number_of_dropped_empty_images	= 0;
	number_of_resized_images		= 0;
	number_of_images_not_resized	= 0;
	number_of_tiles_created			= 0;
	number_of_zooms_created			= 0;
	number_of_dropped_annotations	= 0;

	File dir = File(info.project_dir).getChildFile("darkmark_image_cache");
	if (dir.exists())
	{
		for (const auto & name : {"resize", "tiles", "zoom"})
		{
			File subdir = dir.getChildFile(name);
			subdir.deleteRecursively();
		}
	}

	// these vectors will have the full path of the images we need to use (or which have been skipped)
	VStr negative_samples;
	VStr annotated_images;
	VStr skipped_images;
	VStr all_output_images;
	find_all_annotated_images(progress_window, annotated_images, skipped_images, number_of_marks, number_of_empty_images);
	number_of_annotated_images = annotated_images.size();
	number_of_skipped_files = skipped_images.size();

	Log("total number of skipped input images ..... " + std::to_string(number_of_skipped_files		));
	Log("original number of annotated images ...... " + std::to_string(number_of_annotated_images	));
	Log("original number of marks ................. " + std::to_string(number_of_marks				));
	Log("original number of empty images .......... " + std::to_string(number_of_empty_images		));

	if (info.do_not_resize_images)
	{
		Log("not resizing any images");
		all_output_images = annotated_images;
	}
	else
	{
		// reset these counters and let the resize/tile/zoom+crop functions set these values
		number_of_marks = 0;
		number_of_empty_images = 0;
	}

	if (info.resize_images)
	{
		Log("resizing all images");
		resize_images(progress_window, annotated_images, all_output_images, number_of_resized_images, number_of_images_not_resized, number_of_marks, number_of_empty_images);
		Log("number of images resized ................. " + std::to_string(number_of_resized_images		));
		Log("number of images not resized ............. " + std::to_string(number_of_images_not_resized	));
	}
	if (info.tile_images)
	{
		Log("tiling all images");
		tile_images(progress_window, annotated_images, all_output_images, number_of_marks, number_of_tiles_created, number_of_empty_images);
		Log("number of tiles created .................. " + std::to_string(number_of_tiles_created));
	}
	if (info.zoom_images)
	{
		Log("crop+zoom all images");
		random_zoom_images(progress_window, annotated_images, all_output_images, number_of_marks, number_of_zooms_created, number_of_empty_images);
		Log("number of crop+zoom images created ....... " + std::to_string(number_of_zooms_created));
	}

	std::shuffle(all_output_images.begin(), all_output_images.end(), get_random_engine());

	if (info.limit_negative_samples)
	{
		// see if we need to limit the negative samples (especially useful when using tiling with large images)
		negative_samples.clear();
		annotated_images.clear();
		double work_done = 0.0;
		double work_to_do = all_output_images.size() + 1.0;
		progress_window.setProgress(0.0);
		progress_window.setStatusMessage(dm::getText("Limit negative samples..."));
		for (size_t idx = 0; idx < all_output_images.size(); idx ++)
		{
			work_done ++;
			progress_window.setProgress(work_done / work_to_do);

			const auto & fn = all_output_images[idx];
			if (File(fn).withFileExtension(".txt").getSize() == 0)
			{
				negative_samples.push_back(fn);
			}
			else
			{
				annotated_images.push_back(fn);
			}
		}

		Log("negative samples: " + std::to_string(negative_samples.size()));
		Log("annotated images: " + std::to_string(annotated_images.size()));

		if (negative_samples.size() > 1.2 * annotated_images.size())
		{
			number_of_dropped_empty_images = negative_samples.size() - annotated_images.size();
			Log("number of dropped negative samples ....... " + std::to_string(number_of_dropped_empty_images));
			negative_samples.resize(annotated_images.size());

			number_of_empty_images = negative_samples.size();
			all_output_images.swap(negative_samples);
			all_output_images.insert(all_output_images.end(), annotated_images.begin(), annotated_images.end());
			std::shuffle(all_output_images.begin(), all_output_images.end(), get_random_engine());
		}
	}

	if (info.do_not_resize_images == false and info.remove_small_annotations)
	{
		// go through all of the annotations and see if anything needs to be dropped

		Log("drop small annotations");
		drop_small_annotations(progress_window, all_output_images, number_of_dropped_annotations);
	}

	// now that we know the exact set of images (including resized and tiled images)
	// we can create the training and validation .txt files

	double work_done = 0.0;
	double work_to_do = all_output_images.size() + 1.0;
	progress_window.setProgress(0.0);
	progress_window.setStatusMessage(dm::getText("Writing training and validation files..."));
	Log("total number of output images ............ " + std::to_string(all_output_images.size()));

	const bool use_all_images = info.train_with_all_images;
	number_of_files_train = std::round(info.training_images_percentage * all_output_images.size());
	number_of_files_valid = all_output_images.size() - number_of_files_train;

	if (use_all_images)
	{
		number_of_files_train = all_output_images.size();
		number_of_files_valid = all_output_images.size();
	}

	const size_t maximum_number_of_validation_images = 10 * content.names.size();
	if (info.limit_validation_images)
	{
		if (number_of_files_valid > maximum_number_of_validation_images)
		{
			number_of_files_valid = maximum_number_of_validation_images;

			if (not use_all_images)
			{
				number_of_files_train = all_output_images.size() - number_of_files_valid;
			}
		}
	}

	number_of_annotated_images = all_output_images.size() - number_of_empty_images;
	Log("total number of annotated images ............ " + std::to_string(number_of_annotated_images	));
	Log("total number of marks ....................... " + std::to_string(number_of_marks - number_of_dropped_annotations));
	Log("total number of dropped marks (too small) ... " + std::to_string(number_of_dropped_annotations));
	Log("total number of empty images ................ " + std::to_string(number_of_empty_images		));
	Log("total number of training images ............. " + std::to_string(number_of_files_train) + " (" + info.train_filename + ")");
	Log("total number of validation images ........... " + std::to_string(number_of_files_valid) + " (" + info.valid_filename + ")");
	Log("cap validation images ....................... " + std::string(info.limit_validation_images ? "true" : "false"));

	std::ofstream fs_train(info.train_filename);
	std::ofstream fs_valid(info.valid_filename);

	size_t current_number_of_validation_images = 0;
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
			if (info.limit_validation_images and current_number_of_validation_images >= maximum_number_of_validation_images)
			{
				continue;
			}
			fs_valid << all_output_images[idx] << std::endl;
			current_number_of_validation_images ++;
		}
	}

	Log("training and validation files have been saved to disk");

	return;
}


void dm::DarknetWnd::create_Darknet_shell_scripts()
{
	if (simplified_interface)
	{
		return;
	}

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
		std::string cmd =
				cfg().get_str("darknet_executable") +
				" detector train " +
				(v_verbose_output		.getValue() ? "-verbose "	: "") +
				(v_keep_augmented_images.getValue() ? "-show_imgs "	: "") +
				v_extra_flags.toString().toStdString() + " " +
				info.data_filename + " " + info.cfg_filename;

		if (info.save_weights > 0)
		{
			cmd += " --save-weights " + std::to_string(info.save_weights);
		}
		if (info.restart_training)
		{
			cmd += " " + cfg().get_str(content.cfg_prefix + "weights");
			cmd += " -clear";
		}

		std::stringstream ss;
		ss	<< header
			<< ""												<< std::endl
			<< "rm -f output.log"								<< std::endl
			<< "#rm -f chart.png"								<< std::endl
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
			<< "	rsync --update --human-readable --info=progress2,stats2 --times --no-compress gpurig:" << info.project_dir << "/\\* ." << std::endl
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
			<< "	rsync --update --human-readable --recursive --no-inc-recursive --info=progress2,stats2 --times --no-compress . gpurig:" << info.project_dir << std::endl
			<< "fi"																								<< std::endl
			<< ""																								<< std::endl;
		const std::string data = ss.str();
		File f = File(info.project_dir).getChildFile("send_files_to_gpu_rig.sh");
		f.replaceWithData(data.c_str(), data.size());
		f.setExecutePermission(true);
	}

	return;
}
