////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __PHYSICS_HPP__
#define __PHYSICS_HPP__ 1

////////////////////////////////////////////////////////////////////////////////

#include <SFML/System/Time.hpp>

#include "types.hpp"

////////////////////////////////////////////////////////////////////////////////

/**
 *  Axis-aligned box, used for physics simulation of (most) blocks.
 */
class BoundingVolume {
public:
    enum Type {
        AABB,
        Sphere,
        Capsule
    };

private:
    Type mType;
    Dimension mDimensions;

    BoundingVolume(BoundingVolume::Type type, const Dimension &dimensions);

public:
    BoundingVolume();
    explicit BoundingVolume(const Dimension &dimensions);
    explicit BoundingVolume(Size radius);
    BoundingVolume(Size radius, Size height);

    Type getType() const;
    const Dimension &getDimensions() const;

    Size getWidth() const; // for box
    Size getRadius() const; // for sphere and capsule
    Size getHeight() const; // for box and capsule
    Size getDepth() const; // for box

public:
    static BoundingVolume box(const Dimension &dimensions);
    static BoundingVolume sphere(Size radius);
    static BoundingVolume capsule(Size radius, Size height);
};

////////////////////////////////////////////////////////////////////////////////

class Physics {
    Velocity mGravity;

public:
    static const Size Epsilon = 3; // ~1%

    Physics();

    enum class CollisionType {
        None,       //!< Bounding volumes do not intersect
        Contact,    //!< Bounding volume surfaces intersect ("touch")
        Intrusion   //!< Bounding volume interiors intersect ("overlap")
    };

    class Body {
        Position mPosition;
        Velocity mVelocity;
        Size mMass;
        BoundingVolume mBounds;

        friend class Physics;

    public:
        const Position &getPosition() const {
            return mPosition;
        }

        void setPosition(const Position &position) {
            mPosition = position;
        }

        const Velocity &getVelocity() const {
            return mVelocity;
        }

        void setVelocity(const Velocity &velocity) {
            mVelocity = velocity;
        }

        Size getMass() const {
            return mMass;
        }

        void setMass(Size mass) {
            mMass = mass;
        }

        const BoundingVolume &getBounds() const {
            return mBounds;
        }

        void setBounds(const BoundingVolume &bounds) {
            mBounds = bounds;
        }
    };

    CollisionType checkCollision(const Body &a, const Body &b);

    void update(Body &b, const sf::Time &t) const;
    void accelerate(Body &b, const Velocity &v) const;
    void gravitate(Body &b) const;
    void impulse(Body &b, const Velocity &v, const sf::Time &t) const;
};

////////////////////////////////////////////////////////////////////////////////

#endif // __PHYSICS_HPP__

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

