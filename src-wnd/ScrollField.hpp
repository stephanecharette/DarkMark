// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class ScrollField final : public CrosshairComponent, public Thread
	{
		public:

			ScrollField(DMContent & c);

			virtual ~ScrollField();

			virtual void rebuild_entire_field_on_thread();

			virtual void run() override;

			virtual void update_index(const size_t idx);

			virtual void mouseUp(const MouseEvent & event) override;
			virtual void mouseDown(const MouseEvent & event) override;
			virtual void mouseDrag(const MouseEvent & event) override;
			virtual void mouseMove(const MouseEvent & event) override;

			virtual void jump_to_location(const MouseEvent & event, const bool full_load = false);

			virtual void rebuild_cache_image() override;
			virtual void draw_triangles_at_image_sets();
			virtual void draw_marker_at_current_image();

			/// Link to the parent which manages the content.
			DMContent & content;

			/// The width of a single class "line" in the field.
			double line_width;

			/// The full-size scrollfield with all the markup from the .json files.
			cv::Mat field;

			/// The @ref field image resized to fit exactly within the window.
			cv::Mat resized_image;

			/// Track at which index the image sets can be found so we can draw our little triangles.
			MIdxStr map_idx_imagesets;

			/// The size -- in pixels -- of the triangle markers used to show the different image sets.
			int triangle_size;
	};
}
