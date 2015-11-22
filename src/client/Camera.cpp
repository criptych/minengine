////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#include "Camera.hpp"
#include "engine/engine.hpp"

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
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

const glm::mat4 &Camera::getTransform() const {
    if (mNeedsUpdate) {
        mTransform = glm::perspective(glm::radians(mFOV), mAspect, mZNear, mZFar);
        mNeedsUpdate = false;
    }
    return mTransform;
}

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

