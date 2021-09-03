// DarkMark (C) 2019-2021 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"
#include <random>
#include <magic.h>


dm::VideoImportWindow::VideoImportWindow(const std::string & dir, const VStr & v) :
	DocumentWindow("DarkMark - Import Video Frames", Colours::darkgrey, TitleBarButtons::closeButton),
	ThreadWithProgressWindow("DarkMark", true, true							),
	base_directory			(dir											),
	filenames				(v												),
	tb_extract_all			("extract all frames"							),
	tb_extract_sequences	("extract sequences of consecutive frames"		),
	sl_sequences			(Slider::SliderStyle::LinearHorizontal, Slider::TextEntryBoxPosition::TextBoxRight),
	txt_sequences			("", "sequences"								),
	sl_consecutive_frames	(Slider::SliderStyle::LinearHorizontal, Slider::TextEntryBoxPosition::TextBoxRight),
	txt_consecutive_frames	("", "consecutive frames"						),
	tb_extract_maximum		("maximum number of random frames to extract:"	),
	sl_maximum				(Slider::SliderStyle::LinearHorizontal, Slider::TextEntryBoxPosition::TextBoxRight),
	tb_extract_percentage	("percentage of random frames to extract:"		),
	sl_percentage			(Slider::SliderStyle::LinearHorizontal, Slider::TextEntryBoxPosition::TextBoxRight),
	tb_do_not_resize		("do not resize frames"							),
	tb_resize				("resize frames:"								),
	txt_x					("", "x"										),
	tb_keep_aspect_ratio	("maintain aspect ratio"						),
	tb_force_resize			("resize to exact dimensions"					),
	tb_save_as_png			("save as PNG"									),
	tb_save_as_jpeg			("save as JPEG"									),
	txt_jpeg_quality		("", "image quality:"							),
	sl_jpeg_quality			(Slider::SliderStyle::LinearHorizontal, Slider::TextEntryBoxPosition::TextBoxRight),
	cancel					("Cancel"),
	ok						("Import"),
	extra_lines_needed(0),
	number_of_processed_frames(0)
{
	setContentNonOwned		(&canvas, true	);
	setUsingNativeTitleBar	(true			);
	setResizable			(true, false	);
	setDropShadowEnabled	(true			);

	canvas.addAndMakeVisible(header_message			);
	canvas.addAndMakeVisible(tb_extract_all			);
	canvas.addAndMakeVisible(tb_extract_sequences	);
	canvas.addAndMakeVisible(sl_sequences			);
	canvas.addAndMakeVisible(txt_sequences			);
	canvas.addAndMakeVisible(sl_consecutive_frames	);
	canvas.addAndMakeVisible(txt_consecutive_frames	);
	canvas.addAndMakeVisible(tb_extract_maximum		);
	canvas.addAndMakeVisible(sl_maximum				);
	canvas.addAndMakeVisible(tb_extract_percentage	);
	canvas.addAndMakeVisible(sl_percentage			);
	canvas.addAndMakeVisible(tb_do_not_resize		);
	canvas.addAndMakeVisible(tb_resize				);
	canvas.addAndMakeVisible(ef_width				);
	canvas.addAndMakeVisible(txt_x					);
	canvas.addAndMakeVisible(ef_height				);
	canvas.addAndMakeVisible(tb_keep_aspect_ratio	);
	canvas.addAndMakeVisible(tb_force_resize		);
	canvas.addAndMakeVisible(tb_save_as_png			);
	canvas.addAndMakeVisible(tb_save_as_jpeg		);
	canvas.addAndMakeVisible(txt_jpeg_quality		);
	canvas.addAndMakeVisible(sl_jpeg_quality		);
	canvas.addAndMakeVisible(cancel					);
	canvas.addAndMakeVisible(ok						);

	tb_extract_all			.setRadioGroupId(1);
	tb_extract_sequences	.setRadioGroupId(1);
	tb_extract_maximum		.setRadioGroupId(1);
	tb_extract_percentage	.setRadioGroupId(1);

	tb_do_not_resize		.setRadioGroupId(2);
	tb_resize				.setRadioGroupId(2);

	tb_keep_aspect_ratio	.setRadioGroupId(3);
	tb_force_resize			.setRadioGroupId(3);

	tb_save_as_png			.setRadioGroupId(4);
	tb_save_as_jpeg			.setRadioGroupId(4);

	tb_extract_all			.addListener(this);
	tb_extract_sequences	.addListener(this);
	tb_extract_maximum		.addListener(this);
	tb_extract_percentage	.addListener(this);
	tb_do_not_resize		.addListener(this);
	tb_resize				.addListener(this);
	tb_keep_aspect_ratio	.addListener(this);
	tb_force_resize			.addListener(this);
	tb_save_as_png			.addListener(this);
	tb_save_as_jpeg			.addListener(this);
	cancel					.addListener(this);
	ok						.addListener(this);

	tb_extract_sequences	.setToggleState(true, NotificationType::sendNotification);
	tb_do_not_resize		.setToggleState(true, NotificationType::sendNotification);
	tb_keep_aspect_ratio	.setToggleState(true, NotificationType::sendNotification);
	tb_save_as_jpeg			.setToggleState(true, NotificationType::sendNotification);

	sl_sequences			.setRange(1.0, 999.0, 1.0);
	sl_sequences			.setNumDecimalPlacesToDisplay(0);
	sl_sequences			.setValue(10);

	sl_consecutive_frames	.setRange(10.0, 9999.0, 1.0);
	sl_consecutive_frames	.setNumDecimalPlacesToDisplay(0);
	sl_consecutive_frames	.setValue(30);

	sl_maximum				.setRange(1.0, 9999.0, 1.0);
	sl_maximum				.setNumDecimalPlacesToDisplay(0);
	sl_maximum				.setValue(500.0);

	sl_percentage			.setRange(1.0, 99.0, 1.0);
	sl_percentage			.setNumDecimalPlacesToDisplay(0);
	sl_percentage			.setValue(25.0);

	sl_jpeg_quality			.setRange(30.0, 99.0, 1.0);
	sl_jpeg_quality			.setNumDecimalPlacesToDisplay(0);
	sl_jpeg_quality			.setValue(75.0);

	ef_width				.setInputRestrictions(5, "0123456789");
	ef_height				.setInputRestrictions(5, "0123456789");
	ef_width				.setText("640");
	ef_height				.setText("480");
	ef_width				.setJustification(Justification::centred);
	ef_height				.setJustification(Justification::centred);

	std::stringstream ss;
	try
	{
		const std::string filename = filenames.at(0);
		const std::string shortname = File(filename).getFileName().toStdString();

		cv::VideoCapture cap;
		const bool success = cap.open(filename);
		if (not success)
		{
			throw std::runtime_error("failed to open video file " + filename);
		}

		const auto number_of_frames		= cap.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_COUNT			);
		const auto fps					= cap.get(cv::VideoCaptureProperties::CAP_PROP_FPS					);
		const auto frame_width			= cap.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH			);
		const auto frame_height			= cap.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT			);

		const auto fpm					= fps * 60.0;
		const auto len_minutes			= std::floor(number_of_frames / fpm);
		const auto len_seconds			= (number_of_frames - (len_minutes * fpm)) / fps;

		std::string opencv_ver_and_name	= "OpenCV v" + std::to_string(CV_VERSION_MAJOR) + "." + std::to_string(CV_VERSION_MINOR) + "." + std::to_string(CV_VERSION_REVISION);

		// Almost all videos will be "avc1" and "I420", but every once in a while we might see something else.
		const uint32_t fourcc			= cap.get(cv::VideoCaptureProperties::CAP_PROP_FOURCC				);

#if CV_VERSION_MAJOR >= 4
		// Turns out the pixel format enum and backend name only exists in OpenCV v4.x and newer,
		// but Ubuntu 18.04 is still using the older version 3.2, so those have to be handled differently.
		const uint32_t format			= cap.get(cv::VideoCaptureProperties::CAP_PROP_CODEC_PIXEL_FORMAT	);
		opencv_ver_and_name				+= "/" + cap.getBackendName();
#else
		const uint32_t format			= 0;
#endif

		const std::string fourcc_str	= (fourcc == 0 ? std::to_string(fourcc) : std::string(reinterpret_cast<const char *>(&fourcc), 4));
		const std::string format_str	= (format == 0 ? std::to_string(format) : std::string(reinterpret_cast<const char *>(&format), 4));

		std::string description;
		magic_t magic_cookie = magic_open(MAGIC_NONE);
		if (magic_cookie != 0)
		{
			const auto rc = magic_load(magic_cookie, nullptr);
			if (rc == 0)
			{
				description = magic_file(magic_cookie, filename.c_str());
			}
			magic_close(magic_cookie);
		}

		if (v.size() > 1)
		{
			ss << "There are " << v.size() << " videos to import. The settings below will be applied individually to each video." << std::endl << std::endl;
			extra_lines_needed += 2;
		}

		ss	<< "Using " << opencv_ver_and_name << " to read the video file \"" << shortname << "\"." << std::endl
			<< std::endl
			<< "File type is FourCC: " << fourcc_str << ", pixel format: " << format_str
			<< (description.empty() ? "" : ", " + description) << "." << std::endl
			<< std::endl
			<< "The video contains " << number_of_frames << " individual frames at " << fps << " FPS "
			<< "for a total length of " << len_minutes << "m " << std::fixed << std::setprecision(1) << len_seconds << "s. " << std::endl
			<< std::endl
			<< "Each frame measures " << static_cast<int>(frame_width) << " x " << static_cast<int>(frame_height) << ".";
	}
	catch (...)
	{
		ss << "Error reading video file \"" << filenames.at(0) << "\".";
	}
	header_message.setText(ss.str(), NotificationType::sendNotification);

	setIcon(DarkMarkLogo());
	ComponentPeer *peer = getPeer();
	if (peer)
	{
		peer->setIcon(DarkMarkLogo());
	}

	if (cfg().containsKey("VideoImportWnd"))
	{
		restoreWindowStateFromString(cfg().getValue("VideoImportWnd"));
	}
	else
	{
		centreWithSize(380, 600);
	}

	setVisible(true);

	return;
}


dm::VideoImportWindow::~VideoImportWindow()
{
	signalThreadShouldExit();
	cfg().setValue("VideoImportWnd", getWindowStateAsString());

	return;
}


void dm::VideoImportWindow::closeButtonPressed()
{
	// close button

	setVisible(false);
	exitModalState(0);

	return;
}


void dm::VideoImportWindow::userTriedToCloseWindow()
{
	// ALT+F4

	closeButtonPressed();

	return;
}


void dm::VideoImportWindow::resized()
{
	// get the document window to resize the canvas, then we'll deal with the rest of the components
	DocumentWindow::resized();

	const auto height = 20.0f;
	const auto margin_size = 5;
	const FlexItem::Margin left_indent(0.0f, 0.0f, 0.0f, margin_size * 5.0f);
	const FlexItem::Margin new_row_indent(margin_size * 5.0f, 0.0f, 0.0f, 0.0f);

	FlexBox fb_rows;
	fb_rows.flexDirection	= FlexBox::Direction::column;
	fb_rows.alignItems		= FlexBox::AlignItems::stretch;
	fb_rows.justifyContent	= FlexBox::JustifyContent::flexStart;

	fb_rows.items.add(FlexItem(header_message			).withHeight(height * (8 + extra_lines_needed)).withFlex(1.0));
	fb_rows.items.add(FlexItem(tb_extract_all			).withHeight(height).withMargin(new_row_indent));
	fb_rows.items.add(FlexItem(tb_extract_sequences		).withHeight(height));

	FlexBox fb_sequences_1;
	fb_sequences_1.flexDirection	= FlexBox::Direction::row;
	fb_sequences_1.justifyContent	= FlexBox::JustifyContent::flexStart;
	fb_sequences_1.items.add(FlexItem(sl_sequences		).withHeight(height).withWidth(150.0f));
	fb_sequences_1.items.add(FlexItem(txt_sequences		).withHeight(height).withWidth(150.0f));
	fb_rows.items.add(FlexItem(fb_sequences_1			).withHeight(height).withMargin(left_indent));

	FlexBox fb_sequences_2;
	fb_sequences_2.flexDirection	= FlexBox::Direction::row;
	fb_sequences_2.justifyContent	= FlexBox::JustifyContent::flexStart;
	fb_sequences_2.items.add(FlexItem(sl_consecutive_frames	).withHeight(height).withWidth(150.0f));
	fb_sequences_2.items.add(FlexItem(txt_consecutive_frames).withHeight(height).withWidth(150.0f));
	fb_rows.items.add(FlexItem(fb_sequences_2			).withHeight(height).withMargin(left_indent));

	fb_rows.items.add(FlexItem(tb_extract_maximum		).withHeight(height));
	fb_rows.items.add(FlexItem(sl_maximum				).withHeight(height).withWidth(150.0f).withMargin(left_indent));
	fb_rows.items.add(FlexItem(tb_extract_percentage	).withHeight(height));
	fb_rows.items.add(FlexItem(sl_percentage			).withHeight(height).withWidth(150.0f).withMargin(left_indent));
	fb_rows.items.add(FlexItem(tb_do_not_resize			).withHeight(height).withMargin(new_row_indent));
	fb_rows.items.add(FlexItem(tb_resize				).withHeight(height));

	FlexBox fb_dimensions;
	fb_dimensions.flexDirection	= FlexBox::Direction::row;
	fb_dimensions.justifyContent = FlexBox::JustifyContent::flexStart;
	fb_dimensions.items.add(FlexItem(ef_width			).withHeight(height).withWidth(50.0f));
	fb_dimensions.items.add(FlexItem(txt_x				).withHeight(height).withWidth(20.0f));
	fb_dimensions.items.add(FlexItem(ef_height			).withHeight(height).withWidth(50.0f));
	fb_rows.items.add(FlexItem(fb_dimensions			).withHeight(height).withMargin(left_indent));

	fb_rows.items.add(FlexItem(tb_keep_aspect_ratio		).withHeight(height).withMargin(left_indent));
	fb_rows.items.add(FlexItem(tb_force_resize			).withHeight(height).withMargin(left_indent));
	fb_rows.items.add(FlexItem(tb_save_as_png			).withHeight(height).withMargin(new_row_indent));
	fb_rows.items.add(FlexItem(tb_save_as_jpeg			).withHeight(height));

	FlexBox fb_quality;
	fb_quality.flexDirection = FlexBox::Direction::row;
	fb_quality.justifyContent = FlexBox::JustifyContent::flexStart;
	fb_quality.items.add(FlexItem(txt_jpeg_quality		).withHeight(height).withWidth(100.0f));
	fb_quality.items.add(FlexItem(sl_jpeg_quality		).withHeight(height).withWidth(150.0f));
	fb_rows.items.add(FlexItem(fb_quality				).withHeight(height).withMargin(left_indent));

	FlexBox button_row;
	button_row.flexDirection = FlexBox::Direction::row;
	button_row.justifyContent = FlexBox::JustifyContent::flexEnd;
	button_row.items.add(FlexItem()				.withFlex(1.0));
	button_row.items.add(FlexItem(cancel)		.withWidth(100.0).withMargin(FlexItem::Margin(0, margin_size, 0, 0)));
	button_row.items.add(FlexItem(ok)			.withWidth(100.0));

	fb_rows.items.add(FlexItem().withFlex(1.0));
	fb_rows.items.add(FlexItem(button_row).withHeight(30.0));

	auto r = getLocalBounds();
	r.reduce(margin_size, margin_size);
	fb_rows.performLayout(r);

	return;
}


void dm::VideoImportWindow::buttonClicked(Button * button)
{
	if (button == &cancel)
	{
		closeButtonPressed();
		return;
	}

	bool b = tb_extract_maximum.getToggleState();
	sl_maximum.setEnabled(b);

	b = tb_extract_sequences.getToggleState();
	sl_sequences.setEnabled(b);
	txt_sequences.setEnabled(b);
	sl_consecutive_frames.setEnabled(b);
	txt_consecutive_frames.setEnabled(b);

	b = tb_extract_percentage.getToggleState();
	sl_percentage.setEnabled(b);

	b = tb_resize.getToggleState();
	ef_width.setEnabled(b);
	txt_x.setEnabled(b);
	ef_height.setEnabled(b);
	tb_keep_aspect_ratio.setEnabled(b);
	tb_force_resize.setEnabled(b);

	b = tb_save_as_jpeg.getToggleState();
	txt_jpeg_quality.setEnabled(b);
	sl_jpeg_quality.setEnabled(b);

	if (button == &ok)
	{
		// disable all of the controls and start the frame extraction
		canvas.setEnabled(false);
		runThread(); // this waits for the thread to be done
		closeButtonPressed();
	}

	return;
}


void dm::VideoImportWindow::run()
{
	std::string current_filename		= "?";
	double work_completed				= 0.0;
	double work_to_be_done				= 1.0;
	bool error_shown					= false;
	number_of_processed_frames			= 0;

	try
	{
		const bool extract_all_frames		= tb_extract_all		.getToggleState();
		const bool extract_sequences		= tb_extract_sequences	.getToggleState();
		const bool extract_maximum_frames	= tb_extract_maximum	.getToggleState();
		const bool extract_percentage		= tb_extract_percentage	.getToggleState();
		const double number_of_sequences	= sl_sequences			.getValue();
		const double consecutive_frames		= sl_consecutive_frames	.getValue();
		const double maximum_to_extract		= sl_maximum			.getValue();
		const double percent_to_extract		= sl_percentage			.getValue() / 100.0;
		const bool resize_frame				= tb_resize				.getToggleState();
		const bool maintain_aspect_ratio	= tb_keep_aspect_ratio	.getToggleState();
		const int new_width					= std::atoi(ef_width	.getText().toStdString().c_str());
		const int new_height				= std::atoi(ef_height	.getText().toStdString().c_str());
		const bool save_as_png				= tb_save_as_png		.getToggleState();
		const bool save_as_jpg				= tb_save_as_jpeg		.getToggleState();
		const int jpg_quality				= sl_jpeg_quality		.getValue();

		setStatusMessage("Determining the amount of frames to extract...");

		for (auto && filename : filenames)
		{
			if (threadShouldExit())
			{
				break;
			}

			current_filename = filename;

			cv::VideoCapture cap;
			cap.open(filename);
			const auto number_of_frames = cap.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_COUNT);

			if (extract_all_frames)
			{
				work_to_be_done += number_of_frames;
			}
			if (extract_sequences)
			{
				const double work = number_of_sequences * consecutive_frames;
				work_to_be_done += std::min(work, number_of_frames);
			}
			else if (extract_maximum_frames)
			{
				work_to_be_done += std::min(number_of_frames, maximum_to_extract);
			}
			else if (extract_percentage)
			{
				work_to_be_done += number_of_frames * percent_to_extract;
			}
		}

		// now start actually extracting frames
		for (auto && filename : filenames)
		{
			if (threadShouldExit())
			{
				break;
			}

			current_filename = filename;

			const std::string shortname = File(filename).getFileName().toStdString(); // filename+extension, but no path

			std::string sanitized_name = shortname;
			while (true)
			{
				auto p = sanitized_name.find_first_not_of(
					"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
					"abcdefghijklmnopqrstuvwxyz"
					"0123456789"
					"-_");
				if (p == std::string::npos)
				{
					break;
				}
				sanitized_name[p] = '_';
			}

			File dir(base_directory);
			File child = dir.getChildFile(Time::getCurrentTime().formatted("video_import_%Y-%m-%d_%H-%M-%S_" + sanitized_name));
			child.createDirectory();
			std::string partial_output_filename = child.getChildFile(shortname).getFullPathName().toStdString();
			size_t pos = partial_output_filename.rfind("."); // erase the extension if we find one
			if (pos != std::string::npos)
			{
				partial_output_filename.erase(pos);
			}

			setStatusMessage("Processing video file " + shortname + "...");

			cv::VideoCapture cap;
			cap.open(filename);
			const auto number_of_frames = cap.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_COUNT);

			std::random_device rd;
			std::mt19937 rng(rd());
			std::uniform_int_distribution<size_t> uni(0, number_of_frames - 1);

			std::set<size_t> frames_needed;
			if (extract_sequences)
			{
				/* Say the video is this long:
				 *
				 *		[...............................]
				 *
				 * If we need to extract 3 sequences, then we divide it into 3+1 sections:
				 *
				 *		[...............................]
				 *		|		|		|		|		|
				 *		0		1		2		3		4
				 *
				 * Each of those numbered sections becomes the mid-point of a sequence,
				 * so we take some frames before and some frames after each mid-point.
				 */
				for (size_t sequence_counter = 1; sequence_counter <= number_of_sequences; sequence_counter ++)
				{
					const size_t mid_point = number_of_frames * sequence_counter / (number_of_sequences + 1);
					const size_t half = consecutive_frames / 2;
					size_t min_point = 0;
					size_t max_point = mid_point + half;
					if (mid_point > half)
					{
						min_point = mid_point - half;
					}
					if (max_point > number_of_frames - 1)
					{
						max_point = number_of_frames - 1;
					}

					// if the number of frames is even, then the mid point + 2 * half will give us 1 too many frames
					if (max_point - min_point + 1 > consecutive_frames)
					{
						max_point -= 1;
					}

					Log("sequence counter .. " + std::to_string(sequence_counter));
					Log("-> min point ...... " + std::to_string(min_point));
					Log("-> mid point ...... " + std::to_string(mid_point));
					Log("-> max point ...... " + std::to_string(max_point));
					Log("-> total frames ... " + std::to_string(max_point - min_point + 1));

					for (size_t frame = min_point; frame <= max_point; frame ++)
					{
						frames_needed.insert(frame);
					}
				}
			}
			else if (extract_maximum_frames)
			{
				while (threadShouldExit() == false and frames_needed.size() < std::min(number_of_frames, maximum_to_extract))
				{
					const auto random_frame = uni(rng);
					frames_needed.insert(random_frame);
				}
			}
			else if (extract_percentage)
			{
				while (threadShouldExit() == false and frames_needed.size() < percent_to_extract * number_of_frames)
				{
					const auto random_frame = uni(rng);
					frames_needed.insert(random_frame);
				}
			}

			Log("about to start extracting " + std::to_string(frames_needed.size()) + " frames from " + filename);

			size_t frame_number = 0;
			while (threadShouldExit() == false)
			{
				if (extract_sequences or extract_maximum_frames or extract_percentage)
				{
					if (frames_needed.empty())
					{
						// we've extracted all the frames we need
						break;
					}

					const auto next_frame_needed = *frames_needed.begin();
					frames_needed.erase(next_frame_needed);
					if (frame_number != next_frame_needed)
					{
						// only explicitely set the absolute frame position if the frames are not consecutive
						cap.set(cv::VideoCaptureProperties::CAP_PROP_POS_FRAMES, static_cast<double>(next_frame_needed));
						frame_number = next_frame_needed;
					}
				}

				cv::Mat mat;
				cap >> mat;
				if (mat.empty())
				{
					// must have reached the EOF
					Log("received an empty mat while reading frame #" + std::to_string(frame_number));
					continue;
				}

				if (resize_frame and (mat.cols != new_width or mat.rows != new_height))
				{
					if (maintain_aspect_ratio)
					{
						mat = resize_keeping_aspect_ratio(mat, {new_width, new_height});
					}
					else
					{
						cv::Mat dst;
						cv::resize(mat, dst, {new_width, new_height}, 0, 0,  CV_INTER_AREA);
						mat = dst;
					}
				}

				std::stringstream ss;
				ss << partial_output_filename << "_frame_" << std::setfill('0') << std::setw(6) << frame_number;

				if (save_as_png)
				{
					cv::imwrite(ss.str() + ".png", mat, {CV_IMWRITE_PNG_COMPRESSION, 9});
				}
				else if (save_as_jpg)
				{
					cv::imwrite(ss.str() + ".jpg", mat, {CV_IMWRITE_JPEG_QUALITY, jpg_quality});
				}

				frame_number ++;
				number_of_processed_frames ++;
				work_completed += 1.0;
				setProgress(work_completed / work_to_be_done);
			}
		}
	}
	catch (const std::exception & e)
	{
		std::stringstream ss;
		ss	<< "An error was detected while processing the video file \"" + current_filename + "\":" << std::endl
			<< std::endl
			<< e.what();
		dm::Log(ss.str());
		error_shown = true;
		AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "DarkMark - Error!", ss.str());
	}
	catch (...)
	{
		const std::string msg = "An unknown error was encountered while processing the video file \"" + current_filename + "\".";
		dm::Log(msg);
		error_shown = true;
		AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "DarkMark - Error!", msg);
	}

	File dir(base_directory);
	dir.revealToUser();

	if (error_shown == false and threadShouldExit() == false)
	{
		Log("finished extracting " + std::to_string(number_of_processed_frames) + " video frames");
		AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::InfoIcon, "DarkMark", "Extracted " + std::to_string(number_of_processed_frames) + " video frames.");
	}

	return;
}
