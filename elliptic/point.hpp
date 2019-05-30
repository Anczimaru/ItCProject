#ifndef POINT_ELLIPTIC_ECDHA_ITC_INCLUDED
#define POINT_ELLIPTIC_ECDHA_ITC_INCLUDED

#include <memory>
#include <ostream>
#include <stdexcept>
#include "curve.hpp"
#include <type_traits>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <iostream>

#include <boost/multiprecision/integer.hpp>


namespace ec
{

template <typename Numeric>
class Point
{
public:
    typedef std::shared_ptr<Curve<Numeric>> CurvePtr;
    Point(Numeric x, Numeric y, CurvePtr ptr):
        curve_ptr_{ptr},
        x_{x}, y_{y},
        is_zero_{false}
    {
    }

    Point(CurvePtr ptr = nullptr):
        curve_ptr_{ptr},
        x_{0}, y_{0},
        is_zero_{true}
    {
    }

    inline bool    isZero() const {return is_zero_;}
    inline Numeric getX()   const {return x_;}
    inline Numeric getY()   const {return y_;}

    Point<Numeric> operator+(const Point<Numeric> &rhs) const;

    Point<Numeric> operator-() const
    {
        return Point<Numeric>{x_, modulo(-y_, curve_ptr_->mod_), curve_ptr_};
    }

    Point<Numeric> operator-(const Point<Numeric> &rhs) const
    {
        return *this + (-rhs);
    }

    Point<Numeric> operator*(const Numeric &rhs) const;

    bool operator==(const Point<Numeric> &rhs) const;
    bool operator!=(const Point<Numeric> &rhs) const
    {
        return !(*this == rhs);
    }
private:
    CurvePtr curve_ptr_;
    Numeric x_, y_;
    bool is_zero_;
};

// Template functions implementations follow

template <typename Numeric>
std::ostream & operator<<(std::ostream &str, const Point<Numeric> &p)
{
    if (p.isZero())
        str << "(point at infinity)";
    else
        str << '(' << p.getX() << ',' << p.getY() << ')';
    return str;
}

template <typename Numeric>
bool Point<Numeric>::operator== (const Point<Numeric> &rhs) const
{
    if (curve_ptr_ != rhs.curve_ptr_)
        return false;


    if (is_zero_ && rhs.is_zero_)
        return true;
    else if (is_zero_ || rhs.is_zero_)
        return false;
    else
        return ((x_ == rhs.x_) && (y_ == rhs.y_));
}

template <typename Numeric>
Point<Numeric> Point<Numeric>::operator+(const Point<Numeric> &rhs) const
{
    // Special cases
    // Points not on the same curve
    if (curve_ptr_ != rhs.curve_ptr_)
        throw std::invalid_argument{"in Point::operator+ - points not on the same curve"};
    // Points are zero
    if (is_zero_)
        return rhs;
    if (rhs.is_zero_)
        return *this;

    Numeric lambda;

    if (x_ == rhs.x_) {
        if ((y_ != rhs.y_) || (rhs.y_ == 0))
            return Point<Numeric>{curve_ptr_};
        else
            //when 2P
            lambda = eeModDiv((3 * (rhs.x_ * rhs.x_) + curve_ptr_->a_), 2 * rhs.y_, curve_ptr_->mod_);
    } else {
        //when P+Q
        lambda = eeModDiv((y_ - rhs.y_), (x_ - rhs.x_), curve_ptr_->mod_);
    }
    Point<Numeric> ret{0, 0, curve_ptr_};
    ret.x_ = modulo(lambda * lambda - x_ - rhs.x_,        curve_ptr_->mod_);
    ret.y_ = modulo(lambda * (rhs.x_ - ret.x_) - rhs.y_,  curve_ptr_->mod_);

    return ret;
}

template <typename Numeric>
Point<Numeric> Point<Numeric>::operator*(const Numeric &rhs) const
{
    // Define unsigned type for the template
    typedef typename std::make_unsigned<Numeric>::type UNumeric;

    if (rhs == 0) {
        return Point<Numeric>{curve_ptr_};
    }

    Point<Numeric> q = (rhs < 0) ? (-(*this)) : (*this);
    UNumeric k = ensure_unsigned(rhs);
    UNumeric h = 3 * k;

    Point<Numeric> s = q;

    // Iterate over bits of k and h, starting from most significant bit
    // Finish when all bits are covered
    std::vector<bool> k_bits = std::move(makeRevBitVector(k));
    std::vector<bool> h_bits = std::move(makeRevBitVector(h));
    // Reverse vectors after padding k with zeros
    while (k_bits.size() < h_bits.size())
        k_bits.push_back(false);

    std::reverse(k_bits.begin(), k_bits.end());
    std::reverse(h_bits.begin(), h_bits.end());

    // Purposefully skip first and last bit
    for (unsigned i = 1; i < h_bits.size() - 1; ++i) {
        // Double the point
        s = s + s;
        if ((h_bits[i]) && (!k_bits[i]))
            s = s + q;
        if ((!h_bits[i]) && (k_bits[i]))
            s = s - q;
    }

    return s;
}

template <typename Numeric>
Point<Numeric> operator* (const Numeric &lhs, const Point<Numeric> &rhs)
{
    return rhs * lhs;
}

template <typename Numeric>
Point<Numeric> makePointFromX (const Numeric &x, std::shared_ptr<Curve<Numeric>> ptr)
{
    Numeric y_square = x * x * x + ptr->a_ * x + ptr->b_;
    Numeric y = 0;
    return Point<Numeric>{x, y, ptr};
}

//Finding a generating point. Some minor changes /introduced features on the basis of the standard.
template <typename Numeric>
Point<Numeric> findGeneratingPointOnCurve(const Numeric &x,std::shared_ptr<Curve<Numeric>> ptr)
{
    // For prettier operation, make references to common variables
    Numeric &a = ptr->a_;
    Numeric &b = ptr->b_;
    Numeric &p = ptr->mod_;



    if (p < 3)
    {
      throw std::out_of_range{"in findGeneratingPointOnCurve() - invalid input invalid_argument"};
    }

    Numeric alpha;
    Numeric y;

    do {

        alpha = modulo((x * x * x + a * x + b),p);

        if (alpha == 0)
            return Point<Numeric> {x,0,ptr};

        y = squareRootsModPrime(p,alpha);
        if (y == -1)
            continue;
        if (modulo(y * y, p) == modulo(alpha, p)) {
            if (y > 0 && y < p)
            {
                return Point<Numeric> {x,y,ptr};
            }
        }
      } while(true);
}


} /* ec */

#endif /* end of include guard: POINT_ELLIPTIC_ECDHA_ITC_INCLUDED */
