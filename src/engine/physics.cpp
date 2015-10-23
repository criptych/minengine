////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#include "physics.hpp"

#include <SFML/System/Err.hpp>

////////////////////////////////////////////////////////////////////////////////

BoundingVolume::BoundingVolume(
    BoundingVolume::Type type,
    const Dimension &dimensions
): mType(type), mDimensions(dimensions) {
}

BoundingVolume::BoundingVolume(
): mType(), mDimensions() {
}

BoundingVolume::BoundingVolume(
    const Dimension &dimensions
): mType(AABB), mDimensions(dimensions) {
}

BoundingVolume::BoundingVolume(
    Size radius
): mType(Sphere), mDimensions(radius, radius, radius) {
}

BoundingVolume::BoundingVolume(
    Size radius,
    Size height
): mType(Capsule), mDimensions(radius, height, radius) {
}

BoundingVolume::Type BoundingVolume::getType() const {
    return mType;
}

const Dimension &BoundingVolume::getDimensions() const {
    return mDimensions;
}

Size BoundingVolume::getRadius() const {
    return mDimensions.x;
}

Size BoundingVolume::getHeight() const {
    return mDimensions.y;
}

BoundingVolume BoundingVolume::box(const Dimension &dimensions) {
    return BoundingVolume(dimensions);
}

BoundingVolume BoundingVolume::sphere(Size radius) {
    return BoundingVolume(radius);
}

BoundingVolume BoundingVolume::capsule(Size radius, Size height) {
    return BoundingVolume(radius, height);
}

////////////////////////////////////////////////////////////////////////////////

Physics::Physics(
): mGravity(0,-6,0) {
}

Physics::CollisionType Physics::checkCollision(
    const Body &a,
    const Body &b
) {
    const Position &pa = a.getPosition();
    const Position &pb = b.getPosition();
    const BoundingVolume &va = a.getBounds();
    const BoundingVolume &vb = b.getBounds();

    if (va.getType() == vb.getType()) {
        switch (va.getType()) {
            case BoundingVolume::AABB: {
                // box <-> box collision test
                Position minA = pa - Position(va.getDimensions());
                Position maxA = pa + Position(va.getDimensions());
                Position minB = pb - Position(vb.getDimensions());
                Position maxB = pb + Position(vb.getDimensions());

                if (
                    minA.x <= maxB.x + Epsilon && minA.y <= maxB.y + Epsilon && minA.z <= maxB.z + Epsilon &&
                    minB.x <= maxA.x + Epsilon && minB.y <= maxA.y + Epsilon && minB.z <= maxA.z + Epsilon
                ) {
                    if (
                        minA.x < maxB.x - Epsilon && minA.y < maxB.y - Epsilon && minA.z < maxB.z - Epsilon &&
                        minB.x < maxA.x - Epsilon && minB.y < maxA.y - Epsilon && minB.z < maxA.z - Epsilon
                    ) {
                        return CollisionType::Intrusion;
                    } else {
                        return CollisionType::Contact;
                    }
                } else {
                    return CollisionType::None;
                }
            }

            case BoundingVolume::Sphere: {
                // sphere <-> sphere collision test
                Position c = pb - pa;
                LargeDelta r = vb.getRadius() + va.getRadius();
                HugeDelta d = c.x * c.x + c.y * c.y + c.z * c.z - r * r;

                if (d >= Epsilon) {
                    return CollisionType::None;
                } else if (d > -Epsilon) {
                    return CollisionType::Contact;
                } else {
                    return CollisionType::Intrusion;
                }
            }

            case BoundingVolume::Capsule: {
                // capsule <-> capsule collision test
                Position c = pb - pa;
                LargeDelta r = vb.getRadius() + va.getRadius();
                //~ LargeDelta h = b.getHeight() + a.getHeight() - r;
                HugeDelta d = c.x * c.x + c.z * c.z - r * r;

                if (d >= Epsilon) {
                    // out of horizontal range, no need to check vertical
                    return CollisionType::None;
                } else if (d > -Epsilon) {
                    //! @todo vertical contact test
                    return CollisionType::Contact;
                } else {
                    //! @todo vertical intrusion test
                    return CollisionType::Intrusion;
                }
            }
        }
    } else {
        if (va.getType() == BoundingVolume::AABB && vb.getType() == BoundingVolume::Sphere) {
            //! @todo box <-> sphere collision test
        } else if (va.getType() == BoundingVolume::Sphere && vb.getType() == BoundingVolume::AABB) {
            //! @todo sphere <-> box collision test
        } else if (va.getType() == BoundingVolume::AABB && vb.getType() == BoundingVolume::Capsule) {
            //! @todo box <-> capsule collision test
        } else if (va.getType() == BoundingVolume::Capsule && vb.getType() == BoundingVolume::AABB) {
            //! @todo capsule <-> box collision test
        } else if (va.getType() == BoundingVolume::Sphere && vb.getType() == BoundingVolume::Capsule) {
            //! @todo sphere <-> capsule collision test
        } else if (va.getType() == BoundingVolume::Capsule && vb.getType() == BoundingVolume::Sphere) {
            //! @todo capsule <-> sphere collision test
        }
    }

    // no collision, or no test for given bounding volumes
    sf::err() << "no collision test for given bounding volumes (" <<
        va.getType() << ", " << vb.getType() << ")\n";
    return CollisionType::None;
}

void Physics::update(Body &b, const sf::Time &t) const {
    float s = t.asSeconds();
    b.mPosition += Position(b.mVelocity.x * s, b.mVelocity.y * s, b.mVelocity.z * s);
}

void Physics::accelerate(Body &b, const Velocity &v) const {
    b.mVelocity += v;
}

void Physics::gravitate(Body &b) const {
    accelerate(b, mGravity);
}

void Physics::impulse(Body &b, const Velocity &v, const sf::Time &t) const {
    float s = t.asSeconds();
    accelerate(b, Velocity(v.x * s, v.y * s, v.z * s));
}

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

