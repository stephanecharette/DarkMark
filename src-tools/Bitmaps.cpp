/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include "DarkMark.hpp"
#include "Bitmaps.h"


Image dm::DarkMarkLogo()
{
	return ImageCache::getFromMemory(swirl_300x300_green_jpg, sizeof(swirl_300x300_green_jpg));
}
