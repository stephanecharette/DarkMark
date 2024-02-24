// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	class FilterWnd : public DocumentWindow, public Button::Listener, public Value::Listener, public Timer
	{
		public:

			FilterWnd(DMContent & c);

			virtual ~FilterWnd();

			virtual void closeButtonPressed() override;
			virtual void userTriedToCloseWindow() override;
			virtual void resized() override;
			virtual void buttonClicked(Button * button) override;
			virtual void valueChanged(Value & value) override;
			virtual void timerCallback() override;

			void apply_filters_on_thread();

			Value v_total_number_of_images;
			Value v_images_after_regex;
			Value v_images_after_filters;
			Value v_usable_images;

			Value v_inclusion_regex;
			Value v_exclusion_regex;
			Value v_age_of_annotations;
			Value v_include_all_classes;
			Value v_include_empty_images;
			Value v_include_non_annotated_images;

			std::vector<Value> value_for_each_class;
			std::vector<BooleanPropertyComponent*> checkbox_for_each_class;

			ButtonPropertyComponent * select_all;
			ButtonPropertyComponent * select_none;

			DMContent & content;
			Component canvas;
			PropertyPanel pp;

			TextButton cancel_button;
			TextButton apply_button;
			TextButton ok_button;

			std::atomic<bool> worker_thread_needs_to_end;

			std::thread worker_thread;

			VStr filtered_image_filenames;
			SId class_ids_to_include;
	};
}
