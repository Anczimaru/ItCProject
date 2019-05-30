#include "../elliptic/curve.hpp"
#include "../elliptic/point.hpp"

#include <memory>
#include <iostream>
#include <ctime>
#include <cstdlib>

#define BOOST_TEST_MODULE curve_points_test
#include <boost/test/included/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/data/monomorphic.hpp>
#include <boost/multiprecision/cpp_int.hpp>

namespace bdata = boost::unit_test::data;
namespace bnum = boost::multiprecision;

using namespace ec;


BOOST_AUTO_TEST_CASE(PointsEq)
{
    auto curve_1 = std::make_shared<Curve<int>>(2, 3, 5);
    auto curve_2 = std::make_shared<Curve<int>>(2, 5, 7);

    Point<int> left{2, 3, curve_1};
    Point<int> equal_right{2, 3, curve_1};
    BOOST_REQUIRE_EQUAL(left, left);
    BOOST_REQUIRE_EQUAL(left, equal_right);

    Point<int> not_equal_curve{2, 3, curve_2};
    Point<int> not_equal_x    {3, 3, curve_1};
    Point<int> not_equal_y    {2, 5, curve_1};
    BOOST_REQUIRE_NE(left, not_equal_curve);
    BOOST_REQUIRE_NE(left, not_equal_x);
    BOOST_REQUIRE_NE(left, not_equal_y);

    Point<int> zero_c1{curve_1};
    Point<int> zero_c2{curve_2};
    Point<int> zero_equal{curve_1};
    BOOST_REQUIRE_EQUAL(zero_c1, zero_equal);
    BOOST_REQUIRE_NE(zero_c1, zero_c2);
    BOOST_REQUIRE_NE(left, zero_c1);
}

BOOST_AUTO_TEST_CASE(Modulo)
{
    BOOST_REQUIRE_EQUAL(modulo(4,  2), 0);
    BOOST_REQUIRE_EQUAL(modulo(5,  2), 1);
    BOOST_REQUIRE_EQUAL(modulo(-4, 2), 0);
    BOOST_REQUIRE_EQUAL(modulo(-5, 2), 1);

    BOOST_REQUIRE_EQUAL(modulo(4,  -2), 0);
    BOOST_REQUIRE_EQUAL(modulo(5,  -2), 1);
    BOOST_REQUIRE_EQUAL(modulo(-4, -2), 0);
    BOOST_REQUIRE_EQUAL(modulo(-5, -2), 1);
}

BOOST_AUTO_TEST_CASE(eeModuloDiv1)
{
    bnum::int256_t a = 417;
    bnum::int256_t b = 129;
    bnum::int256_t p = 242;
    bnum::int256_t out = eeModDiv(a,b,p);
    // Inv of 129 is 227, 417 * 227 mod 242 is 37
    BOOST_REQUIRE_EQUAL(out, 37);
}
/*BOOST_AUTO_TEST_CASE(eeModuloDiv256B)
{
    bnum::int256_t a("297190522446607939568481567949428902921613329152");
    bnum::int256_t b("173245649450172891208247283053495198538671808088");
    bnum::int256_t p("1332297598440044874827085558802491743757193798159");
    bnum::int256_t out(eeModDiv(a,b,p));
    // Inv of 129 is 227, 417 * 227 mod 242 is 37
    BOOST_REQUIRE_EQUAL(out, "73958144846821981242838446280227822923963964972");
}
*/
BOOST_AUTO_TEST_CASE(eeModuloDiv2)
{
    int divres = eeModDiv(619, 79, 144);
    // Inv of 79 is 31, 619 * 31 mod 144 is 37
    BOOST_REQUIRE_EQUAL(divres, 37);
}

BOOST_AUTO_TEST_CASE(eeModuloDiv4)
{
    int divres = eeModDiv(37, 13, 17);
    // Inv of 13 is 4, 37 * 4 mod 17 is 12
    BOOST_REQUIRE_EQUAL(divres, 12);
}

BOOST_AUTO_TEST_CASE(eeModuloDiv3)
{
    int divres = eeModDiv(605, 466, 266);
    // Inv of 466 mod 266 does not exist
    BOOST_REQUIRE_EQUAL(divres, -1);
}

BOOST_AUTO_TEST_CASE(pointAdd1)
{
    auto c1 = std::make_shared<Curve<int>>(2, 2, 17);
    Point<int> test_points[19] = {
        {5, 1, c1},
        {6, 3, c1},
        {10, 6, c1},
        {3, 1, c1},
        {9, 16, c1},
        {16, 13, c1},
        {0, 6, c1},
        {13, 7, c1},
        {7, 6, c1},
        {7, 11, c1},
        {13, 10, c1},
        {0, 11, c1},
        {16, 4, c1},
        {9, 1, c1},
        {3, 16, c1},
        {10, 11, c1},
        {6, 14, c1},
        {5, 16, c1},
        {c1}
    };

    Point<int> g = test_points[0];
    Point<int> cumulative = g;
    for (int i = 1; i < 19; ++i) {
        cumulative = cumulative + g;
        //auto both_sides = g + cumulative;
        BOOST_REQUIRE_EQUAL(cumulative, test_points[i]);
        //BOOST_REQUIRE_EQUAL(cumulative, both_sides);
    }
}

BOOST_AUTO_TEST_CASE(pointMultiply)
{
    auto c1 = std::make_shared<Curve<int>>(2, 2, 17);
    Point<int> test_points[19] = {
        {5, 1, c1},
        {6, 3, c1},
        {10, 6, c1},
        {3, 1, c1},
        {9, 16, c1},
        {16, 13, c1},
        {0, 6, c1},
        {13, 7, c1},
        {7, 6, c1},
        {7, 11, c1},
        {13, 10, c1},
        {0, 11, c1},
        {16, 4, c1},
        {9, 1, c1},
        {3, 16, c1},
        {10, 11, c1},
        {6, 14, c1},
        {5, 16, c1},
        {c1}
    };

    Point<int> g = test_points[0];
    for (int i = 1; i < 19; ++i) {
        Point<int> test = (i + 1) * g;
        BOOST_CHECK_EQUAL(test, test_points[i]);
    }
}

BOOST_AUTO_TEST_CASE(modExp1)
{
    int out = modExp(3,3,2);
    // Inv of 466 mod 266 does not exist
    BOOST_REQUIRE_EQUAL(out, 2);
}
BOOST_AUTO_TEST_CASE(modExp2)
{
    int out = modExp(3,2,2);
    // Inv of 466 mod 266 does not exist
    BOOST_REQUIRE_EQUAL(out, 0);
}
BOOST_AUTO_TEST_CASE(modExp3)
{
    int out = modExp(13,103,232);
    // Inv of 466 mod 266 does not exist
    BOOST_REQUIRE_EQUAL(out, 19);
}

BOOST_AUTO_TEST_CASE(modExp4)
{
    int out = modExp(13,103,-232);
    // Inv of 466 mod 266 does not exist
    BOOST_REQUIRE_EQUAL(out, 84);
}
BOOST_AUTO_TEST_CASE(modExp5)
{
    int out = modExp(17,27,-214);
    // Inv of 466 mod 266 does not exist
    BOOST_REQUIRE_EQUAL(out, 14);
}
BOOST_AUTO_TEST_CASE(Lucas1)
{
    int qout = 0;
    int out = generateLucas(4,2,1,2,qout);

    BOOST_REQUIRE_EQUAL(out, 2 );
    BOOST_REQUIRE_EQUAL(qout, 1 );
}



BOOST_AUTO_TEST_CASE(sqrtMod3)
{
    srand(time(NULL));
    int out = squareRootsModPrime(41,4);

    BOOST_REQUIRE_EQUAL(out, 2);

}
BOOST_AUTO_TEST_CASE(sqrtMod4)
{
    srand(time(NULL));
    int out = squareRootsModPrime(17,2);

    BOOST_REQUIRE_EQUAL(out, 6);

}

BOOST_AUTO_TEST_CASE(sqrtMod5)
{
    srand(time(NULL));
    int out = squareRootsModPrime(73,58);

    BOOST_REQUIRE_EQUAL(out, -1);

}
// BOOST_AUTO_TEST_CASE(RandGen)
// {
//
//     Point<int> randomPoint = findRandomPointOnCurve(23,1,1);
//     std::cout << "Random point generation test: " << randomPoint << '\n';
//
//
// }
