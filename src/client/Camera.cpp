////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#include "Camera.hpp"

#include <GL/glew.h>
#include "GLCheck.hpp"

////////////////////////////////////////////////////////////////////////////////

Camera::Camera(
): mFOV(75.0), mAspect(1.0), mZNear(0.1), mZFar(100.0), mNeedsUpdate(true) {
}

Camera::Camera(
    float fov,
    float aspect,
    float zNear,
    float zFar
): mFOV(fov), mAspect(aspect), mZNear(zNear), mZFar(zFar), mNeedsUpdate(true) {
}

Camera::~Camera() {
}

void Camera::setFOV(float fov) {
    mFOV = fov;
    mNeedsUpdate = true;
}

float Camera::getFOV() const {
    return mFOV;
}

void Camera::setAspect(float aspect) {
    mAspect = aspect;
    mNeedsUpdate = true;
}

float Camera::getAspect() const {
    return mAspect;
}

void Camera::setZNear(float zNear) {
    mZNear = zNear;
    mNeedsUpdate = true;
}

float Camera::getZNear() const {
    return mZNear;
}

void Camera::setZFar(float zFar) {
    mZFar = zFar;
    mNeedsUpdate = true;
}

float Camera::getZFar() const {
    return mZFar;
}

void Camera::setZRange(float zNear, float zFar) {
    mZNear = zNear;
    mZFar = zFar;
    mNeedsUpdate = true;
}

void Camera::setPosition(const sf::Vector3f &position) {
    mPosition = position;
    mNeedsUpdate = true;
}

void Camera::setPosition(float x, float y, float z) {
    setPosition(sf::Vector3f(x, y, z));
}

const sf::Vector3f &Camera::getPosition() const {
    return mPosition;
}

void Camera::move(const sf::Vector3f &offset) {
    setPosition(mPosition.x + offset.x,
                mPosition.y + offset.y,
                mPosition.z + offset.z);
}

void Camera::move(const sf::Vector3f &offset, float angle) {
    move(Transform3D().rotate(angle, sf::Vector3f(0, 1, 0)).transformPoint(offset));
}

void Camera::move(float dx, float dy, float dz) {
    move(sf::Vector3f(dx, dy, dz));
}

void Camera::setLook(const sf::Vector2f &look) {
    mLook = look;
    mNeedsUpdate = true;
}

const sf::Vector2f &Camera::getLook() const {
    return mLook;
}

const Transform3D &Camera::getTransform() const {
    if (mNeedsUpdate) {
        mTransform = Transform3D();
        mTransform.perspective(mFOV, mAspect, mZNear, mZFar);
        mTransform.rotate(mLook.y, sf::Vector3f(1.0f, 0.0f, 0.0f));
        mTransform.rotate(mLook.x, sf::Vector3f(0.0f, 1.0f, 0.0f));
        mTransform.translate(-sf::Vector3f(mPosition));
        mNeedsUpdate = false;
    }
    return mTransform;
}

void Camera::render() const {
    GLChecked(glMatrixMode(GL_PROJECTION));
    GLChecked(glLoadMatrixf(getTransform().getMatrix()));
    GLChecked(glMatrixMode(GL_MODELVIEW));
}

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

