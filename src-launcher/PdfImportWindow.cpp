// DarkMark (C) 2019-2023 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"
#include <poppler/cpp/poppler-version.h>
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page-renderer.h>
#include <poppler/cpp/poppler-image.h>
#include <poppler/cpp/poppler-page.h>


dm::PdfImportWindow::PdfImportWindow(const std::string & dir, const VStr & v) :
	DocumentWindow("DarkMark - Import PDF Documents", Colours::darkgrey, TitleBarButtons::closeButton),
	ThreadWithProgressWindow("DarkMark", true, true							),
	base_directory			(dir											),
	filenames				(v												),
	txt_dpi					("", "dpi:"										),
	sl_dpi					(Slider::SliderStyle::LinearHorizontal, Slider::TextEntryBoxPosition::TextBoxRight),
	tb_do_not_resize		("do not resize pages"							),
	tb_resize				("resize pages:"								),
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
	number_of_imported_pages(0)
{
	setContentNonOwned		(&canvas, true	);
	setUsingNativeTitleBar	(true			);
	setResizable			(true, false	);
	setDropShadowEnabled	(true			);

	canvas.addAndMakeVisible(header_message			);
	canvas.addAndMakeVisible(txt_dpi				);
	canvas.addAndMakeVisible(sl_dpi					);
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

	tb_do_not_resize		.setRadioGroupId(2);
	tb_resize				.setRadioGroupId(2);

	tb_keep_aspect_ratio	.setRadioGroupId(3);
	tb_force_resize			.setRadioGroupId(3);

	tb_save_as_png			.setRadioGroupId(4);
	tb_save_as_jpeg			.setRadioGroupId(4);

	tb_do_not_resize		.addListener(this);
	tb_resize				.addListener(this);
	tb_keep_aspect_ratio	.addListener(this);
	tb_force_resize			.addListener(this);
	tb_save_as_png			.addListener(this);
	tb_save_as_jpeg			.addListener(this);
	cancel					.addListener(this);
	ok						.addListener(this);

	tb_do_not_resize		.setToggleState(true, NotificationType::sendNotification);
	tb_keep_aspect_ratio	.setToggleState(true, NotificationType::sendNotification);
	tb_save_as_jpeg			.setToggleState(true, NotificationType::sendNotification);

	sl_dpi					.setRange(1.0, 9999.0, 1.0);
	sl_dpi					.setNumDecimalPlacesToDisplay(0);
	sl_dpi					.setValue(100);

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

		if (v.size() > 1)
		{
			ss << "There are " << v.size() << " PDF files to import. The settings below will be applied individually to each document." << std::endl << std::endl;
			extra_lines_needed += 2;
		}

		std::unique_ptr<poppler::document> doc(poppler::document::load_from_file(v[0]));

		ss	<< "Using the PDF parsing library Poppler v" << poppler::version_string() << " to read the document \"" << shortname << "\"." << std::endl
			<< std::endl
			<< "This PDF contains " << doc->pages() << " page" << (doc->pages() == 1 ? "" : "s") << ".";
	}
	catch (...)
	{
		ss << "Error reading document \"" << filenames.at(0) << "\".";
	}
	header_message.setText(ss.str(), NotificationType::sendNotification);

	setIcon(DarkMarkLogo());
	ComponentPeer *peer = getPeer();
	if (peer)
	{
		peer->setIcon(DarkMarkLogo());
	}

	if (cfg().containsKey("PdfImportWnd"))
	{
		restoreWindowStateFromString(cfg().getValue("PdfImportWnd"));
	}
	else
	{
		centreWithSize(400, 500);
	}

	setVisible(true);

	return;
}


dm::PdfImportWindow::~PdfImportWindow()
{
	signalThreadShouldExit();
	cfg().setValue("PdfImportWnd", getWindowStateAsString());

	return;
}


void dm::PdfImportWindow::closeButtonPressed()
{
	// close button

	setVisible(false);
	exitModalState(0);

	return;
}


void dm::PdfImportWindow::userTriedToCloseWindow()
{
	// ALT+F4

	closeButtonPressed();

	return;
}


void dm::PdfImportWindow::resized()
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

	fb_rows.items.add(FlexItem(header_message			).withHeight(height * (2 + extra_lines_needed)).withFlex(1.0));

	FlexBox fb_dpi;
	fb_dpi.flexDirection = FlexBox::Direction::row;
	fb_dpi.justifyContent = FlexBox::JustifyContent::flexStart;
	fb_dpi.items.add(FlexItem(txt_dpi).withHeight(height).withWidth(40.0f));
	fb_dpi.items.add(FlexItem(sl_dpi).withHeight(height).withWidth(200.0f));
	fb_rows.items.add(FlexItem(fb_dpi).withHeight(height).withMargin(new_row_indent));

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


void dm::PdfImportWindow::buttonClicked(Button * button)
{
	if (button == &cancel)
	{
		closeButtonPressed();
		return;
	}

	bool b = tb_resize.getToggleState();
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
		// disable all of the controls and start processing the pages
		canvas.setEnabled(false);
		runThread(); // this waits for the thread to be done
		closeButtonPressed();
	}

	return;
}


void dm::PdfImportWindow::run()
{
	std::string current_filename		= "?";
	double work_completed				= 0.0;
	double work_to_be_done				= 1.0;
	bool error_shown					= false;
	number_of_imported_pages			= 0;

	try
	{
		const int dpi = sl_dpi.getValue();
		const bool resize_page				= tb_resize				.getToggleState();
		const bool maintain_aspect_ratio	= tb_keep_aspect_ratio	.getToggleState();
		const int new_width					= std::atoi(ef_width	.getText().toStdString().c_str());
		const int new_height				= std::atoi(ef_height	.getText().toStdString().c_str());
		const bool save_as_png				= tb_save_as_png		.getToggleState();
		const bool save_as_jpg				= tb_save_as_jpeg		.getToggleState();
		const int jpg_quality				= sl_jpeg_quality		.getValue();

		setStatusMessage("Determining the number of pages to import...");

		for (auto && filename : filenames)
		{
			if (threadShouldExit())
			{
				break;
			}

			current_filename = filename;
			work_to_be_done ++;

			std::unique_ptr<poppler::document> doc(poppler::document::load_from_file(filename));
			if (doc)
			{
				work_to_be_done += doc->pages();
			}
		}

		// now start actually extracting pages
		poppler::page_renderer renderer;
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
			File child = dir.getChildFile(Time::getCurrentTime().formatted("pdf_import_%Y-%m-%d_%H-%M-%S_" + sanitized_name));
			child.createDirectory();
			std::string partial_output_filename = child.getChildFile(shortname).getFullPathName().toStdString();
			size_t pos = partial_output_filename.rfind("."); // erase the extension if we find one
			if (pos != std::string::npos)
			{
				partial_output_filename.erase(pos);
			}

			work_completed += 1.0;
			setProgress(work_completed / work_to_be_done);
			setStatusMessage("Processing PDF document " + shortname + "...");

			std::unique_ptr<poppler::document> doc(poppler::document::load_from_file(filename));
			if (doc == nullptr)
			{
				continue;
			}

			for (int page_number = 0; page_number < doc->pages(); page_number++)
			{
				if (threadShouldExit())
				{
					break;
				}

				work_completed += 1.0;
				setProgress(work_completed / work_to_be_done);

				Log("about to start extracting page #" + std::to_string(page_number) + " from " + filename);

				std::unique_ptr<poppler::page> page(doc->create_page(page_number));
				if (page == nullptr)
				{
					continue;
				}

				#if POPPLER_VERSION_MAJOR > 0 || (POPPLER_VERSION_MAJOR == 0 && POPPLER_VERSION_MINOR > 62)
				/* I don't know when these calls were introduced, but:
				 *
				 * - Ubuntu 18.04 uses Poppler 0.62.0
				 * - Ubuntu 20.04 uses Poppler 0.86.0
				 */
				renderer.set_image_format(poppler::image::format_enum::format_bgr24);
				renderer.set_line_mode(poppler::page_renderer::line_mode_enum::line_default); // is the default the same as "none"?
				#endif
				renderer.set_render_hint(poppler::page_renderer::render_hint::antialiasing		, true);
				renderer.set_render_hint(poppler::page_renderer::render_hint::text_antialiasing	, true);
				renderer.set_render_hint(poppler::page_renderer::render_hint::text_hinting		, true);
				renderer.set_paper_color(0xffffffff); // opaque white

				poppler::image image = renderer.render_page(page.get(), dpi, dpi);
				if (image.is_valid() == false)
				{
					// something has gone wrong
					Log("received an empty image while extracting page #" + std::to_string(page_number) + " from " + filename);
					continue;
				}

				#if (POPPLER_VERSION_MAJOR == 0 && POPPLER_VERSION_MINOR <= 62)
				/* Looks like the old versions of Poppler used 32-bit BGRA as the image
				 * format.  Though I'm not sure what version we need to use as a check,
				 * I'll use 0.62 for now until I know better.  This number will need to
				 * be tweaked.  But with these old versions of Poppler, we need to drop
				 * the alpha layer and just keep BGR.
				 */
				cv::Mat mat(image.height(), image.width(), CV_8UC4, image.data(), image.bytes_per_row());
				cv::cvtColor(mat, mat, cv::COLOR_BGRA2BGR);
				#else
				cv::Mat mat(image.height(), image.width(), CV_8UC3, image.data(), image.bytes_per_row());
				#endif

				if (resize_page and (mat.cols != new_width or mat.rows != new_height))
				{
					if (maintain_aspect_ratio)
					{
						mat = DarkHelp::resize_keeping_aspect_ratio(mat, {new_width, new_height});
					}
					else
					{
						cv::Mat dst;
						cv::resize(mat, dst, {new_width, new_height}, 0, 0,  CV_INTER_AREA);
						mat = dst;
					}
				}

				std::stringstream ss;
				ss << partial_output_filename << "_page_" << std::setfill('0') << std::setw(3) << (page_number + 1);

				if (save_as_png)
				{
					cv::imwrite(ss.str() + ".png", mat, {CV_IMWRITE_PNG_COMPRESSION, 1});
				}
				else if (save_as_jpg)
				{
					cv::imwrite(ss.str() + ".jpg", mat, {CV_IMWRITE_JPEG_QUALITY, jpg_quality});
				}

				number_of_imported_pages ++;
			}
		}
	}
	catch (const std::exception & e)
	{
		std::stringstream ss;
		ss	<< "An error was detected while processing the PDF file \"" + current_filename + "\":" << std::endl
			<< std::endl
			<< e.what();
		dm::Log(ss.str());
		error_shown = true;
		AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "DarkMark - Error!", ss.str());
	}
	catch (...)
	{
		const std::string msg = "An unknown error was encountered while processing the PDF file \"" + current_filename + "\".";
		dm::Log(msg);
		error_shown = true;
		AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "DarkMark - Error!", msg);
	}

	File dir(base_directory);
	dir.revealToUser();

	if (error_shown == false and threadShouldExit() == false)
	{
		Log("finished extracting " + std::to_string(number_of_imported_pages) + " pages");
		AlertWindow::showMessageBoxAsync(
			AlertWindow::AlertIconType::InfoIcon,
			"DarkMark",
			"Imported " +
			std::to_string(number_of_imported_pages	) + " page"		+ (number_of_imported_pages	== 1 ? "" : "s") + " from " +
			std::to_string(filenames.size()			) + " PDF file"	+ (filenames.size()			== 1 ? "" : "s") + "."
			);
	}

	return;
}
