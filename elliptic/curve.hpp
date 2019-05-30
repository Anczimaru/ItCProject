#ifndef CURVE_ELLIPTIC_ECDHA_ITC_INCLUDED
#define CURVE_ELLIPTIC_ECDHA_ITC_INCLUDED

#include <vector>
#include "utility.hpp"
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <stdexcept>
#include <algorithm>
#include <set>
#include <iterator>




namespace ec
{

template <typename Numeric>
inline Numeric modulo(Numeric a, Numeric b)
{
    Numeric mod = a % b;
    if (mod < 0) {
        if (b < 0)
            return mod - b;
        else
            return mod + b;
    } else
        return mod;
}

template <typename Numeric>
Numeric eeModDiv(Numeric a, Numeric b, Numeric mod)
{
    if (b == 1)
    {
      std::cout << "d= 1, c = g = "<< a << '\n';
        return a;
    }
    Numeric r0{mod},
            r1{modulo(b, mod)},
            s0{0},
            s1{modulo(a, mod)};
    while (r1 > 0)
    {
        Numeric q{r0 / r1};
        Numeric r2{modulo((r0 - q * r1), mod)};
        Numeric s2{modulo((s0 - q * s1), mod)};
        r0 = r1;
        r1 = r2;
        s0 = s1;
        s1 = s2;
    }
    if (r0 == 1)
    {
        return s0;
    }
    else
    {
        return r0;
        //return -1;
    }
}
// Modular exp

template <typename Numeric>
Numeric modExp(Numeric v,Numeric mod, Numeric g)
{
    Numeric x;
    std::vector <bool> temp = makeRevBitVector(v);
    if(temp.size()<2){
      return modulo(g,mod);
    }
    x = g;

   for(int i = temp.size()-2; i>=0; i--)
  //for (int i = 0; i< countBits(v); i++)
    {
       x =  modulo(x*x,mod);
       if((temp[i])==1)
       {
           x = modulo(g*x,mod);
       }
    }

    return x;
}
template <typename Numeric>
Numeric generateLucas(Numeric n, Numeric p, Numeric q, Numeric index,
                      Numeric &qOut)
{
    if (index < 3)
    // Change: in A.2.4. k (index) has to be greater or equal 2. Since 3 -> 2
    // Major change: we use  our defined modulo() function instead of %
    // Change: modulo variable --> n according to standard
        throw std::out_of_range{"in generateLucas() - index was not applicable for this algorithm"};

    Numeric v0 = 2, v1 = p;
    Numeric q0 = 1, q1 = 1;
    std::vector<bool> index_bits = std::move(makeRevBitVector(index));
    std::reverse(index_bits.begin(), index_bits.end());
    for (const auto &bit: index_bits) {
        q0 =  modulo((q0*q1),n);    //(q0 * q1) % modulo;
        if (bit) {
            q1 = modulo((q0 * q),n);//(q0 * q) % modulo;
            v0 = modulo((v0 * v1 - p * q0),n);     //(v0 * v1 - p * q0) % modulo;
            v1 = modulo((v1 * v1 - 2 * q1),n);    //(v1 * v1 - 2 * q1) % modulo;
        } else {
            q1 = q0;
            v1 = modulo((v0 * v1 - p * q0),n);  //(v0 * v1 - p * q0) % modulo;
            v0 = modulo((v0 * v0 - 2 * q0),n); //(v0 * v0 - 2 * q0) % modulo;
        }
    }

    qOut = q0;
    return v0;
}


template <typename Numeric>
Numeric squareRootsModPrime(Numeric p, Numeric g)
{
    std::set <Numeric> alreadyPicked;
    if(g>0 && g<p)
    {
        if (modulo((p-3), Numeric{4})==0)
        {
            Numeric k = (p-3)/4;
            return modExp(k+1, p, g);
        }
        else if(modulo((p-5), Numeric{8})==0)
        {
            Numeric k = (p-5)/8;
            Numeric gamma = modExp(k, p, (2*g));
            Numeric i = modulo((2*g*gamma*gamma), p);
            return modulo((g*gamma*(i-1)), p);
        }
        else if(modulo((p-1),Numeric{8})==0)
        {
            Numeric q0;

            Numeric Q=g;
            Numeric P;
            Numeric V;
            Numeric z;
            do {
                do {
                    P=rand()%p;
                } while(P==0 || P==p || (alreadyPicked.find(P) != alreadyPicked.end()) );
                // not already chosen? Does it correspond to "g" as well?
                //generateLucas(n, p, q, index, memory of qout)  -> standard reference
                alreadyPicked.insert(P);
                V = generateLucas(p, P, Q, (p+1)/2, q0);
                generateLucas(p,P,Q,(p-1)/2,q0);
                z= modulo(V/2,p);

                if (modExp(Numeric{2},p,z)==g)
                    return z;

                if (q0>1 && q0<(p-1))
                {
                    std::cout << "No square roots exist" << '\n';
                    return -1; //Only positive values can be returned, thus -1 is a value for lack of roots.
                }
            } while(true);
            //throw std::out_of_range{"in squareRootsModPrime() - no square roots exist"};
        }
        else
            throw std::runtime_error{"in squareRootsModPrime() - case that should never happen occured"};
    }
    else
        throw std::out_of_range{"in squareRootsModPrime() - invalid input argument"};
}

template <typename Numeric>
class Curve
{
public:
    Numeric a_, b_, mod_;

    // TODO: makes this functions POD
    Curve(Numeric a, Numeric b, Numeric p):
        a_{a}, b_{b}, mod_{p}
    {
    }
};

} /* ec */

#endif /* end of include guard: CURVE_ELLIPTIC_ECDHA_ITC_INCLUDED */
