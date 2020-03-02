/* DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include "DarkMark.hpp"


dm::ProjectInfo::ProjectInfo(const std::string & project_directory)
{
	darknet_dir					= cfg().get_str	("darknet_dir"					);
	enable_yolov3_tiny			= cfg().get_bool("darknet_enable_yolov3_tiny"	);
	enable_yolov3_full			= cfg().get_bool("darknet_enable_yolov3_full"	);
	train_with_all_images		= cfg().get_bool("darknet_train_with_all_images");
	training_images_percentage	= cfg().get_int	("darknet_training_percentage"	) / 100.0;
	image_size					= cfg().get_int	("darknet_image_size"			);
	batch_size					= cfg().get_int	("darknet_batch_size"			);
	subdivisions				= cfg().get_int	("darknet_subdivisions"			);
	iterations					= cfg().get_int	("darknet_iterations"			);
	resume_training				= cfg().get_bool("darknet_resume_training"		);
	delete_temp_weights			= cfg().get_bool("darknet_delete_temp_weights"	);
	saturation					= cfg().get_double("darknet_saturation"			);
	exposure					= cfg().get_double("darknet_exposure"			);
	hue							= cfg().get_double("darknet_hue"				);
	enable_flip					= cfg().get_bool("darknet_enable_flip"			);
	angle						= cfg().get_int	("darknet_angle"				);
	enable_mosaic				= cfg().get_bool("darknet_mosaic"				);
	enable_cutmix				= cfg().get_bool("darknet_cutmix"				);
	enable_mixup				= cfg().get_bool("darknet_mixup"				);

	try
	{
		rebuild(project_directory);
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
	darknet_tiny_cfg_filename	= f.getChildFile(project_name + "_yolov3-tiny.cfg")	.getFullPathName().toStdString();
	darknet_full_cfg_filename	= f.getChildFile(project_name + "_yolov3-full.cfg")	.getFullPathName().toStdString();
	data_filename				= f.getChildFile(project_name + ".data")			.getFullPathName().toStdString();
	names_filename				= f.getChildFile(project_name + ".names")			.getFullPathName().toStdString();
	train_filename				= f.getChildFile(project_name + "_train.txt")		.getFullPathName().toStdString();
	valid_filename				= f.getChildFile(project_name + "_valid.txt")		.getFullPathName().toStdString();
	command_filename			= f.getChildFile(project_name + "_train.sh")		.getFullPathName().toStdString();

	darknet_tiny_cfg_template	= File(darknet_dir).getChildFile("cfg").getChildFile("yolov3-tiny.cfg"	).getFullPathName().toStdString();
	darknet_full_cfg_template	= File(darknet_dir).getChildFile("cfg").getChildFile("yolov3.cfg"		).getFullPathName().toStdString();

	// check to make sure the two YOLO template .cfg files are where we expect them to to be
	for (const auto & fn : {darknet_tiny_cfg_template, darknet_full_cfg_template})
	{
		if (not File(fn).existsAsFile())
		{
			throw std::runtime_error("cannot find the darknet YOLOv3 template file " + fn);
		}
	}

	return *this;
}
