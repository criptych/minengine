////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __MATH_HPP__
#define __MATH_HPP__ 1

////////////////////////////////////////////////////////////////////////////////

inline float quadratic(float t) {
    if (t >= 0.5f) {
        t = 1.0f - t;
        return 1.0f - (t * t);
    } else {
        return t * t;
    }
}

inline float cubic(float t) {
    if (t >= 0.5f) {
        t = 1.0f - t;
        return 1.0f - (t * t * t);
    } else {
        return t * t * t;
    }
}

inline float quartic(float t) {
    if (t >= 0.5f) {
        t = 1.0f - t;
        return 1.0f - (t * t * t * t);
    } else {
        return t * t * t * t;
    }
}

inline float quintic(float t) {
    if (t >= 0.5f) {
        t = 1.0f - t;
        return 1.0f - (t * t * t * t * t);
    } else {
        return t * t * t * t * t;
    }
}

template <typename T>
T lerp(float t, T start, T end) {
    return (1.0f - t) * start + t * end;
}

template <typename T>
T easeQuadratic(float t, T start, T end) {
    return lerp(quadratic(t), start, end);
}

template <typename T>
T easeCubic(float t, T start, T end) {
    return lerp(cubic(t), start, end);
}

template <typename T>
T easeQuartic(float t, T start, T end) {
    return lerp(quartic(t), start, end);
}

template <typename T>
T easeQuintic(float t, T start, T end) {
    return lerp(quintic(t), start, end);
}

////////////////////////////////////////////////////////////////////////////////

#endif // __MATH_HPP__

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

