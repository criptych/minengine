////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __TRANSFORM3D_HPP__
#define __TRANSFORM3D_HPP__ 1

////////////////////////////////////////////////////////////////////////////////

#include <SFML/Graphics/Transform.hpp>
#include <SFML/System.hpp>

////////////////////////////////////////////////////////////////////////////////

class Transform3D : public sf::Transform {
public:
    Transform3D();

    Transform3D(
        float a00, float a01, float a02, float a03,
        float a10, float a11, float a12, float a13,
        float a20, float a21, float a22, float a23,
        float a30, float a31, float a32, float a33
    );

    Transform3D(const sf::Transform &transform);

    Transform3D &combine(const sf::Transform& transform);
    sf::Vector3f transformPoint(const sf::Vector3f &point);

    Transform3D &frustum(float left, float right, float bottom, float top, float near, float far);
    Transform3D &perspective(float fov, float aspect, float near, float far);
    Transform3D &lookAt(const sf::Vector3f &eye, const sf::Vector3f &target, const sf::Vector3f &up);
    Transform3D &lookAt(const sf::Vector3f &eye, const sf::Vector3f &target);

    Transform3D &translate(const sf::Vector3f &offset);
    Transform3D &rotate(float angle, const sf::Vector3f &axis);
    Transform3D getInverse() const;
};

////////////////////////////////////////////////////////////////////////////////

#endif // __TRANSFORM3D_HPP__

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

