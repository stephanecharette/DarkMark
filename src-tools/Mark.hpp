/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#pragma once

#include "DarkMark.hpp"


namespace dm
{
	enum class ECorner
	{
		kTL,
		kTR,
		kBR,
		kBL
	};
	typedef std::vector<cv::Point>			VPoints;
	typedef std::vector<cv::Point2d>		VPoints2d;
	typedef std::map<ECorner, cv::Point2d>	MapCornerToPoint2d;
	typedef std::vector<Mark>				VMarks;

	class Mark final
	{
		public:

			/// Destructor
			~Mark();

			/// Empty constructor creates a mark at (0, 0, 0, 0)
			Mark();

			/// Darknet-style constructor with the center point and the size of the mark.
			Mark(const cv::Point2d & midpoint, const cv::Size2d & normalized_size, const cv::Size & image_size, const size_t & class_index);

			/// Return @p true if an image size has not been set and all points are set to zero.
			bool empty() const;

			/// Retrieve the given non-normalized point.  @see @ref tl() @see @ref tr() @see @ref br() @see @ref bl()
			cv::Point get_corner(const ECorner type) const;

			cv::Point get_corner(const ECorner type, const cv::Size & new_image_dimensions);

			VPoints get_all_points() const;

			VPoints get_all_points(const cv::Size & new_image_dimensions);

			/// Get the non-normalized rectangle that contains all 4 points.
			cv::Rect get_bounding_rect() const;

			/// Update the image dimensions before returning the non-normalized bounding rectangle.
			cv::Rect get_bounding_rect(const cv::Size & new_image_dimensions);

			/// Get the normalized rectangle that contains all 4 points.
			cv::Rect2d get_normalized_bounding_rect() const;

			/// Convenient alias to retrieve the given non-normalized corner point.  @see @ref get_corner() @{
			cv::Point tl() const { return get_corner(ECorner::kTL); }
			cv::Point tr() const { return get_corner(ECorner::kTR); }
			cv::Point br() const { return get_corner(ECorner::kBR); }
			cv::Point bl() const { return get_corner(ECorner::kBL); }
			/// @}

			Mark & add(const cv::Point & new_point);
			Mark & add(cv::Point2d new_point);
			Mark & set(const ECorner & corner, const cv::Point & new_point);
			Mark & set(const ECorner & corner, cv::Point2d new_point);

			/** Check each of the corners after one of the points has been modified by the user.  If a point has been
			 * moved to a new location, then the order of TL-TR-BR-BL might also have been impacted, in which case the
			 * points need to be re-assigned to a new corner.  This is automatically called by methods such as
			 * @ref add() and @ref set().
			 */
			Mark & rebalance();

			cv::Scalar get_colour();

			/** Map of @em double points for each of the 4 vertices.  These points are in the range of [0...1] so they
			 * need to be multiplied by the image width/height to get coordinates.  You typically shouldn't have to
			 * access these points directly.  Instead, use @ref get_corner(), @ref tl(), @ref tr(), etc.  To modify
			 * a point, use @ref add() or @ref set() which will keep this in sync with @ref normalized_all_points().
			 */
			MapCornerToPoint2d normalized_corner_points;

			/** Vector of all points that make up the mark.  At the very least, there should be 4 points which are
			 * used as the 4 corners, but a mark may have more than 4 points which will impact the bounding rect.
			 * Similarly to @ref normalized_corner_points, these shouldn't be accessed directly.  Instead, use methods
			 * like @ref get_all_points(), @ref add(), or @ref set().
			 */
			VPoints2d normalized_all_points;

			cv::Size image_dimensions;

			size_t class_idx;
			std::string name;
			std::string description;
//			cv::Scalar colour;
	};
}
