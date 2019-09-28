/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include "DarkMark.hpp"


dm::Mark::~Mark()
{
	return;
}


dm::Mark::Mark() :
	Mark(cv::Point2d(0.0, 0.0), cv::Size2d(0.0, 0.0), cv::Size(0, 0), 0)
{
	return;
}


dm::Mark::Mark(const cv::Point2d & midpoint, const cv::Size2d & normalized_size, const cv::Size & image_size, const size_t & class_index)
{
	const double half_width		= normalized_size.width		/ 2.0;
	const double half_height	= normalized_size.height	/ 2.0;

	const double minx = midpoint.x - half_width;
	const double miny = midpoint.y - half_height;
	const double maxx = midpoint.x + half_width;
	const double maxy = midpoint.y + half_height;

	normalized_all_points =
	{
		{minx, miny},
		{maxx, miny},
		{maxx, maxy},
		{minx, maxy}
	};

	normalized_corner_points =
	{
		{ECorner::kTL, {minx, miny}},
		{ECorner::kTR, {maxx, miny}},
		{ECorner::kBR, {maxx, maxy}},
		{ECorner::kBL, {minx, maxy}}
	};

	image_dimensions	= image_size;
	class_idx			= class_index;

	return;
}


bool dm::Mark::empty() const
{
	// TRUE if image size is zero and every point is zero

	bool is_empty = true;

	if (image_dimensions.width > 0 and image_dimensions.height > 0)
	{
		is_empty = false;
	}
	else
	{
		for (const auto & p : normalized_all_points)
		{
			if (p.x != 0.0 or p.y != 0.0)
			{
				is_empty = false;
				break;
			}
		}
	}

	return is_empty;
}


cv::Point dm::Mark::get_corner(const ECorner corner) const
{
	cv::Point p(0, 0);

	if (normalized_corner_points.count(corner) == 1)
	{
		p.x = std::round(normalized_corner_points.at(corner).x * static_cast<double>(image_dimensions.width));
		p.y = std::round(normalized_corner_points.at(corner).y * static_cast<double>(image_dimensions.height));
	}

	return p;
}


cv::Point dm::Mark::get_corner(const ECorner type, const cv::Size & new_image_dimensions)
{
	image_dimensions = new_image_dimensions;

	return get_corner(type);
}


dm::VPoints dm::Mark::get_all_points() const
{
	VPoints v;
	v.reserve(normalized_all_points.size());

	const double iw = static_cast<double>(image_dimensions.width);
	const double ih = static_cast<double>(image_dimensions.height);

	for (const auto & normalized_point : normalized_all_points)
	{
		cv::Point p;
		p.x = std::round(normalized_point.x * iw);
		p.y = std::round(normalized_point.y * ih);
		v.push_back(p);
	}

	return v;
}


dm::VPoints dm::Mark::get_all_points(const cv::Size & new_image_dimensions)
{
	image_dimensions = new_image_dimensions;

	return get_all_points();
}


cv::Rect dm::Mark::get_bounding_rect() const
{
	const VPoints v = get_all_points();
	const cv::Rect r = cv::boundingRect(v);

	return r;
}


cv::Rect dm::Mark::get_bounding_rect(const cv::Size & new_image_dimensions)
{
	image_dimensions = new_image_dimensions;

	return get_bounding_rect();
}


cv::Rect2d dm::Mark::get_normalized_bounding_rect() const
{
	// calling cv::boundingRect() doesn't work with doubles, so get the full bounding rect and then convert back to normalized values
	const cv::Rect r = get_bounding_rect();

	const double iw = static_cast<double>(image_dimensions.width);
	const double ih = static_cast<double>(image_dimensions.height);

	cv::Rect2d rd;
	rd.x		= static_cast<double>(r.x)		/ iw;
	rd.y		= static_cast<double>(r.y)		/ ih;
	rd.width	= static_cast<double>(r.width)	/ iw;
	rd.height	= static_cast<double>(r.height)	/ ih;

	return rd;
}


dm::Mark & dm::Mark::add(const cv::Point & new_point)
{
	cv::Point2d p;
	p.x = static_cast<double>(new_point.x) / static_cast<double>(image_dimensions.width);
	p.y = static_cast<double>(new_point.y) / static_cast<double>(image_dimensions.height);

	return add(p);
}


dm::Mark & dm::Mark::add(cv::Point2d new_point)
{
	if (new_point.x < 0.0) new_point.x = 0.0;
	if (new_point.x > 1.0) new_point.x = 1.0;
	if (new_point.y < 0.0) new_point.y = 0.0;
	if (new_point.y > 1.0) new_point.y = 1.0;

	normalized_all_points.push_back(new_point);

	return rebalance();
}


dm::Mark & dm::Mark::set(const ECorner & corner, const cv::Point & new_point)
{
	cv::Point2d p;
	p.x = static_cast<double>(new_point.x) / static_cast<double>(image_dimensions.width);
	p.y = static_cast<double>(new_point.y) / static_cast<double>(image_dimensions.height);

	return set(corner, p);
}


dm::Mark & dm::Mark::set(const ECorner & corner, cv::Point2d new_point)
{
	if (new_point.x < 0.0) new_point.x = 0.0;
	if (new_point.x > 1.0) new_point.x = 1.0;
	if (new_point.y < 0.0) new_point.y = 0.0;
	if (new_point.y > 1.0) new_point.y = 1.0;

	cv::Point2d old_point = normalized_corner_points.at(corner);

	auto iter = std::find(normalized_all_points.begin(), normalized_all_points.end(), old_point);
	if (iter != normalized_all_points.end())
	{
		*iter = new_point;
	}
	else
	{
		normalized_all_points.push_back(new_point);
	}

	return rebalance();
}


dm::Mark & dm::Mark::rebalance()
{
	if (normalized_all_points.size() < 4)
	{
		Log("cannot rebalance mark which contains " + std::to_string(normalized_all_points.size()) + " points");
		return *this;
	}

	// convert the map to a vector of normalized points
	VPoints2d points = normalized_all_points;

	const cv::Rect2d bounding_rect = get_normalized_bounding_rect();
	const MapCornerToPoint2d bounding_rect_points =
	{
		{ECorner::kTL, {bounding_rect.x							, bounding_rect.y						}},
		{ECorner::kTR, {bounding_rect.x + bounding_rect.width	, bounding_rect.y						}},
		{ECorner::kBR, {bounding_rect.x + bounding_rect.width	, bounding_rect.y + bounding_rect.height}},
		{ECorner::kBL, {bounding_rect.x							, bounding_rect.y + bounding_rect.height}}
	};

	// take one corner at a time, and figure out the distance between each remaining point and that corner

	MapCornerToPoint2d result;
	for (const auto iter : bounding_rect_points)
	{
		const ECorner type = iter.first;
		const cv::Point2d & corner_point = iter.second;
//		std::cout << "looking for TYPE=" << (int)type << " corner nearest to x=" << corner_point.x << " y=" << corner_point.y << std::endl;

		std::map<double, size_t> distances_to_index_map;
		for (size_t idx = 0; idx < points.size(); idx ++)
		{
			const cv::Point2d & p = points.at(idx);
			const double dx = corner_point.x - p.x;
			const double dy = corner_point.y - p.y;
			const double hypotenuse = std::hypot(dx, dy);
//			std::cout << "-> idx=" << idx << " dx=" << dx << " dy=" << dy << " hypot=" << hypotenuse << std::endl;
			distances_to_index_map[hypotenuse] = idx;
		}

		// maps are sorted in ascending order, so the smallest distance will always be the first entry in the map
		const size_t idx = distances_to_index_map.begin()->second;
//		std::cout << "-> choosing idx=" << idx << std::endl;
		result[type] = points[idx];
		points.erase(points.begin() + idx);
	}

	normalized_corner_points = result;

	return *this;
}
