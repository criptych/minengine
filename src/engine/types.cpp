////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#include "types.hpp"

#include <cmath>

////////////////////////////////////////////////////////////////////////////////

class TrigHelper {
    float sintbl[256];
    float tantbl[256];

public:
    TrigHelper() {
        for (unsigned i = 0; i < 256; i++) {
            sintbl[i] = std::sin(rad(i));
            tantbl[i] = std::tan(rad(i));
        }
    }

    float deg(int8_t x) {
        return x * (180.0 / 128.0);
    }

    float rad(int8_t x) {
        return x * (Pi / 128.0);
    }

    float sin(int8_t x) {
        return sintbl[(x & 255)];
    }

    float cos(int8_t x) {
        return sintbl[((64 - x) & 255)];
    }

    float tan(int8_t x) {
        return tantbl[(x & 255)];
    }

};

static TrigHelper sTrig;

////////////////////////////////////////////////////////////////////////////////

const Angle Angle::Zero(0);
const Angle Angle::Right(64);

float Angle::asDegrees() const {
    return sTrig.deg(mValue);
}

float Angle::asRadians() const {
    return sTrig.rad(mValue);
}

int8_t Angle::asByte() const {
    return mValue;
}

float Angle::sin() const {
    return sTrig.sin(mValue);
}

float Angle::cos() const {
    return sTrig.cos(mValue);
}

float Angle::tan() const {
    return sTrig.tan(mValue);
}

void Angle::sincos(float &s, float &c) const {
    s = sin();
    c = cos();
}

Angle Angle::fromDegrees(float angle) {
    return Angle(static_cast<int8_t>(angle * (128.0f / 180.0f)));
}

Angle Angle::fromRadians(float angle) {
    return Angle(static_cast<int8_t>(angle * (128.0f / Pi)));
}

Angle Angle::fromByte(int8_t angle) {
    return Angle(angle);
}

bool operator == (const Angle &a, const Angle &b) {
    return a.asByte() == b.asByte();
}

bool operator != (const Angle &a, const Angle &b) {
    return a.asByte() != b.asByte();
}

bool operator < (const Angle &a, const Angle &b) {
    return a.asByte() < b.asByte();
}

bool operator > (const Angle &a, const Angle &b) {
    return a.asByte() > b.asByte();
}

bool operator <= (const Angle &a, const Angle &b) {
    return a.asByte() <= b.asByte();
}

bool operator >= (const Angle &a, const Angle &b) {
    return a.asByte() >= b.asByte();
}

Angle operator + (const Angle &a) {
    return a;
}

Angle operator - (const Angle &a) {
    return Angle::fromByte(-a.asByte());
}

Angle operator + (const Angle &a, const Angle &b) {
    return Angle::fromByte(a.asByte() + b.asByte());
}

Angle operator - (const Angle &a, const Angle &b) {
    return Angle::fromByte(a.asByte() - b.asByte());
}

Angle &operator += (Angle &a, const Angle &b) {
    return a = a + b;
}

Angle &operator -= (Angle &a, const Angle &b) {
    return a = a - b;
}

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

