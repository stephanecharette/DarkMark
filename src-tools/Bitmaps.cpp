// DarkMark (C) 2019-2023 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"
#include "Bitmaps.h"


Image dm::DarkMarkLogo()
{
	return ImageCache::getFromMemory(swirl_300x300_green_jpg, sizeof(swirl_300x300_green_jpg));
}


Image dm::convert_opencv_mat_to_juce_image(cv::Mat mat)
{
	// if the image has 1 channel we'll convert it to greyscale RGB
	// if the image has 3 channels (RGB) then all is good,
	// (anything else will cause this function to throw)
	cv::Mat source;
	switch (mat.channels())
	{
		case 1:
		{
			cv::cvtColor(mat, source, cv::COLOR_GRAY2BGR);
			break;
		}
		case 3:
		{
			source = mat;	// nothing to do, use the image as-is
			break;
		}
		default:
		{
			throw std::logic_error("cv::Mat has an unexpected number of channels (" + std::to_string(mat.channels()) + ")");
		}
	}

	// create a JUCE Image the exact same size as the OpenCV mat we're using as our source
	Image image(Image::RGB, source.cols, source.rows, false);

	// iterate through each row of the image, copying the entire row as a series of consecutive bytes
	const size_t number_of_bytes_to_copy = 3 * source.cols; // times 3 since each pixel contains 3 bytes (RGB)
	Image::BitmapData bitmap_data(image, 0, 0, source.cols, source.rows, Image::BitmapData::ReadWriteMode::writeOnly);
	for (int row_index = 0; row_index < source.rows; row_index ++)
	{
		uint8_t * src_ptr = source.ptr(row_index);
		uint8_t * dst_ptr = bitmap_data.getLinePointer(row_index);

		std::memcpy(dst_ptr, src_ptr, number_of_bytes_to_copy);
	}

	return image;
}


Image dm::AboutLogoWhiteBackground()
{
	return ImageCache::getFromMemory(ccr_darkmark_logo_white_background_png, sizeof(ccr_darkmark_logo_white_background_png));
}


Image dm::AboutLogoRedSwirl()
{
	return ImageCache::getFromMemory(ccr_darkmark_logo_red_swirl_png, sizeof(ccr_darkmark_logo_red_swirl_png));
}


Image dm::AboutLogoDarknet()
{
	return ImageCache::getFromMemory(ccr_darkmark_logo_darknet_png, sizeof(ccr_darkmark_logo_darknet_png));
}
