// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"


int toInt(const std::string & str)
{
	int i = 0;
	try
	{
		i = std::stoi(str);
	}
	catch (...)
	{
	}

	return i;
}


float toFloat(const std::string & str)
{
	float f = 0.0f;
	try
	{
		f = std::stof(str);
	}
	catch (...)
	{
	}

	return f;
}


bool toBool(const std::string & str)
{
	if (str == "true"	||
		str == "TRUE"	||
		str == "yes"	||
		str == "YES"	||
		str == "on"		||
		str == "ON"		||
		str == "1"		)
	{
		return true;
	}

	return false;
}


dm::ProjectInfo::ProjectInfo(const std::string & prefix)
{
	// these settings are "global" (not project specific)
	cfg_prefix					= prefix;
//	darknet_dir					= cfg().get_str		("darknet_dir"									, "");

	// every other setting is project-specific (with default values if the settings does not yet exist in configuration)
	cfg_template				= cfg().get_str		(cfg_prefix + "darknet_cfg_template"			, ""	);
	extra_flags					= cfg().get_str		(cfg_prefix + "darknet_extra_flags"				, "-dont_show -map -gpus 0");
	train_with_all_images		= cfg().get_bool	(cfg_prefix + "darknet_train_with_all_images"	, true	);
	training_images_percentage	= cfg().get_int		(cfg_prefix + "darknet_training_percentage"		, 80	) / 100.0;
	limit_validation_images		= cfg().get_bool	(cfg_prefix + "darknet_limit_validation_images"	, false	);
	image_width					= cfg().get_int		(cfg_prefix + "darknet_image_width"				, 352	);
	image_height				= cfg().get_int		(cfg_prefix + "darknet_image_height"			, 256	);
	batch_size					= cfg().get_int		(cfg_prefix + "darknet_batch_size"				, 64	);
	subdivisions				= cfg().get_int		(cfg_prefix + "darknet_subdivisions"			, 1		);
	iterations					= cfg().get_int		(cfg_prefix + "darknet_iterations"				, 4000	);
	learning_rate				= cfg().get_double	(cfg_prefix + "darknet_learning_rate"			, 0.00261);
	max_chart_loss				= cfg().get_double	(cfg_prefix + "darknet_max_chart_loss"			, 4.0	);
	image_type					= cfg().get_str		(cfg_prefix + "darknet_image_type"				, "both");
	do_not_resize_images		= cfg().get_bool	(cfg_prefix + "darknet_do_not_resize_images"	, false	);
	resize_images				= cfg().get_bool	(cfg_prefix + "darknet_resize_images"			, true	);
	tile_images					= cfg().get_bool	(cfg_prefix + "darknet_tile_images"				, false	);
	zoom_images					= cfg().get_bool	(cfg_prefix + "darknet_zoom_images"				, true	);
	remove_small_annotations	= cfg().get_bool	(cfg_prefix + "darknet_remove_small_annotations", true	);
	annotation_area_size		= cfg().get_int		(cfg_prefix + "darknet_annotation_area_size"	, 64	);
	limit_negative_samples		= cfg().get_bool	(cfg_prefix + "darknet_limit_negative_samples"	, true	);
	recalculate_anchors			= cfg().get_bool	(cfg_prefix + "darknet_recalculate_anchors"		, true	);
	anchor_clusters				= cfg().get_int		(cfg_prefix + "darknet_anchor_clusters"			, 9		);
	class_imbalance				= cfg().get_bool	(cfg_prefix + "darknet_class_imbalance"			, false	);
	restart_training			= cfg().get_bool	(cfg_prefix + "darknet_restart_training"		, false	);
	delete_temp_weights			= cfg().get_bool	(cfg_prefix + "darknet_delete_temp_weights"		, false	);
	save_weights				= cfg().get_int		(cfg_prefix + "darknet_save_weights"			, 0		);
	saturation					= cfg().get_double	(cfg_prefix + "darknet_saturation"				, 1.50	);
	exposure					= cfg().get_double	(cfg_prefix + "darknet_exposure"				, 1.50	);
	hue							= cfg().get_double	(cfg_prefix + "darknet_hue"						, 0.10	);
	enable_flip					= cfg().get_bool	(cfg_prefix + "darknet_enable_flip"				, false	);
	angle						= cfg().get_int		(cfg_prefix + "darknet_angle"					, 0		);
	enable_mosaic				= cfg().get_bool	(cfg_prefix + "darknet_mosaic"					, false	);
	enable_cutmix				= cfg().get_bool	(cfg_prefix + "darknet_cutmix"					, false	);
	enable_mixup				= cfg().get_bool	(cfg_prefix + "darknet_mixup"					, false	);

	const auto & options = dmapp().cli_options;
	if (options.count("template"				))	cfg_template			= options.at("template");
	if (options.count("width"					))	image_width				= toInt(options.at("width"						));
	if (options.count("height"					))	image_height			= toInt(options.at("height"						));
	if (options.count("max_batches"				))	iterations				= toInt(options.at("max_batches"				));
	if (options.count("batch_size"				))	batch_size				= toInt(options.at("batch_size"					));
	if (options.count("subdivisions"			))	subdivisions			= toInt(options.at("subdivisions"				));
	if (options.count("do_not_resize_images"	))	do_not_resize_images	= toBool(options.at("do_not_resize_images"		));
	if (options.count("resize_images"			))	resize_images			= toBool(options.at("resize_images"				));
	if (options.count("tile_images"				))	tile_images				= toBool(options.at("tile_images"				));
	if (options.count("zoom_images"				))	zoom_images				= toBool(options.at("zoom_images"				));
	if (options.count("limit_neg_samples"		))	limit_negative_samples	= toBool(options.at("limit_neg_samples"			));
	if (options.count("limit_validation_images"	))	limit_validation_images	= toBool(options.at("limit_validation_images"	));
	if (options.count("yolo_anchors"			))	recalculate_anchors		= toBool(options.at("yolo_anchors"				));
	if (options.count("learning_rate"			))	learning_rate			= toFloat(options.at("learning_rate"			));
	if (options.count("class_imbalance"			))	class_imbalance			= toBool(options.at("class_imbalance"			));
	if (options.count("mosaic"					))	enable_mosaic			= toBool(options.at("mosaic"					));
	if (options.count("cutmix"					))	enable_cutmix			= toBool(options.at("cutmix"					));
	if (options.count("mixup"					))	enable_mixup			= toBool(options.at("mixup"						));
	if (options.count("flip"					))	enable_flip				= toBool(options.at("flip"						));
	if (options.count("restart_training"		))	restart_training		= toBool(options.at("restart_training"			));
	if (options.count("remove_small_annotations"))	remove_small_annotations= toBool(options.at("remove_small_annotations"	));
	if (options.count("annotation_area_size"	))	annotation_area_size	= toInt(options.at("annotation_area_size"		));

	if (image_type != "JPG" and
		image_type != "PNG")
	{
		image_type = "both";
	}

	if (options.count("resize_images") and toBool(options.at("resize_images")) == true)
	{
		do_not_resize_images = false;
	}
	else if (options.count("do_not_resize_images") and toBool(options.at("do_not_resize_images")) == true)
	{
		resize_images = false;
		remove_small_annotations = false;
	}

	// handle the special restrictions between the 3 "image resizing" modes since they're all related
	if (do_not_resize_images == false and resize_images == false and tile_images == false and zoom_images == false)
	{
		do_not_resize_images = true;
	}
	if (do_not_resize_images)
	{
		resize_images	= false;
		tile_images		= false;
		zoom_images		= false;
		remove_small_annotations = false;
	}

	try
	{
		rebuild(cfg().get_str(cfg_prefix + "dir"));
	}
	catch (const std::exception & e)
	{
		dm::Log("project info error: " + std::string(e.what()));
		AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon, "DarkMark", "Error setting up the project:\n\n" + std::string(e.what()));
	}

	return;
}


dm::ProjectInfo & dm::ProjectInfo::rebuild()
{
	return rebuild(project_dir);
}


dm::ProjectInfo & dm::ProjectInfo::rebuild(const std::string & project_directory)
{
	if (project_directory.empty())
	{
		throw std::invalid_argument("project directory cannot be empty");
	}

	File f(project_directory);

	if (not f.exists())
	{
		Result result = f.createDirectory();
		if (result.failed())
		{
			throw std::runtime_error(f.getFullPathName().toStdString() + ": " + result.getErrorMessage().toStdString());
		}
	}

	project_dir					= f													.getFullPathName().toStdString();
	project_name				= f										.getFileNameWithoutExtension().toStdString();
	data_filename				= f.getChildFile(project_name + ".data")			.getFullPathName().toStdString();
	names_filename				= f.getChildFile(project_name + ".names")			.getFullPathName().toStdString();
	train_filename				= f.getChildFile(project_name + "_train.txt")		.getFullPathName().toStdString();
	valid_filename				= f.getChildFile(project_name + "_valid.txt")		.getFullPathName().toStdString();
	command_filename			= f.getChildFile(project_name + "_train.sh")		.getFullPathName().toStdString();
	cfg_filename				= f.getChildFile(project_name + ".cfg")				.getFullPathName().toStdString();

	if (cfg_template.empty())
	{
		cfg_template = File(cfg().get_str("darknet_templates")).getChildFile("yolov4-tiny.cfg").getFullPathName().toStdString();
	}

	// check to make sure the template .cfg files actually exists
	if (not File(cfg_template).existsAsFile())
	{
		throw std::runtime_error("cannot find the darknet configuration template file " + cfg_template);
	}

	return *this;
}
