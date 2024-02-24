// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>
#include <cmath>

#include "DarkMark.hpp"


dm::DMContentRotateImages::DMContentRotateImages(dm::DMContent & c) :
	DocumentWindow("DarkMark - Rotate Images", Colours::darkgrey, TitleBarButtons::closeButton),
	ThreadWithProgressWindow("DarkMark - Rotate Images", true, true),
	content(c),
	tb_090_degrees		(juce::CharPointer_UTF8("rotate images 90\xc2\xb0")),
	tb_180_degrees		(juce::CharPointer_UTF8("rotate images 180\xc2\xb0")),
	tb_270_degrees		(juce::CharPointer_UTF8("rotate images 270\xc2\xb0")),

	tb_save_as_png		("save new images as PNG"		),
	tb_save_as_jpeg		("save new images as JPEG"		),
	txt_jpeg_quality	("", "image quality:"			),
	sl_jpeg_quality		(Slider::SliderStyle::LinearHorizontal, Slider::TextEntryBoxPosition::TextBoxRight),

	tb_annotated_images	("rotate images which contain 1 or more annotation"	),
	tb_empty_images		("rotate empty images (negative samples)"			),
	tb_other_images		("rotate non-annotated images"						),

	help				("Read Me!"),
	cancel				("Cancel"),
	ok					("Rotate"),
	images_created		(0),
	images_skipped		(0),
	images_already_exist(0),
	images_with_errors	(0)
{
	setContentNonOwned		(&canvas, true	);
	setUsingNativeTitleBar	(true			);
	setResizable			(true, false	);
	setDropShadowEnabled	(true			);

	canvas.addAndMakeVisible(header_message			);
	canvas.addAndMakeVisible(tb_090_degrees			);
	canvas.addAndMakeVisible(tb_180_degrees			);
	canvas.addAndMakeVisible(tb_270_degrees			);

	canvas.addAndMakeVisible(tb_save_as_png			);
	canvas.addAndMakeVisible(tb_save_as_jpeg		);
	canvas.addAndMakeVisible(txt_jpeg_quality		);
	canvas.addAndMakeVisible(sl_jpeg_quality		);

	canvas.addAndMakeVisible(tb_annotated_images	);
	canvas.addAndMakeVisible(tb_empty_images		);
	canvas.addAndMakeVisible(tb_other_images		);

	canvas.addAndMakeVisible(help					);
	canvas.addAndMakeVisible(cancel					);
	canvas.addAndMakeVisible(ok						);

	tb_save_as_png			.setRadioGroupId(1);
	tb_save_as_jpeg			.setRadioGroupId(1);

	sl_jpeg_quality			.setRange(30.0, 99.0, 1.0);
	sl_jpeg_quality			.setNumDecimalPlacesToDisplay(0);
	sl_jpeg_quality			.setValue(75.0);

	tb_090_degrees			.addListener(this);
	tb_180_degrees			.addListener(this);
	tb_270_degrees			.addListener(this);
	tb_save_as_png			.addListener(this);
	tb_save_as_jpeg			.addListener(this);
	tb_annotated_images		.addListener(this);
	tb_empty_images			.addListener(this);
	tb_other_images			.addListener(this);
	help					.addListener(this);
	cancel					.addListener(this);
	ok						.addListener(this);

	tb_090_degrees			.setToggleState(true, NotificationType::sendNotification);
	tb_180_degrees			.setToggleState(true, NotificationType::sendNotification);
	tb_270_degrees			.setToggleState(true, NotificationType::sendNotification);
	tb_save_as_jpeg			.setToggleState(true, NotificationType::sendNotification);
	tb_annotated_images		.setToggleState(true, NotificationType::sendNotification);
	tb_empty_images			.setToggleState(true, NotificationType::sendNotification);
	tb_annotated_images		.setToggleState(true, NotificationType::sendNotification);

	header_message.setText(
		"This will rotate images 90, 180, and 270 degrees, and will also copy and "
		"rotate all existing annotations for each new image.\n"
		"\n"
		"Only run this if the network you are training uses images that do not have "
		"an obvious up-down-left-right orientation. ",
		NotificationType::sendNotification);

	setIcon(DarkMarkLogo());
	ComponentPeer *peer = getPeer();
	if (peer)
	{
		peer->setIcon(DarkMarkLogo());
	}

	if (cfg().containsKey("RotateImagesWnd"))
	{
		restoreWindowStateFromString(cfg().getValue("RotateImagesWnd"));
	}
	else
	{
		centreWithSize(400, 400);
	}

	setVisible(true);

	return;
}


dm::DMContentRotateImages::~DMContentRotateImages()
{
	signalThreadShouldExit();
	cfg().setValue("RotateImagesWnd", getWindowStateAsString());

	return;
}


void dm::DMContentRotateImages::closeButtonPressed()
{
	// close button

	setVisible(false);
	exitModalState(0);

	return;
}


void dm::DMContentRotateImages::userTriedToCloseWindow()
{
	// ALT+F4

	closeButtonPressed();

	return;
}


void dm::DMContentRotateImages::resized()
{
	// get the document window to resize the canvas, then we'll deal with the rest of the components
	DocumentWindow::resized();

	const auto height = 20.0f;
	const auto margin_size = 5.0f;
	const FlexItem::Margin left_indent(0.0f, 0.0f, 0.0f, margin_size * 5.0f);

	FlexBox fb_rows;
	fb_rows.flexDirection	= FlexBox::Direction::column;
	fb_rows.alignItems		= FlexBox::AlignItems::stretch;
	fb_rows.justifyContent	= FlexBox::JustifyContent::spaceAround;

	fb_rows.items.add(FlexItem(header_message			).withHeight(height * 5));

	fb_rows.items.add(FlexItem(							).withHeight(height).withFlex(1.0));
	fb_rows.items.add(FlexItem(tb_090_degrees			).withHeight(height));
	fb_rows.items.add(FlexItem(tb_180_degrees			).withHeight(height));
	fb_rows.items.add(FlexItem(tb_270_degrees			).withHeight(height));

	fb_rows.items.add(FlexItem(							).withHeight(height).withFlex(1.0));
	fb_rows.items.add(FlexItem(tb_save_as_png			).withHeight(height));
	fb_rows.items.add(FlexItem(tb_save_as_jpeg			).withHeight(height));

	FlexBox fb_quality;
	fb_quality.flexDirection = FlexBox::Direction::row;
	fb_quality.items.add(FlexItem(txt_jpeg_quality		).withWidth(100.0f));
	fb_quality.items.add(FlexItem(sl_jpeg_quality		).withWidth(200.0f));
	fb_rows.items.add(FlexItem(fb_quality				).withHeight(height).withMargin(left_indent));

	fb_rows.items.add(FlexItem(							).withHeight(height).withFlex(1.0));
	fb_rows.items.add(FlexItem(tb_annotated_images		).withHeight(height));
	fb_rows.items.add(FlexItem(tb_empty_images			).withHeight(height));
	fb_rows.items.add(FlexItem(tb_other_images			).withHeight(height));

	FlexBox button_row;
	button_row.flexDirection = FlexBox::Direction::row;
	button_row.justifyContent = FlexBox::JustifyContent::flexEnd;
	button_row.items.add(FlexItem(help)		.withWidth(100.0));
	button_row.items.add(FlexItem()			.withFlex(1.0));
	button_row.items.add(FlexItem(cancel)	.withWidth(100.0).withMargin(FlexItem::Margin(0.0f, margin_size, 0.0f, 0.0f)));
	button_row.items.add(FlexItem(ok)		.withWidth(100.0));

	fb_rows.items.add(FlexItem(				).withHeight(height).withFlex(1.0));
	fb_rows.items.add(FlexItem(button_row	).withHeight(30.0));

	auto r = getLocalBounds();
	r.reduce(margin_size, margin_size);
	fb_rows.performLayout(r);

	return;
}


void dm::DMContentRotateImages::buttonClicked(Button * button)
{
	if (button == &help)
	{
		URL("https://www.ccoderun.ca/DarkMark/rotate_and_flip.html").launchInDefaultBrowser();
		return;
	}

	if (button == &cancel)
	{
		closeButtonPressed();
		return;
	}

	const bool r090 = tb_090_degrees.getToggleState();
	const bool r180 = tb_180_degrees.getToggleState();
	const bool r270 = tb_270_degrees.getToggleState();

	const bool annotated = tb_annotated_images.getToggleState();
	const bool empty = tb_empty_images.getToggleState();
	const bool other = tb_other_images.getToggleState();

	if	(	(r090 == false and r180 == false and r270 == false)
			or
			(annotated == false and empty == false and other == false)
		)
	{
		ok.setEnabled(false);
	}
	else
	{
		ok.setEnabled(true);
	}

	const bool b = tb_save_as_jpeg.getToggleState();
	txt_jpeg_quality.setEnabled(b);
	sl_jpeg_quality.setEnabled(b);

	if (button == &ok)
	{
		// disable all of the controls and start the rotation
		canvas.setEnabled(false);
		runThread(); // this waits for the thread to be done
		closeButtonPressed();
	}

	return;
}


void dm::DMContentRotateImages::run()
{
	DarkMarkApplication::setup_signal_handling();

	const bool r090 = tb_090_degrees.getToggleState();
	const bool r180 = tb_180_degrees.getToggleState();
	const bool r270 = tb_270_degrees.getToggleState();

	std::map<cv::RotateFlags, std::string> rotations;

	if (r090) rotations[cv::ROTATE_90_CLOCKWISE]		= "_r090";
	if (r180) rotations[cv::ROTATE_180]					= "_r180";
	if (r270) rotations[cv::ROTATE_90_COUNTERCLOCKWISE]	= "_r270";

	const bool rotate_annotated_images	= tb_annotated_images.getToggleState();
	const bool rotate_empty_images		= tb_empty_images.getToggleState();
	const bool rotate_other_images		= tb_other_images.getToggleState();

	const bool use_png = tb_save_as_png.getToggleState();
	const bool use_jpg = tb_save_as_jpeg.getToggleState();
	const int jpg_quality = sl_jpeg_quality.getValue();

	std::string current_filename	= "?";

	double work_completed			= 0.0;
	double work_to_be_done			= content.image_filenames.size();
	images_created					= 0;
	images_skipped					= 0;
	images_already_exist			= 0;
	images_with_errors				= 0;

	const auto previous_scrollfield_width = content.scrollfield_width;
	if (previous_scrollfield_width > 0)
	{
		content.scrollfield_width = 0;
		content.resized();
	}

	// briefly pause here so the window can properly redraw itself (hack?)
	getAlertWindow()->repaint();
	sleep(250); // milliseconds

	// make a set of all filenames **WITHOUT EXTENSION** so we can quickly look up if an image already exists
	SStr filenames_without_extensions;
	for (auto fn : content.image_filenames)
	{
		if (threadShouldExit())
		{
			break;
		}

		const size_t pos = fn.rfind(".");
		if (pos != std::string::npos)
		{
			fn.erase(pos);
			filenames_without_extensions.insert(fn);
		}
	}

	// disable all predictions so the images load faster
	const auto previous_predictions = content.show_predictions;
	content.show_predictions = EToggle::kOff;

	// we're going to be growing the vector of filenames with new images pushed to the back of the vector, but we don't
	// want to re-process those new images, so we must remember the max index
	const size_t original_max_idx = content.image_filenames.size();

	String remember_most_recent_filename = "?";

	try
	{
		for (size_t idx = 0; idx < original_max_idx; idx ++)
		{
			if (threadShouldExit())
			{
				break;
			}

			Log("rotation: next image at idx=" + std::to_string(idx) + " is " + content.image_filenames[idx]);

			setProgress(work_completed / work_to_be_done);
			work_completed ++;

			File original_file(content.image_filenames.at(idx));
			remember_most_recent_filename = original_file.getFullPathName();
			const String original_fn = original_file.getFileNameWithoutExtension();
			if (original_fn.contains("_r090") or
				original_fn.contains("_r180") or
				original_fn.contains("_r270"))
			{
				// this file is the result of a previous rotation, so skip it
				images_skipped ++;
				continue;
			}

			// load the given image so we can get access to the cv::Mat and annotations
			Log("rotation: loading image #" + std::to_string(idx) + ": " + content.image_filenames[idx]);
			content.load_image(idx);
			Log("rotation: done loading image");

			if (content.original_image.empty() or
				content.original_image.cols < 1 or
				content.original_image.rows < 1)
			{
				// something is wrong with this image
				Log("rotation is skipping a bad file: " + content.image_filenames[idx]);
				images_with_errors ++;
				images_skipped ++;
				continue;
			}

			if ((rotate_empty_images and content.image_is_completely_empty) or
				(rotate_other_images and content.image_is_completely_empty == false and content.marks.empty()) or
				(rotate_annotated_images and content.image_is_completely_empty == false and content.marks.size() > 0))
			{
				// if we get here then we have an image we want to rotate

				// remember the image and the markup for this image, because we're going to have to rotate all the points
				const auto original_marks = content.marks;
				cv::Mat original_mat = content.original_image;

				for (const auto & [rotation_code, postfix] : rotations)
				{
					if (threadShouldExit())
					{
						break;
					}

					// see if this rotation already exists
					std::string new_fn = original_file.getSiblingFile(original_fn).getFullPathName().toStdString() + postfix;
					Log("rotation: looking for " + new_fn);
					if (filenames_without_extensions.count(new_fn))
					{
						Log("skip rotation (already exists): " + new_fn);
						images_already_exist ++;
						continue;
					}

					// once we get here we know we need to create a new image!
					Log("rotate " + content.image_filenames[idx] + ": " + std::to_string(rotation_code) + ": " + postfix);

					// rotate and save the image to disk
					cv::Mat dst;
					cv::rotate(original_mat, dst, rotation_code);
					if (use_png)
					{
						new_fn += ".png";
						cv::imwrite(new_fn, dst, {cv::IMWRITE_PNG_COMPRESSION, 1});
					}
					else if (use_jpg)
					{
						new_fn += ".jpg";
						cv::imwrite(new_fn, dst, {cv::IMWRITE_JPEG_QUALITY, jpg_quality});
					}
					content.image_filenames.push_back(new_fn);
					images_created ++;

					if (content.image_is_completely_empty or original_marks.size() > 0)
					{
						// re-create the markup for this newly rotated image by saving a simple .txt file,
						// and then we'll get DarkMark to reload this image which should cause the .json output

						const auto txt_fn = File(new_fn).withFileExtension(".txt").getFullPathName().toStdString();

						Log("rotation: creating annotations for " + txt_fn);

						const double degrees	= (rotation_code == cv::ROTATE_90_CLOCKWISE ? 90.0 : rotation_code == cv::ROTATE_180 ? 180.0 : 270.0);
						const double rads		= degrees * M_PI / 180.0;
						const double s			= std::sin(rads);
						const double c			= std::cos(rads);

						std::ofstream ofs(txt_fn);

						for (const auto & m : original_marks)
						{
							if (m.is_prediction)
							{
								continue;
							}

							const cv::Rect2d r	= m.get_normalized_bounding_rect();
							const double w		= r.width;
							const double h		= r.height;
							const double x		= r.x + r.width / 2.0 - 0.5; // -0.5 to bring it back to the origin
							const double y		= r.y + r.height / 2.0 - 0.5; // -0.5 to bring it back to the origin

							const double new_w		= (degrees == 180.0 ? w : h);
							const double new_h		= (degrees == 180.0 ? h : w);
							const double new_x		= x * c - y * s + 0.5;
							const double new_y		= x * s + y * c + 0.5;

							ofs << std::fixed << std::setprecision(10) << m.class_idx << " " << new_x << " " << new_y << " " << new_w << " " << new_h << std::endl;
						}

						if (ofs.fail())
						{
							Log("Rotate:  error saving " + txt_fn);
							AlertWindow::showMessageBox(
								AlertWindow::AlertIconType::WarningIcon,
								"DarkMark",
								"Failed to save the rotated annotations to " + txt_fn + ".\n"
								"\n"
								"Is the drive full?  Perhaps a read-only file or directory?");
							break;
						}

						ofs.close();

						// load the new images to force DarkMark to create the .json file from the .txt file
						Log("rotation: reloading " + new_fn);
						content.load_image(content.image_filenames.size() - 1);
						Log("rotation: done loading " + new_fn);
					}
				}
			}
			else
			{
				Log("rotation: skipping " + content.image_filenames[idx]);
				images_skipped ++;
			}
		}
	}
	catch (const std::exception & e)
	{
		const String msg = "Error during rotation of \"" + remember_most_recent_filename + "\": " + e.what();
		Log(msg.toStdString());
		AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "DarkMark", msg);
	}

	setProgress(1.1);
	setStatusMessage("Sorting...");
	content.scrollfield_width = previous_scrollfield_width;
	content.show_predictions = previous_predictions;
	content.set_sort_order(ESort::kAlphabetical);
	setStatusMessage("Loading...");
	content.load_image(0);

	if (threadShouldExit() == false)
	{
		std::stringstream ss;
		ss	<< "Summary of image rotation:" << std::endl
			<< std::endl;

		ss << "- images processed: " << filenames_without_extensions.size() << std::endl;
		if (images_skipped > 0)
		{
			ss << "- images skipped: " << images_skipped << std::endl;
		}
		if (images_with_errors > 0)
		{
			ss << "- images with errors: " << images_with_errors << std::endl;
		}
		if (images_already_exist > 0)
		{
			ss << "- rotations already existed: " << images_already_exist << std::endl;
		}
		ss << "- new images created: " << images_created << std::endl;

		Log(ss.str());
		AlertWindow::showMessageBox(AlertWindow::AlertIconType::InfoIcon, "DarkMark", ss.str());
	}

	return;
}
