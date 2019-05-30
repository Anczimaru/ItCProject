#ifndef CONFIG
#define CONFIG

#include "elliptic/curve.hpp"
#include "elliptic/point.hpp"
#include <boost/multiprecision/cpp_int.hpp>
#include "elliptic/utility.hpp"

using boost::multiprecision::int1024_t;
using namespace ec;

enum : unsigned{
    MAX_SEGMENT_SIZE = 512,
    IP_PADDING = 24,
    SERVER_PORT = 1069

};

// Predefined curve values from Brainpool standard, 160 bit version
auto curve_ptr = std::make_shared<Curve<int1024_t>>(
    /* A */ int1024_t{"0x340E7BE2A280EB74E2BE61BADA745D97E8F7C300"},
    /* B */ int1024_t{"0x1E589A8595423412134FAA2DBDEC95C8D8675E58"},
    /* p */ int1024_t{"0xE95E4A5F737059DC60DFC7AD95B3D8139515620F"}
);

Point<int1024_t> generating_point{
    /* x */ int1024_t{"0xBED5AF16EA3F6A4F62938C4631EB5AF7BDBCDBC3"},
    /* y */ int1024_t{"0x1667CB477A1A8EC338F94741669C976316DA6321"},
    /* ptr */ curve_ptr
};

const int1024_t group_size{"0xE95E4A5F737059DC60DF5991D45029409E60FC09"};

#endif
