// DarkMark (C) 2019-2022 Stephane Charette <stephanecharette@gmail.com>

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

	const double minx = std::max(0.0, midpoint.x - half_width);
	const double miny = std::max(0.0, midpoint.y - half_height);
	const double maxx = std::min(1.0, midpoint.x + half_width);
	const double maxy = std::min(1.0, midpoint.y + half_height);

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
	is_prediction		= false;

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

	const int w = image_dimensions.width - 1;
	const int h = image_dimensions.height - 1;

	if (p.x < 0) p.x = 0;
	if (p.y < 0) p.y = 0;
	if (p.x > w) p.x = w;
	if (p.y > h) p.y = h;

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

	const int max_w = image_dimensions.width - 1;
	const int max_h = image_dimensions.height - 1;

	for (const auto & normalized_point : normalized_all_points)
	{
		cv::Point p;
		p.x = std::round(normalized_point.x * iw);
		p.y = std::round(normalized_point.y * ih);

		if (p.x < 0) p.x = 0;
		if (p.y < 0) p.y = 0;
		if (p.x > max_w) p.x = max_w;
		if (p.y > max_h) p.y = max_h;

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
	cv::Rect r = cv::boundingRect(v);

	// limit the rectangle to the exact size of the image
	if (r.x < 0) r.x = 0;
	if (r.y < 0) r.y = 0;
	if (r.x + r.width	> image_dimensions.width)		r.width		= image_dimensions.width	- r.x;
	if (r.y + r.height	> image_dimensions.height)		r.height	= image_dimensions.height	- r.y;

	return r;
}


cv::Rect dm::Mark::get_bounding_rect(const cv::Size & new_image_dimensions)
{
	image_dimensions = new_image_dimensions;

	return get_bounding_rect();
}


cv::Rect2d dm::Mark::get_normalized_bounding_rect() const
{
	// calling cv::boundingRect() doesn't work with cv::Point2d, so do it manually

	double min_x = normalized_all_points.at(0).x;
	double min_y = normalized_all_points.at(0).y;
	double max_x = min_x;
	double max_y = min_y;

	for (const auto & p : normalized_all_points)
	{
		if (p.x < min_x) min_x = p.x;
		if (p.x > max_x) max_x = p.x;
		if (p.y < min_y) min_y = p.y;
		if (p.y > max_y) max_y = p.y;
	}

	// limit the rectangle to the exact size of the image
	if (min_x < 0.0) min_x = 0.0;
	if (min_y < 0.0) min_y = 0.0;
	if (max_x > 1.0) max_x = 1.0;
	if (max_y > 1.0) max_y = 1.0;

	const cv::Point2d p1(min_x, min_y);
	const cv::Point2d p2(max_x, max_y);

	return cv::Rect2d(p1, p2);
}


cv::Point2d dm::Mark::get_normalized_midpoint()
{
	cv::Rect2d r = get_normalized_bounding_rect();
	const double x = r.x + r.width / 2.0;
	const double y = r.y + r.height / 2.0;

	return cv::Point2d(x, y);
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

	// fix up any out-of-bounds points
	for (auto & p : normalized_all_points)
	{
		if (p.x < 0.0) p.x = 0.0;
		if (p.x > 1.0) p.x = 1.0;
		if (p.y < 0.0) p.y = 0.0;
		if (p.y > 1.0) p.y = 1.0;
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
	for (const auto & iter : bounding_rect_points)
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


cv::Scalar dm::Mark::get_colour()
{
	const auto v = DarkHelp::get_default_annotation_colours();
	const auto colour = v[class_idx % v.size()];

	return colour;
}
