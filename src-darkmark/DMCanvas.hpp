// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	/** This is the actual class that draws the current image and all of the annotations/marks.  Most of the work is performed
	 * in @ref rebuild_cache_image().  Also of importance is the mouse event handling to ensure that marks are created and
	 * stretched correctly.
	 *
	 * This class is one of the children of @ref DMContent.
	 */
	class DMCanvas : public CrosshairComponent
	{
		public:

			DMCanvas(DMContent & c);

			virtual ~DMCanvas();

			virtual void rebuild_cache_image();

			virtual void mouseDown(const MouseEvent &event) override;
			void mouseDrag(const juce::MouseEvent &event) override;
			void mouseUp(const juce::MouseEvent &event) override;

			virtual void mouseDoubleClick(const MouseEvent &event) override;
			virtual void mouseDragFinished(juce::Rectangle<int> drag_rect, const MouseEvent &event) override;
			// A helper function to check if a point is in the central region of a rectangle.
			bool isInCenterRegion(const juce::Rectangle<int> &rect, int x, int y, double marginFactor = 0.25) const;

			/// Link to the parent which manages the content, including all the marks.
			DMContent &content;

			bool panning_active = false;   // Are we currently panning?
			juce::Point<int> pan_start;	   // Mouse position where panning began
			juce::Point<int> saved_offset; // Original zoom_image_offset at the start of panning


		private:
			// For the rubber-band mass-delete area:
			juce::Point<int> dragStart;
			juce::Point<int> dragCurrent;
			cv::Rect selectionRect;

			// For repositioning
			cv::Rect2d repositionOriginalNormRect; // Original normalized bounding rectangle of the mark

			bool resizingActive = false;		   ///< true when we are corner‐dragging to resize
			ECorner resizingCorner = ECorner::kTL; ///< which corner is being dragged
			cv::Rect2d resizingOriginalNormRect;   ///< the mark’s original normalized rect before resizing
			cv::Point resizeDragStartScaled;	   ///< the mouseDown location in scaled coords, if needed

			bool repositionActive = false;
			juce::Point<int> repositionDragStart; // starting mouse position (in canvas pixels)
			cv::Point2d markDragStartNormalized;  // starting top-left of the mark (normalized)
	};
}
