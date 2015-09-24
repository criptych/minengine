////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#include "Transform3D.hpp"

#include "engine/engine.hpp"

////////////////////////////////////////////////////////////////////////////////

Transform3D::Transform3D(): Transform() {
}

Transform3D::Transform3D(
    float a00, float a01, float a02, float a03,
    float a10, float a11, float a12, float a13,
    float a20, float a21, float a22, float a23,
    float a30, float a31, float a32, float a33
) {
    float *m = const_cast<float*>(getMatrix());
    m[ 0] = a00; m[ 4] = a01; m[ 8] = a02; m[12] = a03;
    m[ 1] = a10; m[ 5] = a11; m[ 9] = a12; m[13] = a13;
    m[ 2] = a20; m[ 6] = a21; m[10] = a22; m[14] = a23;
    m[ 3] = a30; m[ 7] = a31; m[11] = a32; m[15] = a33;
}

Transform3D::Transform3D(const sf::Transform &transform) {
    const float *a = transform.getMatrix();
    float *b = const_cast<float*>(getMatrix());

    b[ 0] = a[ 0]; b[ 4] = a[ 4]; b[ 8] = a[ 8]; b[12] = a[12];
    b[ 1] = a[ 1]; b[ 5] = a[ 5]; b[ 9] = a[ 9]; b[13] = a[13];
    b[ 2] = a[ 2]; b[ 6] = a[ 6]; b[10] = a[10]; b[14] = a[14];
    b[ 3] = a[ 3]; b[ 7] = a[ 7]; b[11] = a[11]; b[15] = a[15];
}

Transform3D &Transform3D::combine(const sf::Transform& transform) {
    const float *a = getMatrix();
    const float *b = transform.getMatrix();

    *this = Transform3D(a[ 0] * b[ 0] + a[ 4] * b[ 1] + a[ 8] * b[ 2] + a[12] * b[ 3],
                        a[ 0] * b[ 4] + a[ 4] * b[ 5] + a[ 8] * b[ 6] + a[12] * b[ 7],
                        a[ 0] * b[ 8] + a[ 4] * b[ 9] + a[ 8] * b[10] + a[12] * b[11],
                        a[ 0] * b[12] + a[ 4] * b[13] + a[ 8] * b[14] + a[12] * b[15],
                        a[ 1] * b[ 0] + a[ 5] * b[ 1] + a[ 9] * b[ 2] + a[13] * b[ 3],
                        a[ 1] * b[ 4] + a[ 5] * b[ 5] + a[ 9] * b[ 6] + a[13] * b[ 7],
                        a[ 1] * b[ 8] + a[ 5] * b[ 9] + a[ 9] * b[10] + a[13] * b[11],
                        a[ 1] * b[12] + a[ 5] * b[13] + a[ 9] * b[14] + a[13] * b[15],
                        a[ 2] * b[ 0] + a[ 6] * b[ 1] + a[10] * b[ 2] + a[14] * b[ 3],
                        a[ 2] * b[ 4] + a[ 6] * b[ 5] + a[10] * b[ 6] + a[14] * b[ 7],
                        a[ 2] * b[ 8] + a[ 6] * b[ 9] + a[10] * b[10] + a[14] * b[11],
                        a[ 2] * b[12] + a[ 6] * b[13] + a[10] * b[14] + a[14] * b[15],
                        a[ 3] * b[ 0] + a[ 7] * b[ 1] + a[11] * b[ 2] + a[15] * b[ 3],
                        a[ 3] * b[ 4] + a[ 7] * b[ 5] + a[11] * b[ 6] + a[15] * b[ 7],
                        a[ 3] * b[ 8] + a[ 7] * b[ 9] + a[11] * b[10] + a[15] * b[11],
                        a[ 3] * b[12] + a[ 7] * b[13] + a[11] * b[14] + a[15] * b[15]);

    return *this;
}

sf::Vector3f Transform3D::transformPoint(const sf::Vector3f& p) {
    const float *a = getMatrix();
    float x = a[ 0] * p.x + a[ 4] * p.y + a[ 8] * p.z + a[12];
    float y = a[ 1] * p.x + a[ 5] * p.y + a[ 9] * p.z + a[13];
    float z = a[ 2] * p.x + a[ 6] * p.y + a[10] * p.z + a[14];
    float w = a[ 3] * p.x + a[ 7] * p.y + a[11] * p.z + a[15];
    return sf::Vector3f(x/w, y/w, z/w);
}

Transform3D &Transform3D::frustum(float left, float right, float bottom, float top, float near, float far) {
    Transform3D transform(
        (2 * near) / (right - left), 0.0f, (right + left) / (right - left), 0.0f,
        0.0f, (2 * near) / (top - bottom), (top + bottom) / (top - bottom), 0.0f,
        0.0f, 0.0f, (near + far) / (near - far), (2 * near * far) / (near - far),
        0.0f, 0.0f, -1.0f, 0.0f);

    return combine(transform);
}

Transform3D &Transform3D::perspective(float fov, float aspect, float near, float far) {
    float fh = std::tan(fov*Pi/360.0)*near;
    float fw = fh * aspect;
    return frustum(-fw, fw, -fh, fh, near, far);
}

Transform3D &Transform3D::lookAt(const sf::Vector3f &eye, const sf::Vector3f &target, const sf::Vector3f &up) {
    sf::Vector3f f, r, u;
    f = normalize(eye - target);
    r = normalize(cross(up, f));
    u = normalize(cross(f, r));

    Transform3D transform(r.x, u.x, f.x, 0,
                          r.y, u.y, f.y, 0,
                          r.z, u.z, f.z, 0,
                          0,   0,   0,   1);

    return combine(transform);
}

Transform3D &Transform3D::lookAt(const sf::Vector3f &eye, const sf::Vector3f &target) {
    return lookAt(eye, target, sf::Vector3f(0, 1, 0));
}

Transform3D &Transform3D::translate(const sf::Vector3f &offset) {
    Transform3D transform(1, 0, 0, offset.x,
                          0, 1, 0, offset.y,
                          0, 0, 1, offset.z,
                          0, 0, 0, 1);

    return combine(transform);
}

Transform3D &Transform3D::rotate(float angle, const sf::Vector3f &axis) {
    float s = std::sin(angle*Pi/180.0f), c = std::cos(angle*Pi/180.0f);
    float xx = axis.x * axis.x;
    float xy = axis.x * axis.y;
    float xz = axis.x * axis.z;
    float yy = axis.y * axis.y;
    float yz = axis.y * axis.z;
    float zz = axis.z * axis.z;
    float xs = axis.x * s;
    float ys = axis.y * s;
    float zs = axis.z * s;
    float mc = 1.0f - c;

    Transform3D transform(xx * mc +  c, xy * mc - zs, xz * mc + ys, 0,
                          xy * mc + zs, yy * mc +  c, yz * mc - xs, 0,
                          xz * mc - ys, yz * mc + xs, zz * mc +  c, 0,
                          0,        0,        0,        1);

    return combine(transform);
}

Transform3D Transform3D::getInverse() const {
    //! @todo
    return Transform3D();
}

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

