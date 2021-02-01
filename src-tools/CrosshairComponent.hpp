// DarkMark (C) 2019-2021 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	/** Component which uses a cached image to display the contents, and which superimposes a large crosshair to show
	 * where the mouse is located.  Derived classes must implement the pure virtual @ref rebuild_cache_image() which is
	 * called only when @ref need_to_rebuild_cache_image has been set to @p true.
	 */
	class CrosshairComponent : public Component, public Timer
	{
		public:

			/// Constructor.
			CrosshairComponent(DMContent & c);

			/// Destructor.
			virtual ~CrosshairComponent();

			virtual MouseCursor getMouseCursor() override;

			/// Sets the @ref need_to_rebuild_cache_image flag, which eventually results in a call to @ref rebuild_cache_image().
			virtual void resized() override;

			/** Paint the @ref cached_image onto the screen, then calls @ref paint_crosshairs() to draw the crosshairs
			 * over the image.  Derived classes shouldn't have to call this if the content of the window can be fully
			 * determined at the time @ref rebuild_cache_image() was called.  If you do implement your own @p paint(),
			 * then you'll also have to manually call @ref paint_crosshairs().
			 */
			virtual void paint(Graphics & g) override;

			/// Draws the crosshairs at the last known mouse location.  @see @ref paint()  @see @ref mouse_current_loc
			virtual void paint_crosshairs(Graphics & g);

			virtual void mouseMove(const MouseEvent & event) override;
			virtual void mouseEnter(const MouseEvent & event) override;
			virtual void mouseExit(const MouseEvent & event) override;
			virtual void mouseDown(const MouseEvent & event) override;
			virtual void mouseUp(const MouseEvent & event) override;
			virtual void mouseDrag(const MouseEvent & event) override;
			virtual void mouseWheelMove(const MouseEvent & event, const MouseWheelDetails & wheel) override;
			virtual void mouseDragFinished(juce::Rectangle<int> drag_rect) { return; }

			virtual void timerCallback() override;

			/** This is called to generate the content that will be drawn on the screen.  Derived classes must implement
			 * this method.  The resulting image must be stored in @ref cached_image and be the exact same width and
			 * height as the component.  Prior to returning, set the flag @ref need_to_rebuild_cache_image to @p false.
			 */
			virtual void rebuild_cache_image() = 0;

			/// Link to the parent which manages the content, including all the marks.
			DMContent & content;

			bool mouse_drag_is_enabled;

			const juce::Point<int> invalid_point;
			const juce::Rectangle<int> invalid_rectangle;

			juce::Point<int>		mouse_current_loc;
			juce::Point<int>		mouse_previous_loc;
			juce::Point<int>		mouse_down_loc;
			juce::Rectangle<int>	mouse_drag_rectangle;

			juce::Image cached_image;
			bool need_to_rebuild_cache_image;
	};
}
