// DarkMark (C) 2019-2021 Stephane Charette <stephanecharette@gmail.com>

#include <gtest/gtest.h>


int main(int argc, char **argv)
{
	std::cout << std::fixed << std::setprecision(10);

	testing::InitGoogleTest( &argc, argv );

	return RUN_ALL_TESTS();
}
