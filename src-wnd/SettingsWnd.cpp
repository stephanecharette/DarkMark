// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"


#if 0
class CrosshairColourPicker : public ButtonPropertyComponent, public ChangeListener
{
	public:

		CrosshairColourPicker(const String & propertyName) : ButtonPropertyComponent(propertyName, true) { return; }
		virtual String getButtonText()const override { return Colours::white.toDisplayString(false); }
		virtual void buttonClicked() override
		{
			ColourSelector * cs = new ColourSelector(
				ColourSelector::showColourAtTop |
				ColourSelector::editableColour	|
				ColourSelector::showColourspace	);

			cs->setCurrentColour(Colours::white);
			cs->setName("Crosshair Colour");
			cs->setSize(200, 200);
			cs->addChangeListener(this);

			Component * parent = dm::dmapp().settings_wnd.get();
			auto r = parent->getLocalArea(this, getBounds());
			CallOutBox::launchAsynchronously(cs, r, parent);

			return;
		}

		virtual void changeListenerCallback(ChangeBroadcaster * source) override
		{
			ColourSelector * cs = dynamic_cast<ColourSelector*>(source);
			if (cs)
			{
				const auto colour = cs->getCurrentColour();
//				dm::CrosshairComponent::crosshair_colour = colour;
				refresh();
			}
			return;
		}
};
#endif


dm::SettingsWnd::SettingsWnd(dm::DMContent & c) :
	DocumentWindow("DarkMark v" DARKMARK_VERSION " - Settings", Colours::darkgrey, TitleBarButtons::closeButton),
	content(c),
	ok_button("OK")
{
	setContentNonOwned		(&canvas, true	);
	setUsingNativeTitleBar	(true			);
	setResizable			(true, false	);
	setDropShadowEnabled	(true			);
	setAlwaysOnTop			(true			);

	canvas.addAndMakeVisible(pp);
	canvas.addAndMakeVisible(ok_button);

	ok_button.addListener(this);

	setIcon(DarkMarkLogo());
	ComponentPeer *peer = getPeer();
	if (peer)
	{
		peer->setIcon(DarkMarkLogo());
	}

	v_darknet_executable	= String(cfg().get_str("darknet_executable"));
	v_darknet_templates		= String(cfg().get_str("darknet_templates"));

	if (dmapp().darkhelp_nn)
	{
		v_darkhelp_threshold							= std::round(100.0f * dmapp().darkhelp_nn->config.threshold);
//		v_darkhelp_hierchy_threshold					= std::round(100.0f * dmapp().darkhelp_nn->config.hierarchy_threshold);
		v_darkhelp_non_maximal_suppression_threshold	= std::round(100.0f * dmapp().darkhelp_nn->config.non_maximal_suppression_threshold);
		v_image_tiling									= dmapp().darkhelp_nn->config.enable_tiles;
	}
	else
	{
		// DarkHelp didn't load (no neural network?) so use whatever is in configuration instead
		v_darkhelp_threshold							= cfg().get_int("darknet_threshold");
//		v_darkhelp_hierchy_threshold					= cfg().get_int("darknet_hierarchy_threshold");
		v_darkhelp_non_maximal_suppression_threshold	= cfg().get_int("darknet_nms_threshold");
		v_image_tiling									= cfg().get_bool("darknet_image_tiling");
	}

	v_scrollfield_width			= content.scrollfield_width;
	v_scrollfield_marker_size	= content.scrollfield.triangle_size;
	v_show_mouse_pointer		= content.show_mouse_pointer;
	v_corner_size				= content.corner_size;
	v_review_resize_thumbnails	= cfg().get_bool("review_resize_thumbnails");
	v_review_table_row_height	= cfg().get_int("review_table_row_height");

	v_black_and_white_mode_enabled			= content.black_and_white_mode_enabled;
	v_black_and_white_threshold_blocksize	= content.black_and_white_threshold_blocksize;
	v_black_and_white_threshold_constant	= content.black_and_white_threshold_constant;
	v_snapping_enabled						= content.snapping_enabled;
	v_snap_horizontal_tolerance				= content.snap_horizontal_tolerance;
	v_snap_vertical_tolerance				= content.snap_vertical_tolerance;
	v_dilate_erode_mode						= content.dilate_erode_mode;
	v_dilate_kernel_size					= content.dilate_kernel_size;
	v_dilate_iterations						= content.dilate_iterations;
	v_erode_kernel_size						= content.erode_kernel_size;
	v_erode_iterations						= content.erode_iterations;
	v_heatmaps_enabled						= content.heatmap_enabled;
	v_heatmap_class_idx						= content.heatmap_class_idx;
	v_heatmap_alpha_blend					= content.heatmap_alpha_blend;
	v_heatmap_threshold						= content.heatmap_threshold;
	v_heatmap_visualize						= content.heatmap_visualize;

	v_darknet_executable						.addListener(this);
	v_darknet_templates							.addListener(this);
	v_darkhelp_threshold						.addListener(this);
//	v_darkhelp_hierchy_threshold				.addListener(this);
	v_darkhelp_non_maximal_suppression_threshold.addListener(this);
	v_scrollfield_width							.addListener(this);
	v_scrollfield_marker_size					.addListener(this);
	v_show_mouse_pointer						.addListener(this);
	v_image_tiling								.addListener(this);
	v_corner_size								.addListener(this);
	v_review_resize_thumbnails					.addListener(this);
	v_review_table_row_height					.addListener(this);
	v_black_and_white_mode_enabled				.addListener(this);
	v_black_and_white_threshold_blocksize		.addListener(this);
	v_black_and_white_threshold_constant		.addListener(this);
	v_snapping_enabled							.addListener(this);
	v_snap_horizontal_tolerance					.addListener(this);
	v_snap_vertical_tolerance					.addListener(this);
	v_dilate_erode_mode							.addListener(this);
	v_dilate_kernel_size						.addListener(this);
	v_dilate_iterations							.addListener(this);
	v_erode_kernel_size							.addListener(this);
	v_erode_iterations							.addListener(this);
	v_heatmaps_enabled							.addListener(this);
	v_heatmap_class_idx							.addListener(this);
	v_heatmap_alpha_blend						.addListener(this);
	v_heatmap_threshold							.addListener(this);
	v_heatmap_visualize							.addListener(this);

	Array<PropertyComponent*> properties;
	TextPropertyComponent		* t = nullptr;
	BooleanPropertyComponent	* b = nullptr;
	SliderPropertyComponent		* s = nullptr;
//	ButtonPropertyComponent		* b = nullptr;

	t = new TextPropertyComponent(v_darknet_executable, "darknet executable", 1000, false, true);
	t->setTooltip("The location of the darknet executable used to train a network.");
	properties.add(t);

	t = new TextPropertyComponent(v_darknet_templates, "configuration templates", 1000, false, true);
	t->setTooltip("The location of the .cfg files to use as configuration templates when setting up a project to train a network.");
	properties.add(t);

	s = new SliderPropertyComponent(v_darkhelp_threshold, "detection threshold", 0, 100, 1);
	s->setTooltip("Detection threshold is used to determine whether or not there is an object in the predicted bounding box. Default value is 50%.");
	properties.add(s);

//	s = new SliderPropertyComponent(v_darkhelp_hierchy_threshold, "hierarchy threshold", 0, 100, 1);
//	s->setTooltip("The hierarchical threshold is used to decide whether following the tree to a more specific class is the right action to take. When this threshold is 0, the tree will basically follow the highest probability branch all the way to a leaf node. Default value is 50%.");
//	properties.add(s);

	s = new SliderPropertyComponent(v_darkhelp_non_maximal_suppression_threshold, "nms threshold", 0, 100, 1);
	s->setTooltip("Non-Maximal Suppression (NMS) suppresses overlapping bounding boxes and only retains the bounding box that has the maximum probability of object detection associated with it. It examines all bounding boxes and removes the least confident of the boxes that overlap with each other. Default value is 45%.");
	properties.add(s);

	b = new BooleanPropertyComponent(v_image_tiling, "enable image tiling", "enable image tiling");
	b->setTooltip("Determines if images will be tiled when sent to darknet for processing. The default value is \"off\".");
	properties.add(b);

	pp.addSection("darknet", properties, true);
	properties.clear();

//	b = new CrosshairColourPicker("crosshair colour");
//	b->setEnabled(false);
//	properties.add(b);

	s = new SliderPropertyComponent(v_scrollfield_width, "scrollfield width", 0.0, 200.0, 10.0);
	s->setTooltip("The size of the scroll window on the right side of the annotation window. The default value is 100.");
	properties.add(s);

	s = new SliderPropertyComponent(v_scrollfield_marker_size, "scrollfield marker size", 0.0, 9.0, 1.0);
	s->setTooltip("Markers will only be shown when the image sort order is set to 'alphabetical'. The default value is 7.");
	properties.add(s);

	b = new BooleanPropertyComponent(v_show_mouse_pointer, "show mouse pointer", "show mouse pointer");
	b->setTooltip("Determines if the mouse pointer is shown in addition to the crosshairs. The default value is \"off\".");
	properties.add(b);

	s = new SliderPropertyComponent(v_corner_size, "corner size", 0.0, 30.0, 1.0);
	s->setTooltip("In pixels, the size of the corners used to modify annotations. The default value is 10.");
	properties.add(s);

	b = new BooleanPropertyComponent(v_review_resize_thumbnails, "resize review thumbnails", "resize review thumbnails");
	b->setTooltip("Determines if small review thumbnails will be resized to fill the height of the row. The default value is \"on\".");
	properties.add(b);

	s = new SliderPropertyComponent(v_review_table_row_height, "height of review thumbnails", 25.0, 250.0, 1.0);
	s->setTooltip("In pixels, the height of the rows and the height of each annotation thumbnail. The default value is 75.");
	properties.add(s);

	pp.addSection("drawing", properties, true);
	properties.clear();

	b = new BooleanPropertyComponent(v_black_and_white_mode_enabled, "black-and-white mode", "black-and-white mode");
	b->setTooltip("Display black-and-white threshold images.");
	properties.add(b);

	s = new SliderPropertyComponent(v_black_and_white_threshold_blocksize, "threshold block size", 3.0, 99.0, 2.0);
	s->setTooltip("Size of a pixel neighborhood that is used to calculate a threshold value. The default value is 15.");
	properties.add(s);

	s = new SliderPropertyComponent(v_black_and_white_threshold_constant, "threshold constant", -100.0, 100.0, 0.01);
	s->setTooltip("Constant subtracted from the mean or weighted mean. The default value is 15.");
	properties.add(s);

	auto choice = new ChoicePropertyComponent(v_dilate_erode_mode, "dilate/erode mode", {"none", "dilate only", "dilate then erode", "erode then dilate", "erode only"}, {0, 1, 2, 3, 4});
	choice->setTooltip("Use dilation and/or erosion to clean up noisy images. This is only used when black-and-white mode has also been enabled. Dilation grows light areas, while erosion grows dark areas.");
	properties.add(choice);

	s = new SliderPropertyComponent(v_dilate_kernel_size, "dilate kernel size", 2.0, 9, 1.0);
	s->setTooltip("Kernel size to use if dilation has been enabled. The default value is 2.");
	properties.add(s);

	s = new SliderPropertyComponent(v_dilate_iterations, "dilate iterations", 1.0, 9, 1.0);
	s->setTooltip("Number of iterations to use if dilation has been enabled. The default value is 1.");
	properties.add(s);

	s = new SliderPropertyComponent(v_erode_kernel_size, "erode kernel size", 2.0, 9, 1.0);
	s->setTooltip("Kernel size to use if erosion has been enabled. The default value is 2.");
	properties.add(s);

	s = new SliderPropertyComponent(v_erode_iterations, "erode iterations", 1.0, 9, 1.0);
	s->setTooltip("Number of iterations to use if erosion has been enabled. The default value is 1.");
	properties.add(s);

	pp.addSection("black-and-white mode", properties, false);
	properties.clear();

	b = new BooleanPropertyComponent(v_snapping_enabled, "auto-snapping enabled", "auto-snapping enabled");
	b->setTooltip("New annotations will be snapped. This is mostly intended for objects on a light background, such as text on paper.");
	properties.add(b);

	s = new SliderPropertyComponent(v_snap_horizontal_tolerance, "snap horizontal tolerance", 1, 99, 1);
	s->setTooltip("The number of pixels DarkMark will search horizontally when attempting to snap annotations. This is used by the \"snap\" feature when annotating images with text. This may need to be adjusted based on spacing between letters and spacing between words.");
	properties.add(s);

	s = new SliderPropertyComponent(v_snap_vertical_tolerance, "snap vertical tolerance", 1, 99, 1);
	s->setTooltip("The number of pixels DarkMark will search vertically when attempting to snap annotations. This is used by the \"snap\" feature when annotating images with text. This may need to be adjusted based on spacing between lines of text.");
	properties.add(s);

	pp.addSection("snapping", properties, false);
	properties.clear();

	b = new BooleanPropertyComponent(v_heatmaps_enabled, "heatmaps enabled", "heatmaps enabled");
	b->setTooltip("Display heatmaps. This is only available once a neural network has been trained and loaded.");
	properties.add(b);

	StringArray sa = {"combined"};
	Array<var> var = {-1};
	for (size_t idx = 0; idx < content.names.size() - 1; idx ++) // -1 to skip "empty image*
	{
		sa.add(content.names[idx]);
		var.add(static_cast<int>(idx));
	}
	choice = new ChoicePropertyComponent(v_heatmap_class_idx, "class", sa, var);
	choice->setTooltip("Select the heatmap to be used. It can be a specific class, or the combination of all classes.");
	properties.add(choice);

	s = new SliderPropertyComponent(v_heatmap_alpha_blend, "alpha blend", 0.05, 0.95, 0.05);
	s->setTooltip("Alpha blend to use when displaying heatmaps. This is only used when heatmaps are enabled. The default value is 0.5.");
	properties.add(s);

	s = new SliderPropertyComponent(v_heatmap_threshold, "threshold", 0.001, 1.000, 0.001);
	s->setTooltip("Threshold determines how many predictions are included in the heatmap. Default is 0.1.");
	properties.add(s);

	choice = new ChoicePropertyComponent(v_heatmap_visualize, "colourmap",
		{
			// special case
			"none",
			// OpenCV names start here (see cv::ColormapTypes)
			"autumn"		, "bone"		, "jet"			, "winter"		, "rainbow"		, "ocean"				, "summer"	,
			"spring"		, "cool"		, "hsv"			, "pink"		, "hot"			, "parula"				, "magma"	,
			"inferno"		, "plasma"		, "viridis"		, "cividis"		, "twilight"	, "twilight shifted"	, "turbo"	,
//			"deepgreen" note that deepgreen was only added in later versions of OpenCV
		},
		{
			// special case
			-1,
			// OpenCV types
			cv::ColormapTypes::COLORMAP_AUTUMN,
			cv::ColormapTypes::COLORMAP_BONE,
			cv::ColormapTypes::COLORMAP_JET,
			cv::ColormapTypes::COLORMAP_WINTER,
			cv::ColormapTypes::COLORMAP_RAINBOW,
			cv::ColormapTypes::COLORMAP_OCEAN,
			cv::ColormapTypes::COLORMAP_SUMMER,
			cv::ColormapTypes::COLORMAP_SPRING,
			cv::ColormapTypes::COLORMAP_COOL,
			cv::ColormapTypes::COLORMAP_HSV,
			cv::ColormapTypes::COLORMAP_PINK,
			cv::ColormapTypes::COLORMAP_HOT,
			cv::ColormapTypes::COLORMAP_PARULA,
			cv::ColormapTypes::COLORMAP_MAGMA,
			cv::ColormapTypes::COLORMAP_INFERNO,
			cv::ColormapTypes::COLORMAP_PLASMA,
			cv::ColormapTypes::COLORMAP_VIRIDIS,
			cv::ColormapTypes::COLORMAP_CIVIDIS,
			cv::ColormapTypes::COLORMAP_TWILIGHT,
			cv::ColormapTypes::COLORMAP_TWILIGHT_SHIFTED,
			cv::ColormapTypes::COLORMAP_TURBO,
//			cv::ColormapTypes::COLORMAP_DEEPGREEN
		});
	choice->setTooltip("Select which OpenCV visualization colourmap to use for the heatmap. Default is \"jet\".");
	properties.add(choice);

	pp.addSection("heatmap", properties, false);
	properties.clear();

	auto r = dmapp().wnd->getBounds();
	r = r.withSizeKeepingCentre(400, 450);
	setBounds(r);

	setVisible(true);

	return;
}


dm::SettingsWnd::~SettingsWnd()
{
	return;
}


void dm::SettingsWnd::closeButtonPressed()
{
	// close button

//	cfg().setValue("crosshair_colour"					, CrosshairComponent::crosshair_colour			.toString());
	cfg().setValue("darknet_executable"					, v_darknet_executable							.getValue());
	cfg().setValue("darknet_templates"					, v_darknet_templates							.getValue());
	cfg().setValue("darknet_threshold"					, v_darkhelp_threshold							.getValue());
//	cfg().setValue("darknet_hierarchy_threshold"		, v_darkhelp_hierchy_threshold					.getValue());
	cfg().setValue("darknet_nms_threshold"				, v_darkhelp_non_maximal_suppression_threshold	.getValue());
	cfg().setValue("scrollfield_width"					, v_scrollfield_width							.getValue());
	cfg().setValue("scrollfield_marker_size"			, v_scrollfield_marker_size						.getValue());
	cfg().setValue("show_mouse_pointer"					, v_show_mouse_pointer							.getValue());
	cfg().setValue("darknet_image_tiling"				, v_image_tiling								.getValue());
	cfg().setValue("corner_size"						, v_corner_size									.getValue());
	cfg().setValue("review_resize_thumbnails"			, v_review_resize_thumbnails					.getValue());
	cfg().setValue("review_table_row_height"			, v_review_table_row_height						.getValue());
	cfg().setValue("black_and_white_mode_enabled"		, v_black_and_white_mode_enabled				.getValue());
	cfg().setValue("black_and_white_threshold_blocksize", v_black_and_white_threshold_blocksize			.getValue());
	cfg().setValue("black_and_white_threshold_constant"	, v_black_and_white_threshold_constant			.getValue());
	cfg().setValue("snapping_enabled"					, v_snapping_enabled							.getValue());
	cfg().setValue("snap_horizontal_tolerance"			, v_snap_horizontal_tolerance					.getValue());
	cfg().setValue("snap_vertical_tolerance"			, v_snap_vertical_tolerance						.getValue());
	cfg().setValue("dilate_erode_mode"					, v_dilate_erode_mode							.getValue());
	cfg().setValue("dilate_kernel_size"					, v_dilate_kernel_size							.getValue());
	cfg().setValue("dilate_iterations"					, v_dilate_iterations							.getValue());
	cfg().setValue("erode_kernel_size"					, v_erode_kernel_size							.getValue());
	cfg().setValue("erode_iterations"					, v_erode_iterations							.getValue());
	cfg().setValue("heatmap_enabled"					, v_heatmaps_enabled							.getValue());
	cfg().setValue("heatmap_alpha_blend"				, v_heatmap_alpha_blend							.getValue());
	cfg().setValue("heatmap_threshold"					, v_heatmap_threshold							.getValue());
	cfg().setValue("heatmap_visualize"					, v_heatmap_visualize							.getValue());

	dmapp().settings_wnd.reset(nullptr);

	return;
}


void dm::SettingsWnd::userTriedToCloseWindow()
{
	// ALT+F4

	closeButtonPressed();

	return;
}


void dm::SettingsWnd::resized()
{
	// get the document window to resize the canvas, then we'll deal with the rest of the components
	DocumentWindow::resized();

	const int margin_size = 5;

	FlexBox button_row;
	button_row.flexDirection = FlexBox::Direction::row;
	button_row.justifyContent = FlexBox::JustifyContent::flexEnd;
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


void dm::SettingsWnd::buttonClicked(Button * button)
{
	closeButtonPressed();

	return;
}


void dm::SettingsWnd::valueChanged(Value & value)
{
	if (dmapp().darkhelp_nn)
	{
//		dmapp().darkhelp_nn->config.hierarchy_threshold					= static_cast<float>(v_darkhelp_hierchy_threshold					.getValue()) / 100.0f;
		dmapp().darkhelp_nn->config.non_maximal_suppression_threshold	= static_cast<float>(v_darkhelp_non_maximal_suppression_threshold	.getValue()) / 100.0f;
		dmapp().darkhelp_nn->config.threshold							= static_cast<float>(v_darkhelp_threshold							.getValue()) / 100.0f;
		dmapp().darkhelp_nn->config.enable_tiles						= static_cast<bool>(v_image_tiling.getValue());
	}
	content.scrollfield_width					= v_scrollfield_width					.getValue();
	content.scrollfield.triangle_size			= v_scrollfield_marker_size				.getValue();
	content.show_mouse_pointer					= v_show_mouse_pointer					.getValue();
	content.corner_size							= v_corner_size							.getValue();
	content.black_and_white_mode_enabled		= v_black_and_white_mode_enabled		.getValue();
	content.black_and_white_threshold_blocksize	= v_black_and_white_threshold_blocksize	.getValue();
	content.black_and_white_threshold_constant	= v_black_and_white_threshold_constant	.getValue();
	content.snapping_enabled					= v_snapping_enabled					.getValue();
	content.snap_horizontal_tolerance			= v_snap_horizontal_tolerance			.getValue();
	content.snap_vertical_tolerance				= v_snap_vertical_tolerance				.getValue();
	content.dilate_erode_mode					= v_dilate_erode_mode					.getValue();
	content.dilate_kernel_size					= v_dilate_kernel_size					.getValue();
	content.dilate_iterations					= v_dilate_iterations					.getValue();
	content.erode_kernel_size					= v_erode_kernel_size					.getValue();
	content.erode_iterations					= v_erode_iterations					.getValue();
	content.heatmap_enabled						= v_heatmaps_enabled					.getValue();
	content.heatmap_class_idx					= v_heatmap_class_idx					.getValue();
	content.heatmap_alpha_blend					= v_heatmap_alpha_blend					.getValue();
	content.heatmap_threshold					= v_heatmap_threshold					.getValue();
	content.heatmap_visualize					= v_heatmap_visualize					.getValue();

	startTimer(250); // request a callback -- in milliseconds -- at which point in time we'll fully reload the current image

	return;
}


void dm::SettingsWnd::timerCallback()
{
	// if we get called, then the settings are no longer changing, so reload the current image

	stopTimer();

	content.load_image(content.image_filename_index);
	content.resized();
	content.scrollfield.rebuild_cache_image();

	return;
}
