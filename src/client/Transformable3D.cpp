////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#include "Transformable3D.hpp"

////////////////////////////////////////////////////////////////////////////////

Transformable3D::Transformable3D(): mNeedsUpdate(true) {
}

const Transform3D &Transformable3D::getTransform() const {
    if (mNeedsUpdate) {
        mTransform = Transform3D();

        mNeedsUpdate = false;
    }
    return mTransform;
}

const Transform3D &Transformable3D::getInverseTransform() const {
    if (mInverseNeedsUpdate) {
        mInverseTransform = mTransform.getInverse();

        mInverseNeedsUpdate = false;
    }
    return mInverseTransform;
}

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

