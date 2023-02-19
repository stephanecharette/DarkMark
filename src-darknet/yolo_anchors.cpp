// DarkMark (C) 2019-2022 Stephane Charette <stephanecharette@gmail.com>

#include <algorithm>
#include <fstream>
#include <limits>
#include <random>
#include <string>
#include <vector>
#include <map>
#include "yolo_anchors.hpp"
#include "Tools.hpp"


typedef std::vector<int>	VInt;
typedef std::vector<float>	VFloat;


struct box_label
{
	int class_idx;
	float x;
	float y;
	float w;
	float h;
};

typedef std::vector<box_label> Boxes;


struct matrix
{
	size_t rows;
	size_t cols;
	VFloat vals;

	matrix(const size_t r, const size_t c)
	{
		rows = r;
		cols = c;
		vals = VFloat(std::max<size_t>(1lu, r * c), 0.0f);
		return;
	}

	matrix() : matrix(0, 0)
	{
		return;
	}

	float & row_and_col(const size_t r, const size_t c)
	{
		if (rows == 0 or cols == 0)
		{
			throw std::invalid_argument("cannot get cell r=" + std::to_string(r) + " c=" + std::to_string(c) + " when rows=" + std::to_string(rows) + " and cols=" + std::to_string(cols));
		}
		if (r >= rows or c >= cols)
		{
			throw std::invalid_argument("invalid cell r=" + std::to_string(r) + " c=" + std::to_string(c) + " when rows=" + std::to_string(rows) + " and cols=" + std::to_string(cols));
		}

		return vals.at(r * cols + c);
	}

	float * row(const size_t r)
	{
		return & row_and_col(r, 0);
	}
};


struct model
{
	VInt assignments;
	matrix centers;
};


float dist(float *x, float *y, int n)
{
	float mw = (x[0] < y[0]) ? x[0] : y[0];
	float mh = (x[1] < y[1]) ? x[1] : y[1];
	float inter = mw * mh;
	float sum = x[0] * x[1] + y[0] * y[1];
	float un = sum - inter;
	float iou = inter / un;

	return 1.0f - iou;
}


VInt random_samples(const size_t maximum_number_of_samples)
{
	VInt v;
	v.reserve(maximum_number_of_samples);
	while (v.size() < maximum_number_of_samples)
	{
		v.push_back(v.size());
	}

	#if 0
	//
	// If using a stand-alone version of yolo anchors, you'll have to use this
	// (or something similar) as your random number generator.
	//
	std::random_device rd;
	std::mt19937 rng(rd());
	#else
	auto & rng = dm::get_random_engine();
	#endif

	std::shuffle(v.begin(), v.end(), rng);

	return v;
}


void random_centers(matrix & data, matrix & centers)
{
	const VInt s = random_samples(data.rows);
	for (size_t row = 0; row < centers.rows; row ++)
	{
		for (size_t col = 0; col < data.cols; col ++)
		{
			centers.row_and_col(row, col) = data.row_and_col(s[row], col);
		}
	}

	return;
}


int closest_center(float *datum, matrix & centers)
{
	int best = 0;
	float best_dist = dist(datum, centers.row(best), centers.cols);
	for (size_t j = 0; j < centers.rows; ++j)
	{
		float new_dist = dist(datum, centers.row(j), centers.cols);
		if (new_dist < best_dist)
		{
			best_dist = new_dist;
			best = j;
		}
	}

	return best;
}


int kmeans_expectation(matrix & data, VInt & assignments, matrix & centers)
{
	int converged = 1;
	for (size_t i = 0; i < data.rows; ++i)
	{
		int closest = closest_center(data.row(i), centers);
		if (closest != assignments[i])
		{
			converged = 0;
		}
		assignments[i] = closest;
	}

	return converged;
}


void kmeans_maximization(matrix & data, VInt & assignments, matrix & centers)
{
	matrix old_centers(centers.rows, centers.cols);

	VInt counts(centers.rows, 0);

	for (size_t i = 0; i < centers.rows; ++i)
	{
		for (size_t j = 0; j < centers.cols; ++j)
		{
			old_centers.row_and_col(i, j) = centers.row_and_col(i, j);
			centers.row_and_col(i, j) = 0;
		}
	}

	for (size_t i = 0; i < data.rows; ++i)
	{
		++counts[assignments[i]];
		for (size_t j = 0; j < data.cols; ++j)
		{
			centers.row_and_col(assignments[i], j) += data.row_and_col(i, j);
		}
	}

	for (size_t i = 0; i < centers.rows; ++i)
	{
		if (counts[i])
		{
			for (size_t j = 0; j < centers.cols; ++j)
			{
				centers.row_and_col(i, j) /= counts[i];
			}
		}
	}

	for (size_t i = 0; i < centers.rows; ++i)
	{
		for (size_t j = 0; j < centers.cols; ++j)
		{
			if(centers.row_and_col(i, j) == 0)
			{
				centers.row_and_col(i, j) = old_centers.row_and_col(i, j);
			}
		}
	}

	return;
}


model do_kmeans(matrix & data, const size_t number_of_clusters)
{
	matrix centers(number_of_clusters, data.cols);

	VInt assignments(data.rows, 0);

	random_centers(data, centers);

	for (int i = 0; i < 1000 and not kmeans_expectation(data, assignments, centers); ++i)
	{
		kmeans_maximization(data, assignments, centers);
	}

	model m;
	m.assignments = assignments;
	m.centers = centers;

	return m;
}


Boxes read_boxes(const std::string & filename)
{
	Boxes boxes;
	boxes.reserve(20);

	std::ifstream ifs(filename);
	while (ifs.good())
	{
		box_label box;
		ifs >> box.class_idx >> box.x >> box.y >> box.w >> box.h;
		if (ifs.eof())
		{
			// ran into EOF, meaning we didn't read all 5 entries we need, don't use this entry!
			break;
		}
		boxes.push_back(box);
	}

	return boxes;
}


void calc_anchors(const std::string & train_images_filename, const size_t number_of_clusters, const size_t width, const size_t height, const size_t number_of_classes, std::string & new_anchors, std::string & new_counters_per_class, float & new_avg_iou)
{
	new_anchors				= "";
	new_counters_per_class	= "";
	new_avg_iou				= 0.0f;

	if (width	< 32 or
		width	% 32 or
		height	< 32 or
		height	% 32 or
		number_of_clusters <= 1 or
		train_images_filename.empty())
	{
		throw std::invalid_argument("width and height must both be multiples of 32, and number_of_clusters must be greater than 1");
	}

	VFloat rel_width_height_array;
	VInt counter_per_class(number_of_classes, 0);
	size_t number_of_boxes = 0;
	std::string path;
	std::ifstream ifs(train_images_filename);
	while (std::getline(ifs, path))
	{
		if (path.empty())
		{
			continue;
		}

		// find the .txt label file that goes with this image file
		std::string labelpath = path;
		size_t pos = labelpath.rfind(".");
		if (pos != std::string::npos)
		{
			labelpath.erase(pos);
		}
		labelpath += ".txt";

		for (const auto & box : read_boxes(labelpath))
		{
			number_of_boxes ++;
			counter_per_class[box.class_idx] ++;
			rel_width_height_array.push_back(box.w * static_cast<float>(width));
			rel_width_height_array.push_back(box.h * static_cast<float>(height));
		}
	}

	matrix boxes_data(number_of_boxes, 2);
	for (size_t i = 0; i < number_of_boxes; ++i)
	{
		boxes_data.row_and_col(i, 0) = rel_width_height_array[i * 2 + 0];
		boxes_data.row_and_col(i, 1) = rel_width_height_array[i * 2 + 1];
	}

	// K-means
	model anchors_data = do_kmeans(boxes_data, number_of_clusters);

	// Store all the sizes in a multimap.  The key is the total area, and the value is the width+height stored as strings.
	// This way the multimap will automatically sort all the values for us from smallest to largest, and all that we need
	// to do is create the final string from all the values.
	std::multimap<float, std::string> mm;
	for (size_t row = 0; row < number_of_clusters; row ++)
	{
		const float w				= anchors_data.centers.row_and_col(row, 0);
		const float h				= anchors_data.centers.row_and_col(row, 1);
		const float area			= w * h;
		const size_t round_width	= std::round(w);
		const size_t round_height	= std::round(h);
		const std::string text	= std::to_string(round_width) + ", " + std::to_string(round_height);
		mm.insert(std::make_pair(area, text));
	}
	for (auto [key, val] : mm)
	{
		(void)key; // silence "unused variable" warning on older compilers (Ubuntu 18.04 and g++ 7.5.0)

		if (not new_anchors.empty())
		{
			new_anchors += ", ";
		}
		new_anchors += val;
	}

	// now we figure out the values for counters_per_class

	float avg_iou = 0.0f;
	for (size_t i = 0; i < number_of_boxes; ++i)
	{
		float box_w		= rel_width_height_array[i * 2 + 0];
		float box_h		= rel_width_height_array[i * 2 + 1];
		float min_dist	= std::numeric_limits<float>::max();
		float best_iou	= 0.0f;
		for (size_t j = 0; j < number_of_clusters; ++j)
		{
			const float anchor_w		= anchors_data.centers.row_and_col(j, 0);
			const float anchor_h		= anchors_data.centers.row_and_col(j, 1);
			const float min_w			= (box_w < anchor_w) ? box_w : anchor_w;
			const float min_h			= (box_h < anchor_h) ? box_h : anchor_h;
			const float box_intersect	= min_w * min_h;
			const float box_union		= box_w * box_h + anchor_w * anchor_h - box_intersect;
			const float iou				= box_intersect / box_union;
			const float distance		= 1 - iou;
			if (distance < min_dist)
			{
				min_dist = distance;
				best_iou = iou;
			}
		}

		if (best_iou > 0.0f and best_iou < 1.0f)
		{
			avg_iou += best_iou;
		}
	}

	for (size_t i = 0; i < number_of_classes; i++)
	{
		if (i > 0)
		{
			new_counters_per_class += ", ";
		}
		new_counters_per_class += std::to_string(counter_per_class[i]);
	}

	new_avg_iou = 100.0f * avg_iou / static_cast<float>(number_of_boxes);

	return;
}


#if 0
int main(int argc, char * argv[])
{
	std::string new_anchors;
	std::string counters_per_class;
	std::string avg_iou;

	calc_anchors("/home/stephane/nn/vz_pharmacy/vz_pharmacy_train.txt", 9, 800, 608, 8, new_anchors, counters_per_class, avg_iou);

	std::cout	<< "counters_per_class=" << counters_per_class << std::endl
				<< "anchors=" << new_anchors << std::endl
				<< "avg_iou=" << avg_iou << std::endl;

	return 0;
}
#endif
