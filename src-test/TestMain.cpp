/* DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
 * $Id$
 */

#include <gtest/gtest.h>


int main(int argc, char **argv)
{
	std::cout << std::fixed << std::setprecision(10);

	testing::InitGoogleTest( &argc, argv );

	return RUN_ALL_TESTS();
}
