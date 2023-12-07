// DarkMark (C) 2019-2023 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"


dm::DMContentFlipImages::DMContentFlipImages(dm::DMContent & c) :
	DocumentWindow("DarkMark - Flip Images", Colours::darkgrey, TitleBarButtons::closeButton),
	ThreadWithProgressWindow("DarkMark - Flip Images", true, true),
	content(c),
	tb_flip_h			("flip horizontal (left <-> right)"),
	tb_flip_v			("flip vertical (top <-> bottom)"),
	tb_save_as_png		("save new images as PNG"		),
	tb_save_as_jpeg		("save new images as JPEG"		),
	txt_jpeg_quality	("", "image quality:"			),
	sl_jpeg_quality		(Slider::SliderStyle::LinearHorizontal, Slider::TextEntryBoxPosition::TextBoxRight),

	tb_annotated_images	("flip images which contain 1 or more annotation"	),
	tb_empty_images		("flip empty images (negative samples)"				),
	tb_other_images		("flip non-annotated images"						),

	help				("Read Me!"),
	cancel				("Cancel"),
	ok					("Flip"),
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
	canvas.addAndMakeVisible(tb_flip_h				);
	canvas.addAndMakeVisible(tb_flip_v				);

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

	tb_flip_h				.addListener(this);
	tb_flip_v				.addListener(this);
	tb_save_as_png			.addListener(this);
	tb_save_as_jpeg			.addListener(this);
	tb_annotated_images		.addListener(this);
	tb_empty_images			.addListener(this);
	tb_other_images			.addListener(this);
	help					.addListener(this);
	cancel					.addListener(this);
	ok						.addListener(this);

	tb_flip_h				.setToggleState(true, NotificationType::sendNotification);
	tb_save_as_jpeg			.setToggleState(true, NotificationType::sendNotification);
	tb_annotated_images		.setToggleState(true, NotificationType::sendNotification);
	tb_empty_images			.setToggleState(true, NotificationType::sendNotification);
	tb_annotated_images		.setToggleState(true, NotificationType::sendNotification);

	header_message.setText(
		"This will flip (mirror) images horizontally or vertically.\n"
		"\n"
		"Do not run this if the network you are training needs to detect "
		"objects such as left hand vs right hand, or 'b' vs 'd', where "
		"the mirror image alters the class.",
		NotificationType::sendNotification);

	setIcon(DarkMarkLogo());
	ComponentPeer *peer = getPeer();
	if (peer)
	{
		peer->setIcon(DarkMarkLogo());
	}

	if (cfg().containsKey("FlipImagesWnd"))
	{
		restoreWindowStateFromString(cfg().getValue("FlipImagesWnd"));
	}
	else
	{
		centreWithSize(400, 400);
	}

	setVisible(true);

	return;
}


dm::DMContentFlipImages::~DMContentFlipImages()
{
	signalThreadShouldExit();
	cfg().setValue("FlipImagesWnd", getWindowStateAsString());

	return;
}


void dm::DMContentFlipImages::closeButtonPressed()
{
	// close button

	setVisible(false);
	exitModalState(0);

	return;
}


void dm::DMContentFlipImages::userTriedToCloseWindow()
{
	// ALT+F4

	closeButtonPressed();

	return;
}


void dm::DMContentFlipImages::resized()
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
	fb_rows.items.add(FlexItem(tb_flip_h				).withHeight(height));
	fb_rows.items.add(FlexItem(tb_flip_v				).withHeight(height));

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


void dm::DMContentFlipImages::buttonClicked(Button * button)
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

	const bool flip_h = tb_flip_h.getToggleState();
	const bool flip_v = tb_flip_v.getToggleState();

	const bool annotated = tb_annotated_images.getToggleState();
	const bool empty = tb_empty_images.getToggleState();
	const bool other = tb_other_images.getToggleState();

	if	(	(flip_h == false and flip_v == false)
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


void dm::DMContentFlipImages::run()
{
	DarkMarkApplication::setup_signal_handling();

	const bool flip_h = tb_flip_h.getToggleState();
	const bool flip_v = tb_flip_v.getToggleState();

	/* CV flip codes:
	 *
	 *		1 == horizontal flip (left <-> right)
	 *		0 == vertical flip (top <-> bottom)
	 *		-1 == horizontal + vertical, same as 180 degree rotation
	 */
	std::map<int, std::string> flips;

	if (flip_h) flips[1] = "_fh";
	if (flip_v) flips[0] = "_fv";

	const bool flip_annotated_images	= tb_annotated_images.getToggleState();
	const bool flip_empty_images		= tb_empty_images.getToggleState();
	const bool flip_other_images		= tb_other_images.getToggleState();

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

			Log("flip: next image at idx=" + std::to_string(idx) + " is " + content.image_filenames[idx]);

			setProgress(work_completed / work_to_be_done);
			work_completed ++;

			File original_file(content.image_filenames.at(idx));
			remember_most_recent_filename = original_file.getFullPathName();
			const String original_fn = original_file.getFileNameWithoutExtension();
			if (original_fn.contains("_fh") or
				original_fn.contains("_fv"))
			{
				// this file is the result of a previous rotation, so skip it
				images_skipped ++;
				continue;
			}

			// load the given image so we can get access to the cv::Mat and annotations
			Log("flip: loading image #" + std::to_string(idx) + ": " + content.image_filenames[idx]);
			content.load_image(idx);
			Log("flip: done loading image");

			if (content.original_image.empty() or
				content.original_image.cols < 1 or
				content.original_image.rows < 1)
			{
				// something is wrong with this image
				Log("flip is skipping a bad file: " + content.image_filenames[idx]);
				images_with_errors ++;
				images_skipped ++;
				continue;
			}

			if ((flip_empty_images and content.image_is_completely_empty) or
				(flip_other_images and content.image_is_completely_empty == false and content.marks.empty()) or
				(flip_annotated_images and content.image_is_completely_empty == false and content.marks.size() > 0))
			{
				// if we get here then we have an image we want to flip

				// remember the image and the markup for this image, because we're going to have to rotate all the points
				auto original_marks = content.marks;
				cv::Mat original_mat = content.original_image;

				for (const auto & [flip_code, postfix] : flips)
				{
					if (threadShouldExit())
					{
						break;
					}

					//	0 == vertical flip (top <-> bottom)
					//	1 == horizontal flip (left <-> right)
					const bool is_ver = (flip_code == 0);
					const bool is_hor = (flip_code == 1);

					// see if this rotation already exists
					std::string new_fn = original_file.getSiblingFile(original_fn).getFullPathName().toStdString() + postfix;
					Log("flip: looking for " + new_fn);
					if (filenames_without_extensions.count(new_fn))
					{
						Log("skip flip (already exists): " + new_fn);
						images_already_exist ++;
						continue;
					}

					// once we get here we know we need to create a new image!
					Log("flip " + content.image_filenames[idx] + ": " + std::to_string(flip_code) + ": " + postfix);

					// flip and save the image to disk
					cv::Mat dst;
					cv::flip(original_mat, dst, flip_code);
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

						Log("flip: creating annotations for " + txt_fn);

						std::ofstream ofs(txt_fn);

						const double rows = dst.rows;
						const double cols = dst.cols;

						for (auto & m : original_marks)
						{
							if (m.is_prediction)
							{
								continue;
							}

							const cv::Rect2d r	= m.get_bounding_rect(dst.size());
							const double w		= r.width;
							const double h		= r.height;
							const double x		= r.x;
							const double y		= r.y;
							const double flip_x	= cols - x - w;
							const double flip_y	= rows - y - h;
							const double cx		= (is_hor ? flip_x : x) + w / 2.0;
							const double cy		= (is_ver ? flip_y : y) + h / 2.0;
							const double new_w	= w / cols;
							const double new_h	= h / rows;
							const double new_x	= cx / cols;
							const double new_y	= cy / rows;

							ofs << std::fixed << std::setprecision(10) << m.class_idx << " " << new_x << " " << new_y << " " << new_w << " " << new_h << std::endl;
						}

						if (ofs.fail())
						{
							Log("Flip:  error saving " + txt_fn);
							AlertWindow::showMessageBox(
								AlertWindow::AlertIconType::WarningIcon,
								"DarkMark",
								"Failed to save the flipped annotations to " + txt_fn + ".\n"
								"\n"
								"Is the drive full?  Perhaps a read-only file or directory?");
							break;
						}

						ofs.close();

						// load the new images to force DarkMark to create the .json file from the .txt file
						Log("flip: reloading " + new_fn);
						content.load_image(content.image_filenames.size() - 1);
						Log("flip: done loading " + new_fn);
					}
				}
			}
			else
			{
				Log("flip: skipping " + content.image_filenames[idx]);
				images_skipped ++;
			}
		}
	}
	catch (const std::exception & e)
	{
		const String msg = "Error during flip of \"" + remember_most_recent_filename + "\": " + e.what();
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
		ss	<< "Summary of image flip:" << std::endl
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
			ss << "- images already existed: " << images_already_exist << std::endl;
		}
		ss << "- new images created: " << images_created << std::endl;

		Log(ss.str());
		AlertWindow::showMessageBox(AlertWindow::AlertIconType::InfoIcon, "DarkMark", ss.str());
	}

	return;
}
