// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#include "DarkMark.hpp"
#include "Bitmaps.h"


Image dm::DarkMarkLogo()
{
	return ImageCache::getFromMemory(swirl_300x300_green_jpg, sizeof(swirl_300x300_green_jpg));
}


Image dm::convert_opencv_mat_to_juce_image(const cv::Mat & mat)
{
	if (mat.empty())
	{
		return {};
	}

	const int width  = mat.cols;
	const int height = mat.rows;

	// Image::RGB is usually 32-bit (0xXXRRGGBB) but ignores the alpha channel during rendering
	// skip zero init because it will overwritten
	Image image(Image::RGB, width, height, false);

	// lock the underlying pixel data for writing
	Image::BitmapData dest(image, Image::BitmapData::writeOnly);

	// wrap JUCE memory in an OpenCV Mat header
	const int destType = (dest.pixelStride == 4) ? CV_8UC4 : CV_8UC3;

	cv::Mat juceWrapper(height, width, destType, dest.getLinePointer(0), dest.lineStride);

	// perform conversion directly from Source to Destination
	// OpenCV handles the channel shuffling and padding logic efficiently
	if (destType == CV_8UC4)
	{
		switch (mat.channels())
		{
			case 3:
			{
				// most common case: BGR -> BGRA (JUCE RGB is usually stored as BGRA on LE)
				cv::cvtColor(mat, juceWrapper, cv::COLOR_BGR2BGRA);
				break;
			}
			case 4:
			{
				// exact match: fast memory copy (may handle stride differences automatically)
				mat.copyTo(juceWrapper);
				break;
			}
			case 1:
			{
				// Grayscale -> BGRA
				cv::cvtColor(mat, juceWrapper, cv::COLOR_GRAY2BGRA);
				break;
			}
			default:
			{
				throw std::logic_error("cv::Mat has an unexpected number of channels (" + std::to_string(mat.channels()) + ")");
			}
		}
	}
	else
	{
		// rare 24-bit packed format (pixelStride == 3)

		if (mat.channels() == 3)
		{
			cv::cvtColor(mat, juceWrapper, cv::COLOR_BGR2RGB);
		}
		else if (mat.channels() == 1)
		{
			cv::cvtColor(mat, juceWrapper, cv::COLOR_GRAY2RGB);
		}
		else
		{
			throw std::logic_error("cv::Mat has an unexpected number of channels (" + std::to_string(mat.channels()) + ")");
		}
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
