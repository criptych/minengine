////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __CAMERA_HPP__
#define __CAMERA_HPP__ 1

////////////////////////////////////////////////////////////////////////////////

#include "Transform3D.hpp"

#include <SFML/System.hpp>

////////////////////////////////////////////////////////////////////////////////

class Camera {
    float mFOV;
    float mAspect;
    float mZNear;
    float mZFar;
    sf::Vector3f mPosition;
    sf::Vector2f mLook;

    mutable Transform3D mTransform;
    mutable bool mNeedsUpdate;

public:
    Camera();
    Camera(float fov, float aspect, float zNear, float zFar);
    ~Camera();

    void setFOV(float fov);
    float getFOV() const;

    void setAspect(float aspect);
    float getAspect() const;

    void setZNear(float zNear);
    float getZNear() const;
    void setZFar(float zFar);
    float getZFar() const;
    void setZRange(float zNear, float zFar);

    void setPosition(const sf::Vector3f &position);
    void setPosition(float x, float y, float z);
    const sf::Vector3f &getPosition() const;

    void move(const sf::Vector3f &offset);
    void move(const sf::Vector3f &offset, float angle);
    void move(float dx, float dy, float dz);

    void setLook(const sf::Vector2f &look);
    const sf::Vector2f &getLook() const;

    const Transform3D &getTransform() const;

    void render() const;
};

////////////////////////////////////////////////////////////////////////////////

#endif // __CAMERA_HPP__

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

