/* DarkMark (C) 2019-2020 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	/// A collection of filenames, paths, and various configuration settings needed to run DarkMark and darknet.
	class ProjectInfo final
	{
		public:

			std::string cfg_prefix;					///< The text that needs to be prefixed to configuration items, such as @p "project_12345_".
			std::string project_name;				///< e.g., @p flowers
			std::string	darknet_dir;				///< e.g., @p /home/bob/darknet
			std::string	project_dir;				///< e.g., @p /home/bob/nn/flowers
			std::string data_filename;				///< e.g., @p /home/bob/nn/flowers/flowers.data
			std::string names_filename;				///< e.g., @p /home/bob/nn/flowers/flowers.names
			std::string train_filename;				///< e.g., @p /home/bob/nn/flowers/flowers_train.txt
			std::string valid_filename;				///< e.g., @p /home/bob/nn/flowers/flowers_valid.txt
			std::string command_filename;			///< e.g., @p /home/bob/nn/flowers/flowers_train.sh
			std::string cfg_filename;				///< e.g., @p /home/bob/nn/flowers/flowers.cfg
			std::string	cfg_template;				///< e.g., @p /home/bob/darknet/cfg/yolov3-tiny.cfg

			bool		train_with_all_images;		///< should we train with *all* images, or should we use @ref training_images_percentage?
			double		training_images_percentage;	///< between 0.0 and 1.0
			int			image_size;					///< must be a multiple of 32 (416, 608, 832, ...)
			int			batch_size;					///< e.g., @p 64
			int			subdivisions;				///< e.g., @p 8
			int			iterations;					///< number of iterations; e.g., @p 20,000
			bool		resume_training;			///< whether training should start from previous *_best.weights file or start new
			bool		delete_temp_weights;		///< whether the temporary .weights files should be deleted once training has finished
			float		saturation;
			float		exposure;
			float		hue;						///< between 0.0 and 1.0
			bool		enable_flip;				///< when @p TRUE, flip will be set to @p 1, otherwise flip will be set to zero
			int			angle;						///< rotation angle, or zero to disable
			bool		enable_mosaic;
			bool		enable_cutmix;
			bool		enable_mixup;

			ProjectInfo(const std::string & prefix);

			/** Rebuild all the paths, but without changing the project name/dir.  For example, perhaps the location of the darknet
			 * directory was changed, which would require some of the template paths to be updated.
			 */
			ProjectInfo & rebuild();

			/** Rebuild all of the filenames and paths based on the given project directory.  For example, if the project
			 * parameter passed in is @p "/home/bob/nn/flowers/" then the project name will be set to "flowers" and all of the
			 * paths will be similar to @p /home/bob/nn/flowers/flowers*.
			 */
			ProjectInfo & rebuild(const std::string & project_directory);
	};
}
