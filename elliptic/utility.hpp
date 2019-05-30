#ifndef UTILITY_ELLIPTIC_ECDHA_ITC_INCLUDED
#define UTILITY_ELLIPTIC_ECDHA_ITC_INCLUDED

#include <vector>
#include <random>
#include <type_traits>
#include <sstream>
#include <boost/multiprecision/cpp_int.hpp>
using boost::multiprecision::int1024_t;

namespace ec
{

template <typename Numeric>
std::vector<bool> makeRevBitVector(Numeric in)
{
    std::vector<bool> out;
    while (in != 0) {
        bool last_bit = in & 0x1;
        out.push_back(last_bit);
        in >>= 1;
    }
    return out;
}

template <typename Numeric>
Numeric countBits(Numeric in)
{
    Numeric out{0};
    while (in != 0) {
        ++out;
        in >>= 1;
    }
    return out;
}

int1024_t randomBigInt(unsigned bits)
{
    // Initialize random number generation
    std::random_device rd;
    std::mt19937 generator{rd()};
    std::uniform_int_distribution<unsigned> distrib(0);

    int1024_t random_number{0};
    for (unsigned i = bits / (sizeof(unsigned) * 8); i > 0; i--) {
        constexpr unsigned SHIFT_BY = sizeof(unsigned) * 8;
        unsigned random = distrib(generator);
        random_number <<= SHIFT_BY;
        random_number |= random;
    }

    return random_number;
}
}

namespace std {
template<> struct make_unsigned<int1024_t>
{
    typedef boost::multiprecision::uint1024_t type;
};
}

namespace ec {

template <typename T>
auto ensure_unsigned(T x) -> typename std::make_unsigned<T>::type
{
    if (x > 0)
        return x;
    else
        return -x;
}

template<>
auto ensure_unsigned(int1024_t x) -> typename std::make_unsigned<int1024_t>::type
{
    std::stringstream interworker;
    if (x > 0)
        interworker << x;
    else
        interworker << -x;

    boost::multiprecision::uint1024_t rv;
    interworker >> rv;
    return rv;
}

} /* ec */

#endif /* end of include guard: UTILITY_ELLIPTIC_ECDHA_ITC_INCLUDED */
