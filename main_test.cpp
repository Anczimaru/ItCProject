#include "elliptic/curve.hpp"
#include "elliptic/point.hpp"
#include <boost/multiprecision/cpp_int.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <array>
#include <sstream>
#include "elliptic/utility.hpp"

using boost::multiprecision::int1024_t;
using namespace ec;

int main()
{
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

    int1024_t private_key_A{0}; //private value for A
    do {
        private_key_A = randomBigInt(160);
    } while (private_key_A > group_size);

    Point<int1024_t> half_key = generating_point * private_key_A; //g^a  key to send from A

    std::cout << "Here are the results:"      << std::endl
              << "\t curve parameters:"       << std::endl
              << "\t\t a = " << curve_ptr->a_ << std::endl
              << "\t\t b = " << curve_ptr->b_ << std::endl
              << "\t generating point: " << generating_point  << std::endl
              << "\t group size: "       << group_size        << std::endl
              << "\t private_key: " << private_key_A << std::endl
              << "\t half key: "         << half_key.getX() << std::endl;

    Point<int1024_t> p = findGeneratingPointOnCurve(half_key.getX(), curve_ptr); //nigdzie to potem nie jest użyte?

    int1024_t private_key_B{0}; //Private value for B
    do {
        private_key_B = randomBigInt(160);
    } while (private_key_B > group_size);

    Point<int1024_t> half_key_2 = generating_point * private_key_B; //czyli to g^b key to send from B

    std::cout << "\t p point: " << p.getX() <<std::endl //takie samo jak half key 1
              << "\t half key 2:" << private_key_B << std::endl;

    Point<int1024_t> public_key_A = half_key_2*private_key_A;
    Point<int1024_t> public_key_B = half_key*private_key_B;


    std::stringstream sa;
    std::stringstream sb;
    sa << public_key_A;
    sb << public_key_B;
    std::string s_public_key_A = sa.str();
    std::string s_public_key_B = sb.str();
    std::cout<<"Public key from A: "<< s_public_key_A<<std::endl
            <<" Public key from B: "<< s_public_key_B <<std::endl;
    if(public_key_A.getX() == public_key_B.getX()) std::cout <<"Exchange succesfull" <<std::endl;
}
