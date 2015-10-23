////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __TYPES_HPP__
#define __TYPES_HPP__ 1

////////////////////////////////////////////////////////////////////////////////

/**
 *  A note on coordinates and angles:
 *   o  Chunk coordinates are signed 64-bit integer values, in meters/16.
 *   o  Block coordinates are signed 64-bit integer values, in meters.
 *   o  Entity coordinates are signed 64-bit fixed-point values with
 *      8 fractional bits, in meters.
 *   o  Velocities are signed 16-bit fixed-point values with 8 fractional bits,
 *      in meters/tick.
 *   o  Sizes are unsigned 16-bit fixed-point values with 8 fractional bits,
 *      in meters.
 *   o  Angles are signed 8-bit integer values, in a 256-point scale analogous
 *      to degrees, with -128 = 180deg.
 *
 *  These formats were chosen to allow high-performance integer calculations
 *  for most operations and compact representation for transmission between
 *  client and server, with enough precision for reasonably smooth physics.
 *  The format of angles in particular allows for efficient wraparound handling
 *  and implementing most trigonometry operations as simple table lookups.
 */

////////////////////////////////////////////////////////////////////////////////

#include <cinttypes>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>

////////////////////////////////////////////////////////////////////////////////

typedef uint64_t EntityID;

const long double Pi    = 3.141592653589793238462643383279;
const long double TwoPi = 6.283185307179586476925286766559;

class Angle {
    int8_t mValue;

    explicit Angle(int8_t value): mValue(value) {}

public:
    Angle(): mValue() {}

    float asDegrees() const;
    float asRadians() const;
    int8_t asByte() const;

    float sin() const;
    float cos() const;
    float tan() const;

    void sincos(float &s, float &c) const;

    static const Angle Zero;
    static const Angle Right;

    static Angle fromDegrees(float angle);
    static Angle fromRadians(float angle);
    static Angle fromByte(int8_t angle);
};

bool operator == (const Angle &a, const Angle &b);
bool operator != (const Angle &a, const Angle &b);
bool operator <  (const Angle &a, const Angle &b);
bool operator >  (const Angle &a, const Angle &b);
bool operator <= (const Angle &a, const Angle &b);
bool operator >= (const Angle &a, const Angle &b);

Angle operator + (const Angle &a);
Angle operator - (const Angle &a);
Angle operator + (const Angle &a, const Angle &b);
Angle operator - (const Angle &a, const Angle &b);

Angle &operator += (Angle &a, const Angle &b);
Angle &operator -= (Angle &a, const Angle &b);

////////////////////////////////////////////////////////////////////////////////

typedef int64_t Coord;
typedef uint16_t Size;
typedef int16_t Delta;
typedef int32_t LargeDelta;
typedef int64_t HugeDelta;

typedef sf::Vector2<Angle> Orientation;
typedef sf::Vector3<Coord> Position;
typedef sf::Vector3<Size>  Dimension;
typedef sf::Vector3<Delta> Velocity;
typedef sf::Vector3<LargeDelta> Acceleration;
typedef sf::Vector3<LargeDelta> Force;

////////////////////////////////////////////////////////////////////////////////

#endif // __TYPES_HPP__

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

