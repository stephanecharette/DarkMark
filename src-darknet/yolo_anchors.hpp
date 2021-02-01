// DarkMark (C) 2019-2021 Stephane Charette <stephanecharette@gmail.com>

#pragma once

/** The original @p calc_anchors() code from Darknet was taken from src/detector.c.  The code was then heavily modified
 * to bring it up to C++, remove memory leaks, cut out unneeded functionality, and remove all console output.  This new
 * function returns 3 values:  @p new_anchors, @p new_counters_per_class, and @p new_avg_iou.
 */
void calc_anchors(const std::string & train_images_filename, const size_t number_of_clusters, const size_t width, const size_t height, const size_t number_of_classes, std::string & new_anchors, std::string & new_counters_per_class, float & new_avg_iou);
