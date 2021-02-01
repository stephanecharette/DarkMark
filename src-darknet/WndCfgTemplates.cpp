// DarkMark (C) 2019-2021 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"


/** Any timestamp prior to this will be considered "old"
 * - yolov2 is 2018-03
 * - yolov3 is 2018-05
 * - yolov4 is 2020-05
 */
#define OLD_CFG_TIMESTAMP "2020-01-00"

/** Only define this if you need to re-create the doxygen table.  It will
 * rebuild .dox file used on the web site.  This has no impact or purpose
 * normal users of DarkMark, and should be left undefined.
 */
#if 0
#define OUTPUT_DOX_TABLES
#endif


const auto table_header_names = {"name", "full path and name", "lines", "layers", "yolo layers", "network size", "weights size", "related", "last commit", "commit name", "notes", "links"};


dm::WndCfgTemplates::WndCfgTemplates(Value & v) :
		DocumentWindow("DarkMark v" DARKMARK_VERSION " Configuration Templates", Colours::darkgrey, TitleBarButtons::closeButton),
		Thread("configuration templates"),
		value(v),
		filters("filters", "Filters:"),
		status("status"),
		include_new_button("new"),
		include_yolo_button("yolo"),
		include_tiny_button("tiny"),
		help_button("Help"),
		ok_button("OK"),
		cancel_button("Cancel")
{
	setContentNonOwned(&canvas, false);

	canvas.addAndMakeVisible(filters			);
	canvas.addAndMakeVisible(include_new_button	);
	canvas.addAndMakeVisible(include_yolo_button);
	canvas.addAndMakeVisible(include_tiny_button);
	canvas.addAndMakeVisible(status				);
	canvas.addAndMakeVisible(table				);
	canvas.addAndMakeVisible(help_button		);
	canvas.addAndMakeVisible(ok_button			);
	canvas.addAndMakeVisible(cancel_button		);

	include_new_button	.setToggleState(true	, NotificationType::sendNotification);
	include_yolo_button	.setToggleState(true	, NotificationType::sendNotification);
	include_tiny_button	.setToggleState(false	, NotificationType::sendNotification);
	include_new_button	.setEnabled(false);
	include_yolo_button	.setEnabled(false);
	include_tiny_button	.setEnabled(false);

	include_new_button	.addListener(this);
	include_yolo_button	.addListener(this);
	include_tiny_button	.addListener(this);
	help_button			.addListener(this);
	ok_button			.addListener(this);
	cancel_button		.addListener(this);

	status.setJustificationType(Justification::centredRight);

	ok_button.setEnabled(false);

	Row row;
	row.field[Fields::kName] = "loading...";
	table_content_visible.push_back(row);

	int idx = 0;
	for (const auto & field : table_header_names)
	{
		idx ++;
		table.getHeader().addColumn(field, idx, 100, 30, -1, TableHeaderComponent::notSortable);
	}
	if (cfg().containsKey("CfgTemplatesTable"))
	{
		table.getHeader().restoreFromString(cfg().getValue("CfgTemplatesTable"));
	}
	else
	{
		table.getHeader().setStretchToFitActive(true);
	}
	table.getHeader().setPopupMenuActive(false);
	table.setModel(this);

	centreWithSize			(640, 480	);
	setUsingNativeTitleBar	(true		);
	setResizable			(true, true	);
	setDropShadowEnabled	(true		);

	selected_configuration_path_and_filename = value.toString().toStdString();

	setIcon(DarkMarkLogo());
	ComponentPeer *peer = getPeer();
	if (peer)
	{
		peer->setIcon(DarkMarkLogo());
	}

	if (cfg().containsKey("CfgTemplatesWnd"))
	{
		restoreWindowStateFromString(cfg().getValue("CfgTemplatesWnd"));
	}

	setVisible(true);

	startThread();

	return;
}


dm::WndCfgTemplates::~WndCfgTemplates()
{
	signalThreadShouldExit();

	cfg().setValue("CfgTemplatesWnd", getWindowStateAsString());
	cfg().setValue("CfgTemplatesTable", table.getHeader().toString());

	waitForThreadToExit(1000);

	return;
}


void dm::WndCfgTemplates::userTriedToCloseWindow()
{
	// ALT+F4

	exitModalState(-1);

	return;
}


void dm::WndCfgTemplates::resized()
{
	DocumentWindow::resized();

	const float margin_size = 5.0f;

	FlexBox filter_row;
	filter_row.flexDirection = FlexBox::Direction::row;
	filter_row.justifyContent = FlexBox::JustifyContent::flexStart;
	filter_row.items.add(FlexItem(filters).withWidth(100.0).withMargin(FlexItem::Margin(0, margin_size, 0, margin_size)));
	filter_row.items.add(FlexItem(include_new_button).withWidth(100.0));
	filter_row.items.add(FlexItem(include_yolo_button).withWidth(100.0));
	filter_row.items.add(FlexItem(include_tiny_button).withWidth(100.0));
	filter_row.items.add(FlexItem(status).withFlex(1.0));

	FlexBox button_row;
	button_row.flexDirection = FlexBox::Direction::row;
	button_row.justifyContent = FlexBox::JustifyContent::flexEnd;
	button_row.items.add(FlexItem(help_button).withWidth(100.0));
	button_row.items.add(FlexItem().withFlex(1.0));
	button_row.items.add(FlexItem(cancel_button).withWidth(100.0).withMargin(FlexItem::Margin(0, margin_size, 0, margin_size)));
	button_row.items.add(FlexItem(ok_button).withWidth(100.0));

	FlexBox fb;
	fb.flexDirection = FlexBox::Direction::column;
	fb.items.add(FlexItem(filter_row).withHeight(30.0).withMargin(FlexItem::Margin(0, 0, margin_size, 0)));
	fb.items.add(FlexItem(table).withFlex(1.0));
	fb.items.add(FlexItem(button_row).withHeight(30.0).withMargin(FlexItem::Margin(margin_size, 0, 0, 0)));

	auto r = getLocalBounds();
	r.reduce(margin_size, margin_size);
	fb.performLayout(r);

	return;
}


void dm::WndCfgTemplates::buttonClicked(Button * button)
{
	if (button == &help_button)
	{
		URL url("https://www.ccoderun.ca/DarkMark/Configuration.html");
		url.launchInDefaultBrowser();
	}
	else if (button == &cancel_button)
	{
		userTriedToCloseWindow();
	}
	else if (button == &ok_button)
	{
		value = selected_configuration_path_and_filename.c_str();
		exitModalState(0);
	}
	else if (button == &include_new_button or button == &include_yolo_button or button == &include_tiny_button)
	{
		table.deselectAllRows();
		filter_rows();
	}

	return;
}


std::string get_command_output(const std::string & cmd)
{
	std::string output;

	// loosely based on https://stackoverflow.com/questions/478898/how-do-i-execute-a-command-and-get-the-output-of-the-command-within-c-using-po
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
	if (pipe == nullptr)
	{
		throw std::runtime_error("file to open a pipe to run " + cmd);
	}

	while (true)
	{
		char buffer[200];
		const auto number_of_bytes = std::fread(buffer, 1, sizeof(buffer), pipe.get());
		if (number_of_bytes < 1)
		{
			break;
		}
		output += std::string(buffer, number_of_bytes);
	}

	return output;
}


void dm::WndCfgTemplates::run()
{
	File dir = File(cfg().getValue("darknet_dir")).getChildFile("cfg");
	auto files = dir.findChildFiles(File::TypesOfFileToFind::findFiles, true, "*.cfg");
	std::sort(files.begin(), files.end(),
			[](const File & lhs, const File & rhs)
			{
				// perform a case-insensitive comparison on the filename only (ignore the path)
				return lhs.getFileName().toUpperCase() < rhs.getFileName().toUpperCase();
			});

	for (const auto & file : files)
	{
		if (threadShouldExit())
		{
			break;
		}

		Row row;
		row.field[Fields::kName		] = file.getFileName()		.toStdString();
		row.field[Fields::kFullPath	] = file.getFullPathName()	.toStdString();

		StringArray lines;
		file.readLines(lines);
		size_t number_of_lines			= 0;
		size_t number_of_layers			= 0;
		size_t number_of_yolo_layers	= 0;
		size_t network_width			= 0;
		size_t network_height			= 0;
		bool look_for_width_and_height	= true;
		for (const auto line : lines)
		{
			if (threadShouldExit())
			{
				break;
			}
			number_of_lines ++;

			if (line.startsWith("["))
			{
//				Log(file.getFileName().toStdString() + " #" + std::to_string(number_of_lines) + ": " + line.toStdString());
				number_of_layers ++;
				if (line.startsWithIgnoreCase("[yolo"))
				{
					number_of_yolo_layers ++;
				}
			}
			else if (look_for_width_and_height)
			{
				if (line.startsWith("width="))
				{
					network_width = std::atoi(line.substring(6).toStdString().c_str());
				}
				else if (line.startsWith("height="))
				{
					network_height = std::atoi(line.substring(7).toStdString().c_str());
				}

				if (network_width > 0 and network_height > 0)
				{
					look_for_width_and_height = false;
				}
			}
		}
		row.field[Fields::kLines	] = std::to_string(number_of_lines);
		row.field[Fields::kLayers	] = std::to_string(number_of_layers - 1);

		if (number_of_yolo_layers)
		{
			row.field[Fields::kYoloLayers] = std::to_string(number_of_yolo_layers);
		}

		if (network_width > 0 and network_height > 0)
		{
			row.field[Fields::kNetworkSize] = std::to_string(network_width) + "x" + std::to_string(network_height);
		}

		table_content_all.push_back(row);
	}

	// get more details on each row in the table
	for (auto & row : table_content_all)
	{
		if (threadShouldExit())
		{
			break;
		}

		// now try to run the "git" command against each .cfg file to see if we can get some details on each one
		const std::string str = get_command_output("git -C \"" + dir.getFullPathName().toStdString() + "\" log -1 --pretty=\"format:%ci / %cn\" \"" + row.field[Fields::kFullPath] + "\"");
		if (str.size() >= 10)
		{
			row.field[Fields::kLastModified] = str.substr(0, 10);
		}
		const auto pos = str.find(" / ");
		if (pos != std::string::npos)
		{
			row.field[Fields::kLastCommitName] = str.substr(pos + 3);
		}

		const std::string & short_name = row.field[Fields::kName];
		if (short_name == "cd53paspp-gamma.cfg")
		{
			row.field[Fields::kRelated		] = "yolov4.cfg";
			row.field[Fields::kNotes		] = "detector, the same as yolov4.cfg, but with leaky instead of mish";
		}
		else if (short_name == "csdarknet53-omega.cfg")
		{
			row.field[Fields::kRelated		] = "yolov4.cfg";
			row.field[Fields::kNotes		] = "classifier, backbone for yolov4.cfg";
		}
		else if (short_name == "crnn.train.cfg")
		{
			row.field[Fields::kRelated		] = "rnn.train.cfg";
			row.field[Fields::kLink1		] = "1624";
			row.field[Fields::kUrl1			] = "https://github.com/AlexeyAB/darknet/issues/1624";
		}
		else if (short_name == "cspx-p7-mish-omega.cfg")
		{
			row.field[Fields::kRelated		] = "cspx-p7-mish.cfg";
			row.field[Fields::kNotes		] = "classifier, backbone for cspx-p7-mish.cfg";
		}
		else if (short_name == "cspx-p7-mish.cfg")
		{
			row.field[Fields::kNotes		] = "detector, yolov4-p7-large";
		}
		else if (short_name == "cspx-p7-mish_hp.cfg")
		{
			row.field[Fields::kNotes		] = "detector, experimental cfg file";
		}
		else if (short_name == "csresnext50-panet-spp.cfg")
		{
			row.field[Fields::kLink1		] = "2859 (SPP)";
			row.field[Fields::kUrl1			] = "https://github.com/AlexeyAB/darknet/issues/2859";
		}
		else if (short_name == "csresnext50-panet-spp-original-optimal.cfg")
		{
			row.field[Fields::kWeightsSize	] = "215.9 MiB";
			row.field[Fields::kRelated		] = "csresnext50-panet-spp.cfg";
			row.field[Fields::kNotes		] = "\"the best model for detection\"";
			row.field[Fields::kLink1		] = "5092";
			row.field[Fields::kUrl1			] = "https://github.com/AlexeyAB/darknet/issues/5092#issuecomment-602553795";
			row.field[Fields::kLink2		] = "2859 (SPP)";
			row.field[Fields::kUrl2			] = "https://github.com/AlexeyAB/darknet/issues/2859";
		}
		else if (short_name == "darknet53_448_xnor.cfg")
		{
			row.field[Fields::kNotes		] = "\"for Classification rather than Detection\"";
			row.field[Fields::kLink1		] = "5092";
			row.field[Fields::kUrl1			] = "https://github.com/AlexeyAB/darknet/issues/5092#issuecomment-602553795";
		}
		else if (short_name == "efficientnet_b0.cfg")
		{
			row.field[Fields::kNotes		] = "\"for Classification rather than Detection\"";
			row.field[Fields::kLink1		] = "5092";
			row.field[Fields::kUrl1			] = "https://github.com/AlexeyAB/darknet/issues/5092#issuecomment-602553795";
			row.field[Fields::kLink2		] = "3380";
			row.field[Fields::kUrl2			] = "https://github.com/AlexeyAB/darknet/issues/3380";
		}
		else if (short_name == "efficientnet-lite3.cfg")
		{
			row.field[Fields::kNotes		] = "classifier, EfficientNet-lite3";
			row.field[Fields::kLink1		] = "3380";
			row.field[Fields::kUrl1			] = "https://github.com/AlexeyAB/darknet/issues/3380";
		}
		else if (short_name == "enet-coco.cfg")
		{
			row.field[Fields::kNotes		] = "\"partial residual network\"";
			row.field[Fields::kLink1		] = "Enriching Variety of Layer-wise Learning Information by Gradient Combination";
			row.field[Fields::kUrl1			] = "https://github.com/WongKinYiu/PartialResidualNetworks";
		}
		else if (short_name == "Gaussian_yolov3_BDD.cfg")
		{
			row.field[Fields::kNotes		] = "\"models are experimental\"";
			row.field[Fields::kLink1		] = "5092";
			row.field[Fields::kUrl1			] = "https://github.com/AlexeyAB/darknet/issues/5092#issuecomment-602553795";
		}
		else if (short_name == "lstm.train.cfg")
		{
			row.field[Fields::kNotes		] = "\"long short-term memory\"";
			row.field[Fields::kLink1		] = "3114";
			row.field[Fields::kUrl1			] = "https://github.com/AlexeyAB/darknet/issues/3114";
		}
		else if (short_name == "resnet101.cfg")
		{
			row.field[Fields::kNotes		] = "\"for Classification rather than Detection\"";
			row.field[Fields::kLink1		] = "5092";
			row.field[Fields::kUrl1			] = "https://github.com/AlexeyAB/darknet/issues/5092#issuecomment-602553795";
		}
		else if (short_name == "resnet152.cfg")
		{
			row.field[Fields::kNotes		] = "\"for Classification rather than Detection\"";
			row.field[Fields::kLink1		] = "5092";
			row.field[Fields::kUrl1			] = "https://github.com/AlexeyAB/darknet/issues/5092#issuecomment-602553795";
		}
		else if (short_name == "resnet152_trident.cfg")
		{
			row.field[Fields::kNotes		] = "\"models are experimental\"";
			row.field[Fields::kLink1		] = "5092";
			row.field[Fields::kUrl1			] = "https://github.com/AlexeyAB/darknet/issues/5092#issuecomment-602553795";
		}
		else if (short_name == "rnn.train.cfg")
		{
			row.field[Fields::kRelated		] = "crnn.train.cfg";
			row.field[Fields::kLink1		] = "1624";
			row.field[Fields::kUrl1			] = "https://github.com/AlexeyAB/darknet/issues/1624";
		}
		else if (short_name == "yolov3_5l.cfg")
		{
			row.field[Fields::kWeightsSize	] = "236.2 MiB";
			row.field[Fields::kRelated		] = "yolov3.cfg";
			row.field[Fields::kNotes		] = "\"5l\" refers to 5 YOLO layers; \"...or very small objects, or if you want to set high network resolution\"";
			row.field[Fields::kLink1		] = "5092";
			row.field[Fields::kUrl1			] = "https://github.com/AlexeyAB/darknet/issues/5092#issuecomment-602553795";
		}
		else if (short_name == "yolov3.cfg")
		{
			row.field[Fields::kWeightsSize	] = "234.9 MiB";
			row.field[Fields::kNotes		] = "contains 3 YOLO layers";
		}
		else if (short_name == "yolov3.coco-giou-12.cfg")
		{
			row.field[Fields::kRelated		] = "yolov3.cfg";
			row.field[Fields::kNotes		] = "\"models are experimental\"";
			row.field[Fields::kLink1		] = "5092";
			row.field[Fields::kUrl1			] = "https://github.com/AlexeyAB/darknet/issues/5092#issuecomment-602553795";
			row.field[Fields::kLink2		] = "Generalized Intersection over Union (GIoU)";
			row.field[Fields::kUrl2			] = "https://giou.stanford.edu/";
		}
		else if (short_name == "yolov3-openimages.cfg")
		{
			row.field[Fields::kRelated		] = "yolov3.cfg";
			row.field[Fields::kNotes		] = "YOLOv3 but already setup for 601 classes";
			row.field[Fields::kLink1		] = "OpenImage Dataset";
			row.field[Fields::kUrl1			] = "https://github.com/openimages/dataset";
		}
		else if (short_name == "yolov3-spp.cfg")
		{
			row.field[Fields::kRelated		] = "yolov3.cfg";
			row.field[Fields::kNotes		] = "YOLOv3 but with extra layers for spacial pyramid pooling";
			row.field[Fields::kLink1		] = "2859 (SPP)";
			row.field[Fields::kUrl1			] = "https://github.com/AlexeyAB/darknet/issues/2859";
		}
		else if (short_name == "yolov3-tiny_3l.cfg")
		{
			row.field[Fields::kWeightsSize	] = "34.4 MiB";
			row.field[Fields::kRelated		] = "yolov3-tiny.cfg";
			row.field[Fields::kNotes		] = "better at finding small objects; \"3l\" refers to 3 YOLO layers vs the usual 2 in \"tiny\"";
		}
		else if (short_name == "yolov3-tiny.cfg")
		{
			row.field[Fields::kWeightsSize	] = "33.1 MiB";
			row.field[Fields::kNotes		] = "contains 2 YOLO layers";
		}
		else if (short_name == "yolov3-tiny_obj.cfg")
		{
			row.field[Fields::kWeightsSize	] = "33.1 MiB";
			row.field[Fields::kRelated		] = "yolov3-tiny.cfg";
			row.field[Fields::kNotes		] = "Exactly the same as yolov3-tiny.cfg.";
		}
		else if (short_name == "yolov3-tiny_occlusion_track.cfg")
		{
			row.field[Fields::kRelated		] = "yolov3-tiny.cfg";
			row.field[Fields::kNotes		] = "\"object Detection & Tracking using conv-rnn layer on frames from video\"";
			row.field[Fields::kLink1		] = "2553";
			row.field[Fields::kUrl1			] = "https://github.com/AlexeyAB/darknet/issues/2553";
		}
		else if (short_name == "yolov3-tiny-prn.cfg")
		{
			row.field[Fields::kRelated		] = "yolov3-tiny.cfg";
			row.field[Fields::kNotes		] = "\"partial residual network\"";
			row.field[Fields::kLink1		] = "Enriching Variety of Layer-wise Learning Information by Gradient Combination";
			row.field[Fields::kUrl1			] = "https://github.com/WongKinYiu/PartialResidualNetworks";
		}
		else if (short_name == "yolov3-tiny_xnor.cfg")
		{
			row.field[Fields::kRelated		] = "yolov3-tiny.cfg";
			row.field[Fields::kNotes		] = "\"XNOR-net ~2x faster than cuDNN on CUDA\"";
			row.field[Fields::kLink1		] = "3054";
			row.field[Fields::kUrl1			] = "https://github.com/AlexeyAB/darknet/issues/3054";
		}
		else if (short_name == "yolov3-voc.cfg")
		{
			row.field[Fields::kRelated		] = "yolov3.cfg";
			row.field[Fields::kNotes		] = "similar to the usual yolov3.cfg but pre-configured for 20 VOC classes";
		}
		else if (short_name == "yolov3-voc.yolov3-giou-40.cfg")
		{
			row.field[Fields::kRelated		] = "yolov3-voc.cfg";
			row.field[Fields::kNotes		] = "\"models are experimental\"";
			row.field[Fields::kLink1		] = "5092";
			row.field[Fields::kUrl1			] = "https://github.com/AlexeyAB/darknet/issues/5092#issuecomment-602553795";
			row.field[Fields::kLink2		] = "Generalized Intersection over Union (GIoU)";
			row.field[Fields::kUrl2			] = "https://giou.stanford.edu/";
		}
		else if (short_name == "yolov4.cfg")
		{
			row.field[Fields::kNotes		] = "contains 3 YOLO layers";
			row.field[Fields::kWeightsSize	] = "245.7 MiB";
			row.field[Fields::kLink1		] = "YOLOv4 whitepaper";
			row.field[Fields::kUrl1			] = "https://arxiv.org/pdf/2004.10934.pdf";
		}
		else if (short_name == "yolov4-custom.cfg")
		{
			row.field[Fields::kRelated		] = "yolov4.cfg";
			row.field[Fields::kWeightsSize	] = "245.7 MiB";
			row.field[Fields::kLink1		] = "YOLOv4 whitepaper";
			row.field[Fields::kUrl1			] = "https://arxiv.org/pdf/2004.10934.pdf";
			row.field[Fields::kNotes		] = "nearly identical; lower learning rate; one additional change to a convolutional layer";
		}
		else if (short_name == "yolov4-tiny-3l.cfg")
		{
			row.field[Fields::kWeightsSize	] = "23.3 MiB";
			row.field[Fields::kRelated		] = "yolov4-tiny.cfg";
			row.field[Fields::kNotes		] = "better at finding small objects; \"3l\" refers to 3 YOLO layers vs the usual 2 in \"tiny\"";
		}
		else if (short_name == "yolov4-tiny.cfg")
		{
			row.field[Fields::kNotes		] = "contains 2 YOLO layers";
			row.field[Fields::kWeightsSize	] = "23.1 MiB";
			row.field[Fields::kLink1		] = "5346";
			row.field[Fields::kUrl1			] = "https://github.com/AlexeyAB/darknet/issues/5346#issuecomment-649566598";
		}
		else if (short_name == "yolov4-tiny-custom.cfg")
		{
			row.field[Fields::kNotes		] = "similar to yolov4-tiny.cfg, but contains 1 minor change to the first YOLO layer";
			row.field[Fields::kRelated		] = "yolov4-tiny.cfg";
			row.field[Fields::kWeightsSize	] = "23.1 MiB";
			row.field[Fields::kLink1		] = "5346";
			row.field[Fields::kUrl1			] = "https://github.com/AlexeyAB/darknet/issues/5346#issuecomment-649566598";
		}
		else if (short_name == "yolov4-tiny_contrastive.cfg")
		{
			row.field[Fields::kNotes		] = "\"experimental\"; \"suitable for un-supervised learning and for multi-camera object tracking\"";
			row.field[Fields::kRelated		] = "yolov4-tiny.cfg";
			row.field[Fields::kWeightsSize	] = "27.5 MiB";
			row.field[Fields::kLink1		] = "6892";
			row.field[Fields::kUrl1			] = "https://github.com/AlexeyAB/darknet/issues/6892";
		}
		else if (short_name == "yolov4-csp.cfg")
		{
			row.field[Fields::kNotes		] = "cross-stage-partial; more accurate and faster than YOLOv4";
			row.field[Fields::kRelated		] = "yolov4.cfg";
			row.field[Fields::kWeightsSize	] = "202.1 MiB";
			row.field[Fields::kLink1		] = "Scaled YOLO v4";
			row.field[Fields::kUrl1			] = "https://alexeyab84.medium.com/scaled-yolo-v4-is-the-best-neural-network-for-object-detection-on-ms-coco-dataset-39dfa22fa982";
			row.field[Fields::kLink2		] = "YOLOv4-CSP whitepaper";
			row.field[Fields::kUrl2			] = "https://arxiv.org/pdf/2011.08036.pdf";
		}
		else if (short_name == "yolov4x-mish.cfg")
		{
			row.field[Fields::kNotes		] = "detector; something between yolov4-csp and yolov4-p5; more suitable for high resolutions 640x640 - 832x832 than yolov4.cfg; should be trained longer";
			row.field[Fields::kRelated		] = "yolov4-csp.cfg";
			row.field[Fields::kWeightsSize	] = "380.9 MiB";
			row.field[Fields::kLink1		] = "7131";
			row.field[Fields::kUrl1			] = "https://github.com/AlexeyAB/darknet/issues/7131";
		}
		else if (
			short_name == "cifar.cfg"				or
			short_name == "darknet53.cfg"			or
			short_name == "densenet201.cfg"			or
			short_name == "resnet50.cfg"			or
			short_name == "resnext152-32x4d.cfg"	or
			short_name == "strided.cfg"				or
			short_name == "vgg-16.cfg"				or
			short_name == "vgg-conv.cfg"			)
		{
			row.field[Fields::kNotes		] = "\"for Classification rather than Detection\"";
			row.field[Fields::kLink1		] = "5092";
			row.field[Fields::kUrl1			] = "https://github.com/AlexeyAB/darknet/issues/5092#issuecomment-602553795";
		}
	}

	for (auto & row : table_content_all)
	{
		if (threadShouldExit())
		{
			break;
		}

		const auto & timestamp = row.field[Fields::kLastModified];
		if (timestamp.empty() == false and timestamp < OLD_CFG_TIMESTAMP)
		{
			row.is_old = true;
			if (row.field[Fields::kNotes].empty())
			{
				row.field[Fields::kNotes] = "old cfg";
			}
		}
	}

	#ifdef OUTPUT_DOX_TABLES
	std::ofstream ofs_new("new_cfg_table.txt");
	std::ofstream ofs_old("old_cfg_table.txt");
	size_t field_number = kName;
	for (const auto & field : table_header_names)
	{
		if (field_number != kFullPath)
		{
			if (field_number > kName)
			{
				ofs_new << " | ";
				ofs_old << " | ";
			}
			ofs_new << field;
			ofs_old << field;
		}
		field_number ++;
	}
	ofs_new << std::endl;
	ofs_old << std::endl;

	for (field_number = 0; field_number < table_header_names.size() - 1; field_number ++)
	{
		if (field_number > 0)
		{
			ofs_new << "|";
			ofs_old << "|";
		}
		ofs_new << "---";
		ofs_old << "---";
	}
	ofs_new << std::endl;
	ofs_old << std::endl;

	for (const auto & row : table_content_all)
	{
		const auto & timestamp = row.field[Fields::kLastModified];
		auto & ofs = (timestamp < OLD_CFG_TIMESTAMP ? ofs_old : ofs_new);

		for (field_number = kName; field_number < kNumberOfCols; field_number ++)
		{
			if (field_number == kFullPath)
			{
				continue;
			}

			if (field_number > kName)
			{
				ofs << " | ";
			}

			if (field_number < kLink1)
			{
				if (field_number == kName or
					(field_number == kRelated and row.field[field_number].empty() == false))
				{
					ofs << "@p ";
				}

				ofs << row.field[field_number];
			}
			else
			{
				// combine the link1, url1, link2, url2 fields into a single field
				field_number += 3;
				bool need_comma = false;

				if (row.field[kUrl1].empty() == false and row.field[kLink1].empty() == false)
				{
					ofs << "<a target=\"_blank\" href=\"" << row.field[kUrl1] << "\">" << row.field[kLink1] << "</a>";
					need_comma = true;
				}

				if (row.field[kUrl2].empty() == false and row.field[kLink2].empty() == false)
				{
					if (need_comma)
					{
						ofs << ", ";
					}
					ofs << "<a target=\"_blank\" href=\"" << row.field[kUrl2] << "\">" << row.field[kLink2] << "</a>";
					need_comma = true;
				}

				if (need_comma == false)
				{
					// doxygen doesn't behave well when the last field in a table is empty
					ofs << "&nbsp;";
				}
			}
		}
		ofs << std::endl;
	}
	#endif

	include_new_button	.setEnabled(true);
	include_yolo_button	.setEnabled(true);
	include_tiny_button	.setEnabled(true);

	filter_rows();

	return;
}


int dm::WndCfgTemplates::getNumRows()
{
	return table_content_visible.size();
}


void dm::WndCfgTemplates::paintCell(Graphics & g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
	if (rowNumber < 0				or
		rowNumber >= getNumRows()	or
		columnId < 1				or
		columnId > Fields::kLink1	)
	{
		// rows are 0-based, columns are 1-based
		return;
	}

	const Row & row = table_content_visible.at(rowNumber);

	Colour colour = Colours::black;
	std::string str = row.field[columnId];
	if (columnId >= Fields::kLink1)
	{
		// special handling required for kLink1, kLink2, kUrl1, and kUrl2
		str = row.field[Fields::kLink1];
		if (str.empty() == false and row.field[Fields::kLink2].empty() == false)
		{
			str += ", ";
		}
		str += row.field[Fields::kLink2];
		if (str.empty() == false)
		{
			colour = Colours::blue;
		}
	}

	// draw the text and the right-hand-side dividing line between cells
	g.setColour(colour);
	Rectangle<int> r(0, 0, width, height);
	g.drawFittedText(str, r.reduced(2), Justification::centredLeft, 1);

	// draw the divider on the right side of the column
	g.setColour(Colours::black.withAlpha(0.5f));
	g.drawLine(width, 0, width, height);

	return;
}


void dm::WndCfgTemplates::paintRowBackground(Graphics & g, int rowNumber, int width, int height, bool rowIsSelected)
{
	Colour colour = Colours::white;

	if (rowNumber >= 0				and
		rowNumber < getNumRows()	)
	{
		const Row & row = table_content_visible.at(rowNumber);
		const auto & last_modified = row.field[Fields::kLastModified];
		if (last_modified.empty())
		{
			colour = Colours::yellow;
		}
		else if (last_modified < OLD_CFG_TIMESTAMP)
		{
			colour = Colours::lightpink.interpolatedWith(Colours::white, 0.7f);
		}
	}

	if (rowIsSelected)
	{
		colour = Colours::lightblue; // selected rows will have a blue background
	}
	g.fillAll(colour);

	// draw the cell bottom divider between rows
	g.setColour(Colours::black.withAlpha(0.9f));
	g.drawLine(0, height, width, height);

	return;
}


void dm::WndCfgTemplates::selectedRowsChanged(int lastRowSelected)
{
	bool is_valid = false;
	if (lastRowSelected >= 0 and lastRowSelected < getNumRows())
	{
		// this should help exclude the fake "loading..." row
		const std::string & filename = table_content_visible.at(lastRowSelected).field[kFullPath];
		if (File(filename).existsAsFile())
		{
			selected_configuration_path_and_filename = filename;
			is_valid = true;
		}
	}

	ok_button.setEnabled(is_valid);

	return;
}


void dm::WndCfgTemplates::cellClicked(int rowNumber, int columnId, const MouseEvent & event)
{
	if (rowNumber >= 0				and
		rowNumber < getNumRows()	and
		columnId == Fields::kLink1	)
	{
		const Row & row = table_content_visible.at(rowNumber);

		if (row.field[Fields::kUrl1].empty() == false)
		{
			URL(row.field[Fields::kUrl1]).launchInDefaultBrowser();
		}
		if (row.field[Fields::kUrl2].empty() == false)
		{
			URL(row.field[Fields::kUrl2]).launchInDefaultBrowser();
		}
	}

	return;
}


void dm::WndCfgTemplates::filter_rows()
{
	const bool only_include_new		= include_new_button	.getToggleState();
	const bool only_include_yolo	= include_yolo_button	.getToggleState();
	const bool only_include_tiny	= include_tiny_button	.getToggleState();

	std::vector<Row> v;
	for (const auto & row : table_content_all)
	{
		if (only_include_new and row.is_old)
		{
			continue;
		}

		if (only_include_yolo)
		{
			// the old V1 and V2 configuration  files for Yolo don't actually have a [yolo] section,
			// so also look at the filenames to make sure the yolo filter includes the old files
			if (row.field[Fields::kYoloLayers].empty() and
				row.field[Fields::kName].find("yolo") == std::string::npos)
			{
				continue;
			}
		}

		if (only_include_tiny and row.field[Fields::kName].find("tiny") == std::string::npos)
		{
			continue;
		}

		// otherwise if we get here then show this row to the user
		v.push_back(row);
	}

	table_content_visible.swap(v);
	table.updateContent();

	status.setText("showing " + std::to_string(table_content_visible.size()) + " of " + std::to_string(table_content_all.size()) + " configuration files", NotificationType::sendNotification);

	// see if the currently selected name is included in the table, and if so then we select it
	if (selected_configuration_path_and_filename.empty() == false)
	{
		for (size_t idx = 0; idx < table_content_visible.size(); idx ++)
		{
			if (table_content_visible.at(idx).field[Fields::kFullPath] == selected_configuration_path_and_filename)
			{
				table.selectRow(idx);
				break;
			}
		}
	}

	return;
}
