/* DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include "DarkMark.hpp"


dm::ProjectInfo::ProjectInfo(const std::string & prefix)
{
	// these settings are "global" (not project specific)
	cfg_prefix					= prefix;
	darknet_dir					= cfg().get_str		("darknet_dir"									, "");

	// every other setting is project-specific (with default values if the settings does not yet exist in configuration)
	cfg_template				= cfg().get_str		(cfg_prefix + "darknet_cfg_template"			, ""	);
	train_with_all_images		= cfg().get_bool	(cfg_prefix + "darknet_train_with_all_images"	, true	);
	training_images_percentage	= cfg().get_int		(cfg_prefix + "darknet_training_percentage"		, 80	) / 100.0;
	image_width					= cfg().get_int		(cfg_prefix + "darknet_image_width"				, 448	);
	image_height				= cfg().get_int		(cfg_prefix + "darknet_image_height"			, 256	);
	batch_size					= cfg().get_int		(cfg_prefix + "darknet_batch_size"				, 64	);
	subdivisions				= cfg().get_int		(cfg_prefix + "darknet_subdivisions"			, 8		);
	iterations					= cfg().get_int		(cfg_prefix + "darknet_iterations"				, 4000	);
	learning_rate				= cfg().get_double	(cfg_prefix + "darknet_learning_rate"			, 0.00261);
	max_chart_loss				= cfg().get_double	(cfg_prefix + "darknet_max_chart_loss"			, 4.0	);
	restart_training			= cfg().get_bool	(cfg_prefix + "darknet_restart_training"		, false	);
	delete_temp_weights			= cfg().get_bool	(cfg_prefix + "darknet_delete_temp_weights"		, true	);
	saturation					= cfg().get_double	(cfg_prefix + "darknet_saturation"				, 1.50	);
	exposure					= cfg().get_double	(cfg_prefix + "darknet_exposure"				, 1.50	);
	hue							= cfg().get_double	(cfg_prefix + "darknet_hue"						, 0.10	);
	enable_flip					= cfg().get_bool	(cfg_prefix + "darknet_enable_flip"				, true	);
	angle						= cfg().get_int		(cfg_prefix + "darknet_angle"					, 0		);
	enable_mosaic				= cfg().get_bool	(cfg_prefix + "darknet_mosaic"					, true	);
	enable_cutmix				= cfg().get_bool	(cfg_prefix + "darknet_cutmix"					, false	);
	enable_mixup				= cfg().get_bool	(cfg_prefix + "darknet_mixup"					, false	);

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
		cfg_template = File(darknet_dir).getChildFile("cfg").getChildFile("yolov4-tiny.cfg").getFullPathName().toStdString();
	}

	// check to make sure the template .cfg files actually exists
	if (not File(cfg_template).existsAsFile())
	{
		throw std::runtime_error("cannot find the darknet configuration template file " + cfg_template);
	}

	return *this;
}
