////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#include "Camera.hpp"
#include "engine/engine.hpp"

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

const sf::Transform3D &Camera::getTransform() const {
    if (mNeedsUpdate) {
        float zn = mZNear;
        float zf = mZFar;
        float fh = std::tan(mFOV*Pi/360.0)*zn;
        float fw = fh * mAspect;

        mTransform = sf::Transform3D(
            (2 * zn) / (2 * fw), 0.0f, 0.0f, 0.0f,
            0.0f, (2 * zn) / (2 * fh), 0.0f, 0.0f,
            0.0f, 0.0f, (zn + zf) / (zn - zf), (2 * zn * zf) / (zn - zf),
            0.0f, 0.0f, -1.0f, 0.0f);
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

