/* DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

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

	if (dmapp().darkhelp)
	{
		v_darkhelp_threshold							= std::round(100.0f * dmapp().darkhelp->threshold);
		v_darkhelp_hierchy_threshold					= std::round(100.0f * dmapp().darkhelp->hierarchy_threshold);
		v_darkhelp_non_maximal_suppression_threshold	= std::round(100.0f * dmapp().darkhelp->non_maximal_suppression_threshold);
	}

	v_scrollfield_width			= content.scrollfield_width;
	v_scrollfield_marker_size	= content.scrollfield.triangle_size;
	v_show_mouse_pointer		= content.show_mouse_pointer;

	v_darkhelp_threshold						.addListener(this);
	v_darkhelp_hierchy_threshold				.addListener(this);
	v_darkhelp_non_maximal_suppression_threshold.addListener(this);
	v_scrollfield_width							.addListener(this);
	v_scrollfield_marker_size					.addListener(this);
	v_show_mouse_pointer						.addListener(this);

	Array<PropertyComponent*> properties;
//	TextPropertyComponent		* t = nullptr;
	BooleanPropertyComponent	* b = nullptr;
	SliderPropertyComponent		* s = nullptr;
//	ButtonPropertyComponent		* b = nullptr;

	s = new SliderPropertyComponent(v_darkhelp_threshold, "detection threshold", 0, 100, 1);
	s->setTooltip("Detection threshold is used to determine whether or not there is an object in the predicted bounding box.");
	properties.add(s);

	s = new SliderPropertyComponent(v_darkhelp_hierchy_threshold, "hierarchy threshold", 0, 100, 1);
	s->setTooltip("The hierarchical threshold is used to decide whether following the tree to a more specific class is the right action to take. When this threshold is 0, the tree will basically follow the highest probability branch all the way to a leaf node.");
	properties.add(s);

	s = new SliderPropertyComponent(v_darkhelp_non_maximal_suppression_threshold, "nms threshold", 0, 100, 1);
	s->setTooltip("Non-Maximal Suppression (NMS) suppresses overlapping bounding boxes and only retains the bounding box that has the maximum probability of object detection associated with it. It examines all bounding boxes and removes the least confident of the boxes that overlap with each other.");
	properties.add(s);

	pp.addSection("darknet", properties);
	properties.clear();

//	b = new CrosshairColourPicker("crosshair colour");
//	b->setEnabled(false);
//	properties.add(b);

	s = new SliderPropertyComponent(v_scrollfield_width, "scrollfield width", 0.0, 200.0, 10.0);
	properties.add(s);

	s = new SliderPropertyComponent(v_scrollfield_marker_size, "scrollfield marker size", 0.0, 9.0, 1.0);
	s->setTooltip("Markers will only be shown when the image sort order is set to 'alphabetical'.");
	properties.add(s);

	b = new BooleanPropertyComponent(v_show_mouse_pointer, "show mouse pointer", "show mouse pointer");
	b->setTooltip("Determines if the mouse pointer is shown in addition to the crosshairs.");
	properties.add(b);

	pp.addSection("drawing", properties);
	properties.clear();

	auto r = dmapp().wnd->getBounds();
	r = r.withSizeKeepingCentre(400, 300);
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

//	cfg().setValue("crosshair_colour"			, CrosshairComponent::crosshair_colour			.toString());
	cfg().setValue("darknet_threshold"			, v_darkhelp_threshold							.getValue());
	cfg().setValue("darknet_hierarchy_threshold", v_darkhelp_hierchy_threshold					.getValue());
	cfg().setValue("darknet_nms_threshold"		, v_darkhelp_non_maximal_suppression_threshold	.getValue());
	cfg().setValue("scrollfield_width"			, v_scrollfield_width							.getValue());
	cfg().setValue("scrollfield_marker_size"	, v_scrollfield_marker_size						.getValue());
	cfg().setValue("show_mouse_pointer"			, v_show_mouse_pointer							.getValue());

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
	if (dmapp().darkhelp)
	{
		dmapp().darkhelp->hierarchy_threshold				= static_cast<float>(v_darkhelp_hierchy_threshold					.getValue()) / 100.0f;
		dmapp().darkhelp->non_maximal_suppression_threshold	= static_cast<float>(v_darkhelp_non_maximal_suppression_threshold	.getValue()) / 100.0f;
		dmapp().darkhelp->threshold							= static_cast<float>(v_darkhelp_threshold							.getValue()) / 100.0f;
	}
	content.scrollfield_width			= v_scrollfield_width		.getValue();
	content.scrollfield.triangle_size	= v_scrollfield_marker_size	.getValue();
	content.show_mouse_pointer			= v_show_mouse_pointer		.getValue();

	startTimer(250); // request a callback -- in milliseconds -- at which point in time we'll fully reload the current image

	return;
}


void dm::SettingsWnd::timerCallback()
{
	// if we get called, then the settings are no longer changing, so reload the current image

	stopTimer();

	content.scrollfield.rebuild_cache_image();
	content.load_image(content.image_filename_index);
	content.resized();

	return;
}
