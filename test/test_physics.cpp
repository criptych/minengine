////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#include "catch.hpp"

#include "engine/engine.hpp"

////////////////////////////////////////////////////////////////////////////////

SCENARIO("physics","[physics]") {

    Physics physics;

    GIVEN("Two AABB bodies of dimension 0.5 all directions") {
        Physics::Body a, b;
        a.setBounds(BoundingVolume::box(Dimension((1<<7),(1<<7),(1<<7))));
        b.setBounds(BoundingVolume::box(Dimension((1<<7),(1<<7),(1<<7))));

        CAPTURE(a.getBounds().getDimensions().x);
        CAPTURE(a.getBounds().getDimensions().y);
        CAPTURE(a.getBounds().getDimensions().z);
        CAPTURE(b.getBounds().getDimensions().x);
        CAPTURE(b.getBounds().getDimensions().y);
        CAPTURE(b.getBounds().getDimensions().z);

        GIVEN("A at (0,0,0), B at (1,1,1)") {
            a.setPosition(Position(0,0,0));
            b.setPosition(Position((1<<8),(1<<8),(1<<8)));

            CHECK(physics.checkCollision(a, b) == Physics::CollisionType::Contact);
        }

        GIVEN("a at (0,0,0), b approaching from +X") {
            a.setPosition(Position(0,0,0));

            for (int i = (1<<7)+Physics::Epsilon*2; i >= (1<<7)-Physics::Epsilon*2; i--) {
                CAPTURE(i);
                b.setPosition(Position(i,0,0));
                if (i < 256 - Physics::Epsilon) {
                    CHECK(physics.checkCollision(a, b) == Physics::CollisionType::Intrusion);
                } else if (i > 256 + Physics::Epsilon) {
                    CHECK(physics.checkCollision(a, b) == Physics::CollisionType::None);
                } else {
                    CHECK(physics.checkCollision(a, b) == Physics::CollisionType::Contact);
                }
            }
        }

        GIVEN("A at (0,0,0), B approaching from +Y") {
            a.setPosition(Position(0,0,0));

            for (int i = (1<<7)+Physics::Epsilon*2; i >= (1<<7)-Physics::Epsilon*2; i--) {
                CAPTURE(i);
                b.setPosition(Position(0,i,0));
                if (i < 256 - Physics::Epsilon) {
                    CHECK(physics.checkCollision(a, b) == Physics::CollisionType::Intrusion);
                } else if (i > 256 + Physics::Epsilon) {
                    CHECK(physics.checkCollision(a, b) == Physics::CollisionType::None);
                } else {
                    CHECK(physics.checkCollision(a, b) == Physics::CollisionType::Contact);
                }
            }
        }

        GIVEN("A at (0,0,0), B approaching from +Z") {
            a.setPosition(Position(0,0,0));

            for (int i = (1<<7)+Physics::Epsilon*2; i >= (1<<7)-Physics::Epsilon*2; i--) {
                CAPTURE(i);
                b.setPosition(Position(0,0,i));
                if (i < 256 - Physics::Epsilon) {
                    CHECK(physics.checkCollision(a, b) == Physics::CollisionType::Intrusion);
                } else if (i > 256 + Physics::Epsilon) {
                    CHECK(physics.checkCollision(a, b) == Physics::CollisionType::None);
                } else {
                    CHECK(physics.checkCollision(a, b) == Physics::CollisionType::Contact);
                }
            }
        }

        GIVEN("A at (0,0,0), B approaching from +XYZ") {
            a.setPosition(Position(0,0,0));

            for (int i = (1<<7)+Physics::Epsilon*2; i >= (1<<7)-Physics::Epsilon*2; i--) {
                CAPTURE(i);
                b.setPosition(Position(i,i,i));
                if (i < 256 - Physics::Epsilon) {
                    CHECK(physics.checkCollision(a, b) == Physics::CollisionType::Intrusion);
                } else if (i > 256 + Physics::Epsilon) {
                    CHECK(physics.checkCollision(a, b) == Physics::CollisionType::None);
                } else {
                    CHECK(physics.checkCollision(a, b) == Physics::CollisionType::Contact);
                }
            }
        }

    }
}

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

