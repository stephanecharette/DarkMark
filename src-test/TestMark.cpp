// DarkMark (C) 2019-2024 Stephane Charette <stephanecharette@gmail.com>

#include <gtest/gtest.h>
#include "DarkMark.hpp"


TEST(Mark, DefaultConstructor)
{
	dm::Mark mark;

	ASSERT_EQ(mark.normalized_all_points.size(), 4);
	ASSERT_EQ(mark.normalized_corner_points.size(), 4);
	ASSERT_EQ(mark.image_dimensions.width, 0);
	ASSERT_EQ(mark.image_dimensions.height, 0);
	ASSERT_EQ(mark.class_idx, 0);
	ASSERT_EQ(mark.tl(), cv::Point(0, 0));
	ASSERT_EQ(mark.tr(), cv::Point(0, 0));
	ASSERT_EQ(mark.br(), cv::Point(0, 0));
	ASSERT_EQ(mark.bl(), cv::Point(0, 0));
}


TEST(Mark, MidPointConstructor)
{
	dm::Mark mark(cv::Point2d(0.5, 0.5), cv::Size2d(0.5, 0.5), cv::Size(800, 600), 2);

	ASSERT_EQ(mark.normalized_all_points.size(), 4);
	ASSERT_EQ(mark.normalized_corner_points.size(), 4);
	ASSERT_EQ(mark.image_dimensions.width, 800);
	ASSERT_EQ(mark.image_dimensions.height, 600);
	ASSERT_EQ(mark.class_idx, 2);

	// given the image size of 800x600, the mark should be (200, 150) to (600, 450)

	ASSERT_EQ(mark.tl(), cv::Point(200, 150));
	ASSERT_EQ(mark.tr(), cv::Point(600, 150));
	ASSERT_EQ(mark.br(), cv::Point(600, 450));
	ASSERT_EQ(mark.bl(), cv::Point(200, 450));

	// change the image size to 1024x1024 and re-test the numbers: (256, 256) to (768, 768)

	mark.image_dimensions = cv::Size(1024, 1024);

	ASSERT_EQ(mark.tl(), cv::Point(256, 256));
	ASSERT_EQ(mark.tr(), cv::Point(768, 256));
	ASSERT_EQ(mark.br(), cv::Point(768, 768));
	ASSERT_EQ(mark.bl(), cv::Point(256, 768));
}


TEST(Mark, Empty)
{
	dm::Mark mark;
	ASSERT_TRUE(mark.empty());

	mark.add(cv::Point2d(0.5, 0.5));
	ASSERT_FALSE(mark.empty());
}


TEST(Mark, SizeMin)
{
	dm::Mark mark(cv::Point2d(0.0, 0.0), cv::Size2d(0.0, 0.0), cv::Size(640, 480), 1);
	ASSERT_EQ(mark.normalized_all_points.size(), 4);
	ASSERT_EQ(mark.normalized_corner_points.size(), 4);
	ASSERT_EQ(mark.image_dimensions.width, 640);
	ASSERT_EQ(mark.image_dimensions.height, 480);
	ASSERT_EQ(mark.class_idx, 1);
	ASSERT_EQ(mark.tl(), cv::Point(0, 0));
	ASSERT_EQ(mark.tr(), cv::Point(0, 0));
	ASSERT_EQ(mark.br(), cv::Point(0, 0));
	ASSERT_EQ(mark.bl(), cv::Point(0, 0));
}


TEST(Mark, SizeMax)
{
	dm::Mark mark(cv::Point2d(0.5, 0.5), cv::Size2d(1.0, 1.0), cv::Size(640, 480), 1);
	ASSERT_EQ(mark.normalized_all_points.size(), 4);
	ASSERT_EQ(mark.normalized_corner_points.size(), 4);
	ASSERT_EQ(mark.image_dimensions.width, 640);
	ASSERT_EQ(mark.image_dimensions.height, 480);
	ASSERT_EQ(mark.class_idx, 1);
	ASSERT_EQ(mark.tl(), cv::Point(0, 0));
	ASSERT_EQ(mark.tr(), cv::Point(640, 0));
	ASSERT_EQ(mark.br(), cv::Point(640, 480));
	ASSERT_EQ(mark.bl(), cv::Point(0, 480));
}


TEST(Mark, GetAllPoints)
{
	dm::Mark mark(cv::Point2d(0.5, 0.5), cv::Size2d(0.5, 0.5), cv::Size(640, 480), 1);
	dm::VPoints all_points = mark.get_all_points();
	ASSERT_EQ(all_points.size(), 4);

	dm::VPoints expected_points = { cv::Point(160, 120), cv::Point(480, 120), cv::Point(480, 360), cv::Point(160, 360) };
	for (const auto p : all_points)
	{
		// make sure the point "p" is one of the known expected points
		auto iter = std::find(expected_points.begin(), expected_points.end(), p);
		ASSERT_FALSE(iter == expected_points.end());
	}

	// try again but this time specifying a new image size to use

	all_points = mark.get_all_points(cv::Size(1024, 1024));
	ASSERT_EQ(all_points.size(), 4);

	expected_points = { cv::Point(256, 256), cv::Point(768, 256), cv::Point(768, 768), cv::Point(256, 768) };
	for (const auto p : all_points)
	{
		// make sure the point "p" is one of the known expected points
		auto iter = std::find(expected_points.begin(), expected_points.end(), p);
		ASSERT_FALSE(iter == expected_points.end());
	}
}


TEST(Mark, BoundingRectSimple)
{
	// given the image size of 800x600, the mark should be (200, 150) to (600, 450)
	dm::Mark mark(cv::Point2d(0.5, 0.5), cv::Size2d(0.5, 0.5), cv::Size(800, 600), 2);
	ASSERT_EQ(mark.tl(), cv::Point(200, 150));
	ASSERT_EQ(mark.tr(), cv::Point(600, 150));
	ASSERT_EQ(mark.br(), cv::Point(600, 450));
	ASSERT_EQ(mark.bl(), cv::Point(200, 450));

	auto r = mark.get_bounding_rect();

	ASSERT_EQ(r.tl(), cv::Point(200, 150));
	ASSERT_EQ(r.br(), cv::Point(601, 451));

	// change the image size to 1024x1024
	mark.image_dimensions = cv::Size(1024, 1024);
	ASSERT_EQ(mark.tl(), cv::Point(256, 256));
	ASSERT_EQ(mark.tr(), cv::Point(768, 256));
	ASSERT_EQ(mark.br(), cv::Point(768, 768));
	ASSERT_EQ(mark.bl(), cv::Point(256, 768));

	r = mark.get_bounding_rect();

	ASSERT_EQ(r.tl(), cv::Point(256, 256));
	ASSERT_EQ(r.br(), cv::Point(769, 769));
}


TEST(Mark, BoundingRectComplex)
{
	// image=800x600, center=0.543,0.456, size=0.321,0.123
	// w = 800 * 0.321 = 256.8 -> 257
	// h = 600 * 0.123 =  73.8 ->  74
	// x = 800 * 0.543 - w/2 = 305.9 -> 306
	// y = 600 * 0.456 - h/2 = 236.6 -> 237
	// so this means the mark is 306,237 to 563,311
	dm::Mark mark(cv::Point2d(0.543, 0.456), cv::Size2d(0.321, 0.123), cv::Size(800, 600), 0);
	ASSERT_EQ(mark.tl(), cv::Point(306, 237));
	ASSERT_EQ(mark.br(), cv::Point(563, 311));

	auto r = mark.get_bounding_rect();
	ASSERT_EQ(r.tl(), cv::Point(306, 237));
	ASSERT_EQ(r.br(), cv::Point(564, 312));
	ASSERT_EQ(r.width, 258);
	ASSERT_EQ(r.height, 75);
}


TEST(Mark, BoundingRectComplexDouble)
{
	const double x = 0.543;
	const double y = 0.456;
	const double w = 0.321;
	const double h = 0.123;
	// image=800x600, center=0.543,0.456, size=0.321,0.123
	// w = 800 * 0.321 = 256.8 -> 257
	// h = 600 * 0.123 =  73.8 ->  74
	// x = 800 * 0.543 - w/2 = 305.9 -> 306
	// y = 600 * 0.456 - h/2 = 236.6 -> 237
	// so this means the mark is 306,237 to 563,311
	dm::Mark mark(cv::Point2d(x, y), cv::Size2d(w, h), cv::Size(800, 600), 0);
	ASSERT_EQ(mark.tl(), cv::Point(306, 237));
	ASSERT_EQ(mark.br(), cv::Point(563, 311));
	auto r1 = mark.get_bounding_rect();
	ASSERT_EQ(r1.tl(), cv::Point(306, 237));
	ASSERT_EQ(r1.br(), cv::Point(564, 312));

	auto r2 = mark.get_normalized_bounding_rect();
	ASSERT_NEAR(r2.tl().x, x - w / 2.0, 0.005);
	ASSERT_NEAR(r2.tl().y, y - h / 2.0, 0.005);
	ASSERT_NEAR(r2.br().x, x + w / 2.0, 0.005);
	ASSERT_NEAR(r2.br().y, y + h / 2.0, 0.005);
	ASSERT_NEAR(r2.width, w, 0.005);
	ASSERT_NEAR(r2.height, h, 0.005);
}


TEST(Mark, BalanceNothing)
{
	// create a mark from (200, 150) to (600, 450)
	dm::Mark mark(cv::Point2d(0.5, 0.5), cv::Size2d(0.5, 0.5), cv::Size(800, 600), 2);
	ASSERT_EQ(mark.tl(), cv::Point(200, 150));
	ASSERT_EQ(mark.tr(), cv::Point(600, 150));
	ASSERT_EQ(mark.br(), cv::Point(600, 450));
	ASSERT_EQ(mark.bl(), cv::Point(200, 450));

	// the points are already balanced, so this next call should do nothing
	mark.rebalance();

	// nothing should have changed
	ASSERT_EQ(mark.tl(), cv::Point(200, 150));
	ASSERT_EQ(mark.tr(), cv::Point(600, 150));
	ASSERT_EQ(mark.br(), cv::Point(600, 450));
	ASSERT_EQ(mark.bl(), cv::Point(200, 450));
}


TEST(Mark, BalanceSwap2Simple)
{
	// create a mark from (200, 150) to (600, 450)
	dm::Mark mark(cv::Point2d(0.5, 0.5), cv::Size2d(0.5, 0.5), cv::Size(800, 600), 2);
	ASSERT_EQ(mark.tl(), cv::Point(200, 150));
	ASSERT_EQ(mark.tr(), cv::Point(600, 150));
	ASSERT_EQ(mark.br(), cv::Point(600, 450));
	ASSERT_EQ(mark.bl(), cv::Point(200, 450));

	// now swap two of the corners
	cv::Point2d tmp = mark.normalized_corner_points[dm::ECorner::kTR];
	mark.normalized_corner_points[dm::ECorner::kTR] = mark.normalized_corner_points[dm::ECorner::kBL];
	mark.normalized_corner_points[dm::ECorner::kBL] = tmp;
	ASSERT_EQ(mark.tl(), cv::Point(200, 150));
	ASSERT_EQ(mark.tr(), cv::Point(200, 450)); // this one is swapped
	ASSERT_EQ(mark.br(), cv::Point(600, 450));
	ASSERT_EQ(mark.bl(), cv::Point(600, 150)); // this one is swapped

	// balance should restore the points to the appropriate corners
	mark.rebalance();

	ASSERT_EQ(mark.tl(), cv::Point(200, 150));
	ASSERT_EQ(mark.tr(), cv::Point(600, 150));
	ASSERT_EQ(mark.br(), cv::Point(600, 450));
	ASSERT_EQ(mark.bl(), cv::Point(200, 450));
}


TEST(Mark, BalanceSwap2Complex)
{
	const double x = 0.234;
	const double y = 0.345;
	const double w = 0.198;
	const double h = 0.098;
	const double iw = 987.0;
	const double ih = 765.0;
	dm::Mark mark(cv::Point2d(x, y), cv::Size2d(w, h), cv::Size(iw, ih), 0);
	ASSERT_EQ(mark.tl(), cv::Point(133, 226));
	ASSERT_EQ(mark.br(), cv::Point(329, 301));

	// now swap two of the corners
	cv::Point2d tmp = mark.normalized_corner_points[dm::ECorner::kTL];
	mark.normalized_corner_points[dm::ECorner::kTL] = mark.normalized_corner_points[dm::ECorner::kBL];
	mark.normalized_corner_points[dm::ECorner::kBL] = tmp;
	ASSERT_EQ(mark.tl(), cv::Point(133, 301)); // this one is swapped
	ASSERT_EQ(mark.tr(), cv::Point(329, 226));
	ASSERT_EQ(mark.br(), cv::Point(329, 301));
	ASSERT_EQ(mark.bl(), cv::Point(133, 226)); // this one is swapped

	// balance should restore the points to the appropriate corners
	mark.rebalance();

	ASSERT_EQ(mark.tl(), cv::Point(133, 226));
	ASSERT_EQ(mark.tr(), cv::Point(329, 226));
	ASSERT_EQ(mark.br(), cv::Point(329, 301));
	ASSERT_EQ(mark.bl(), cv::Point(133, 301));
}


TEST(Mark, BalanceMove1)
{
	const double x = 0.246;
	const double y = 0.374;
	const double w = 0.486;
	const double h = 0.349;
	const double iw = 749.0;
	const double ih = 836.0;
	const int x1 = std::round(iw * x - iw * w / 2.0);
	const int y1 = std::round(ih * y - ih * h / 2.0);
	const int x2 = std::round(iw * x + iw * w / 2.0);
	const int y2 = std::round(ih * y + ih * h / 2.0);
	dm::Mark mark(cv::Point2d(x, y), cv::Size2d(w, h), cv::Size(iw, ih), 0);
	ASSERT_EQ(mark.tl(), cv::Point(x1, y1));
	ASSERT_EQ(mark.br(), cv::Point(x2, y2));

	// now move the bottom-left corner to a value 1/2 of top-left, which will cause a new rectangle to be defined
	auto p = mark.normalized_all_points[0];
	p.x *= 0.5;
	p.y *= 0.5;
	mark.normalized_all_points[3] = p;

	// balance will shift all the points around since the modified point is now the new TL point
	mark.rebalance();

	ASSERT_EQ(mark.tl(), cv::Point(x1/2, y1/2));	// was BL, now the new TL
	ASSERT_EQ(mark.tr(), cv::Point(x2, y1));		// still is TR
	ASSERT_EQ(mark.br(), cv::Point(x2, y2));		// still is BR
	ASSERT_EQ(mark.bl(), cv::Point(x1, y1));		// used to be TL
}
