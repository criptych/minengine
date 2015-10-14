////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#include "engine.hpp"

#include <cstdio>
#include <cmath>

////////////////////////////////////////////////////////////////////////////////

class TrigHelper {
    float sintbl[256];
    float tantbl[256];

public:
    TrigHelper() {
        for (unsigned i = 0; i < 256; i++) {
            sintbl[i] = std::sin(rad(i));
            tantbl[i] = std::tan(rad(i));
        }
    }

    float deg(int8_t x) {
        return x * (180.0 / 128.0);
    }

    float rad(int8_t x) {
        return x * (Pi / 128.0);
    }

    float sin(int8_t x) {
        return sintbl[(x & 255)];
    }

    float cos(int8_t x) {
        return sintbl[((64 - x) & 255)];
    }

    float tan(int8_t x) {
        return tantbl[(x & 255)];
    }

};

static TrigHelper sTrig;

////////////////////////////////////////////////////////////////////////////////

const Angle Angle::Zero(0);
const Angle Angle::Right(64);

float Angle::asDegrees() const {
    return sTrig.deg(mValue);
}

float Angle::asRadians() const {
    return sTrig.rad(mValue);
}

int8_t Angle::asByte() const {
    return mValue;
}

float Angle::sin() const {
    return sTrig.sin(mValue);
}

float Angle::cos() const {
    return sTrig.cos(mValue);
}

float Angle::tan() const {
    return sTrig.tan(mValue);
}

void Angle::sincos(float &s, float &c) const {
    s = sin();
    c = cos();
}

Angle Angle::fromDegrees(float angle) {
    return Angle(static_cast<int8_t>(angle * (128.0f / 180.0f)));
}

Angle Angle::fromRadians(float angle) {
    return Angle(static_cast<int8_t>(angle * (128.0f / Pi)));
}

Angle Angle::fromByte(int8_t angle) {
    return Angle(angle);
}

bool operator == (const Angle &a, const Angle &b) {
    return a.asByte() == b.asByte();
}

bool operator != (const Angle &a, const Angle &b) {
    return a.asByte() != b.asByte();
}

bool operator < (const Angle &a, const Angle &b) {
    return a.asByte() < b.asByte();
}

bool operator > (const Angle &a, const Angle &b) {
    return a.asByte() > b.asByte();
}

bool operator <= (const Angle &a, const Angle &b) {
    return a.asByte() <= b.asByte();
}

bool operator >= (const Angle &a, const Angle &b) {
    return a.asByte() >= b.asByte();
}

Angle operator + (const Angle &a) {
    return a;
}

Angle operator - (const Angle &a) {
    return Angle::fromByte(-a.asByte());
}

Angle operator + (const Angle &a, const Angle &b) {
    return Angle::fromByte(a.asByte() + b.asByte());
}

Angle operator - (const Angle &a, const Angle &b) {
    return Angle::fromByte(a.asByte() - b.asByte());
}

Angle &operator += (Angle &a, const Angle &b) {
    return a = a + b;
}

Angle &operator -= (Angle &a, const Angle &b) {
    return a = a - b;
}

////////////////////////////////////////////////////////////////////////////////

namespace std {
    template <typename T>
    bool operator< (const sf::Vector3<T> &a, const sf::Vector3<T> &b) {
        if (a.z == b.z) {
            if (a.y == b.y) {
                return a.x < b.x;
            }
            return a.y < b.y;
        }
        return a.z < b.z;
    }

    template <typename T>
    bool operator< (const sf::Vector2<T> &a, const sf::Vector2<T> &b) {
        if (a.y == b.y) {
            return a.x < b.x;
        }
        return a.y < b.y;
    }
}

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

enum GLPrimitive {
    GLPoints,
    GLLines,
    GLLineLoop,
    GLLineStrip,
    GLTriangles,
    GLTriangleStrip,
    GLTriangleFan,
    GLQuads,
    GLQuadStrip,
    GLPolygon,
};

Model::Model(
): mPrimitive(GLPoints) {
}

Model::Model(
    uint32_t primitive
): mPrimitive(primitive) {
}

uint32_t Model::getPrimitive() const {
    return mPrimitive;
}

const std::vector<Vertex> &Model::getVertices() const {
    return mVertices;
}

std::vector<Vertex> &Model::getVertices() {
    return mVertices;
}

void Model::setPrimitive(uint32_t primitive) {
    mPrimitive = primitive;
}

void Model::clearVertices() {
    mVertices.clear();
}

void Model::addVertex(const Vertex &vertex) {
    mVertices.push_back(vertex);
}

void Model::calcNormals(bool smooth) {
    calcNormals(0, mVertices.size(), smooth);
}

void Model::calcNormals(size_t start, size_t end, bool smooth) {
    if (end > mVertices.size()) {
        end = mVertices.size();
    }
    if (start > end) {
        return;
    }

    sf::err() << "start == " << start << ", end == " << end << '\n';

    switch (mPrimitive) {
        case GLTriangles: {
            for (size_t i = start; i < end; i += 3) {
                sf::Vector3f p[3], n[3];

                for (size_t j = 0; j < 3; j++) {
                    p[j] = mVertices[i+j].position;
                }

                for (size_t j = 0; j < 3; j++) {
                    n[j] = normalize(cross(p[(j+1)%3]-p[(j+0)%3],
                                           p[(j+2)%3]-p[(j+0)%3]));
                }

                for (size_t j = 0; j < 3; j++) {
                    mVertices[i+j].normal = n[j];
                }
            }
            break;
        }

        case GLTriangleFan: {
            for (size_t i = start; i < end; i += 1) {
                sf::Vector3f p[3], n[3];

                for (size_t j = 1; j < 3; j++) {
                    p[j] = mVertices[i+j].position;
                }
                p[0] = mVertices[start].position;

                for (size_t j = 0; j < 3; j++) {
                    n[j] = normalize(cross(p[(j+1)%3]-p[(j+0)%3],
                                           p[(j+2)%3]-p[(j+0)%3]));
                }

                for (size_t j = 0; j < 4; j++) {
                    mVertices[i+j].normal = n[j];
                }
            }
            break;
        }

        case GLTriangleStrip: {
            end -= 2;
            for (size_t i = start; i < end; i += 1) {
                sf::Vector3f p[3], n[3];

                for (size_t j = 0; j < 3; j++) {
                    p[j] = mVertices[i+j].position;
                }

                if ((i - start) % 2 == 0) {
                    for (size_t j = 0; j < 3; j++) {
                        n[j] = normalize(cross(p[(j+1)%3]-p[(j+0)%3],
                                               p[(j+2)%3]-p[(j+0)%3]));
                    }
                } else {
                    for (size_t j = 0; j < 3; j++) {
                        n[j] = normalize(cross(p[(j+2)%3]-p[(j+0)%3],
                                               p[(j+1)%3]-p[(j+0)%3]));
                    }
                }

                for (size_t j = 0; j < 3; j++) {
                    mVertices[i+j].normal = n[j];
                }
            }
            break;
        }

        case GLQuads: {
            for (size_t i = start; i < end; i += 4) {
                sf::Vector3f p[4], n[4];

                for (size_t j = 0; j < 4; j++) {
                    p[j] = mVertices[i+j].position;
                }

                for (size_t j = 0; j < 4; j++) {
                    n[j] = normalize(cross(p[(j+1)%4]-p[(j+0)%4],
                                           p[(j+3)%4]-p[(j-0)%4]));
                }

                for (size_t j = 0; j < 4; j++) {
                    mVertices[i+j].normal = n[j];
                }
            }
            break;
        }

        case GLQuadStrip: {
            end -= 2;
            for (size_t i = start; i < end; i += 2) {
                sf::Vector3f p[4], n[4];

                for (size_t j = 0; j < 4; j++) {
                    p[j] = mVertices[i+j].position;
                }

                for (size_t j = 0; j < 4; j++) {
                    n[j] = normalize(cross(p[(j+1)%4]-p[(j+0)%4],
                                           p[(j+3)%4]-p[(j-0)%4]));
                }

                for (size_t j = 0; j < 4; j++) {
                    mVertices[i+j].normal = n[j];
                }
            }
            break;
        }
    }

    sf::err() << std::flush;
}

void Model::makeBox(const sf::Vector3f &size, const sf::Vector3f &center) {
    sf::Vector3f mx = center + size;
    sf::Vector3f mn = center - size;

    clearVertices();

    switch (mPrimitive) {
        default: {
            mPrimitive = GLTriangles;
            // continue
        }

        case GLTriangles: {
            addVertex(Vertex(0x7fff,0x0000, mx.x,mx.y,mn.z));
            addVertex(Vertex(0x7fff,0x7fff, mx.x,mx.y,mx.z));
            addVertex(Vertex(0x0000,0x7fff, mx.x,mn.y,mx.z));
            addVertex(Vertex(0x7fff,0x0000, mx.x,mx.y,mn.z));
            addVertex(Vertex(0x0000,0x7fff, mx.x,mn.y,mx.z));
            addVertex(Vertex(0x0000,0x0000, mx.x,mn.y,mn.z));

            addVertex(Vertex(0x0000,0x0000, mn.x,mn.y,mn.z));
            addVertex(Vertex(0x0000,0x7fff, mn.x,mn.y,mx.z));
            addVertex(Vertex(0x7fff,0x7fff, mn.x,mx.y,mx.z));
            addVertex(Vertex(0x0000,0x0000, mn.x,mn.y,mn.z));
            addVertex(Vertex(0x7fff,0x7fff, mn.x,mx.y,mx.z));
            addVertex(Vertex(0x7fff,0x0000, mn.x,mx.y,mn.z));

            addVertex(Vertex(0x0000,0x0000, mn.x,mx.y,mn.z));
            addVertex(Vertex(0x0000,0x7fff, mn.x,mx.y,mx.z));
            addVertex(Vertex(0x7fff,0x7fff, mx.x,mx.y,mx.z));
            addVertex(Vertex(0x0000,0x0000, mn.x,mx.y,mn.z));
            addVertex(Vertex(0x7fff,0x7fff, mx.x,mx.y,mx.z));
            addVertex(Vertex(0x7fff,0x0000, mx.x,mx.y,mn.z));

            addVertex(Vertex(0x7fff,0x0000, mx.x,mn.y,mn.z));
            addVertex(Vertex(0x7fff,0x7fff, mx.x,mn.y,mx.z));
            addVertex(Vertex(0x0000,0x7fff, mn.x,mn.y,mx.z));
            addVertex(Vertex(0x7fff,0x0000, mx.x,mn.y,mn.z));
            addVertex(Vertex(0x0000,0x7fff, mn.x,mn.y,mx.z));
            addVertex(Vertex(0x0000,0x0000, mn.x,mn.y,mn.z));

            addVertex(Vertex(0x7fff,0x0000, mx.x,mn.y,mx.z));
            addVertex(Vertex(0x7fff,0x7fff, mx.x,mx.y,mx.z));
            addVertex(Vertex(0x0000,0x7fff, mn.x,mx.y,mx.z));
            addVertex(Vertex(0x7fff,0x0000, mx.x,mn.y,mx.z));
            addVertex(Vertex(0x0000,0x7fff, mn.x,mx.y,mx.z));
            addVertex(Vertex(0x0000,0x0000, mn.x,mn.y,mx.z));

            addVertex(Vertex(0x0000,0x0000, mn.x,mn.y,mn.z));
            addVertex(Vertex(0x0000,0x7fff, mn.x,mx.y,mn.z));
            addVertex(Vertex(0x7fff,0x7fff, mx.x,mx.y,mn.z));
            addVertex(Vertex(0x0000,0x0000, mn.x,mn.y,mn.z));
            addVertex(Vertex(0x7fff,0x7fff, mx.x,mx.y,mn.z));
            addVertex(Vertex(0x7fff,0x0000, mx.x,mn.y,mn.z));
            break;
        }

        case GLQuads: {
            addVertex(Vertex(0x7fff,0x0000, mx.x,mx.y,mn.z));
            addVertex(Vertex(0x7fff,0x7fff, mx.x,mx.y,mx.z));
            addVertex(Vertex(0x0000,0x7fff, mx.x,mn.y,mx.z));
            addVertex(Vertex(0x0000,0x0000, mx.x,mn.y,mn.z));

            addVertex(Vertex(0x0000,0x0000, mn.x,mn.y,mn.z));
            addVertex(Vertex(0x0000,0x7fff, mn.x,mn.y,mx.z));
            addVertex(Vertex(0x7fff,0x7fff, mn.x,mx.y,mx.z));
            addVertex(Vertex(0x7fff,0x0000, mn.x,mx.y,mn.z));

            addVertex(Vertex(0x0000,0x0000, mn.x,mx.y,mn.z));
            addVertex(Vertex(0x0000,0x7fff, mn.x,mx.y,mx.z));
            addVertex(Vertex(0x7fff,0x7fff, mx.x,mx.y,mx.z));
            addVertex(Vertex(0x7fff,0x0000, mx.x,mx.y,mn.z));

            addVertex(Vertex(0x7fff,0x0000, mx.x,mn.y,mn.z));
            addVertex(Vertex(0x7fff,0x7fff, mx.x,mn.y,mx.z));
            addVertex(Vertex(0x0000,0x7fff, mn.x,mn.y,mx.z));
            addVertex(Vertex(0x0000,0x0000, mn.x,mn.y,mn.z));

            addVertex(Vertex(0x7fff,0x0000, mx.x,mn.y,mx.z));
            addVertex(Vertex(0x7fff,0x7fff, mx.x,mx.y,mx.z));
            addVertex(Vertex(0x0000,0x7fff, mn.x,mx.y,mx.z));
            addVertex(Vertex(0x0000,0x0000, mn.x,mn.y,mx.z));

            addVertex(Vertex(0x0000,0x0000, mn.x,mn.y,mn.z));
            addVertex(Vertex(0x0000,0x7fff, mn.x,mx.y,mn.z));
            addVertex(Vertex(0x7fff,0x7fff, mx.x,mx.y,mn.z));
            addVertex(Vertex(0x7fff,0x0000, mx.x,mn.y,mn.z));
            break;
        }
    }

    calcNormals();
}

void Model::makeBox(const sf::Vector3f &size) {
    makeBox(sf::Vector3f(), size);
}

void Model::makeBox() {
    makeBox(sf::Vector3f(1,1,1));
}

static void addPolarVertex(Model &model, const sf::Vector3f &center, float t, float p, float r) {
    float ct = std::cos(t);
    float st = std::sin(t);
    float cp = std::cos(p);
    float sp = std::sin(p);
    sf::Vector3f n(sp*ct, cp, sp*st);
    model.addVertex(Vertex(n, center + r * n));
}

/*

void hacerEsfera (float radio, int divO, int divA) {
    float px, py,pz;
    int i,j;
    float incO = 2*M_PI / divO;
    float incA = M_PI /divA;

    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_TRIANGLE_STRIP);
    glColor3i(1,0,0);

    //Depende del polo en el que empezemos
    for (i= 0 ; i<= divO; i++){
        for (j = 0; j<=divA ; j++) {

            if (i % 2 == 0){

                pz = cos (M_PI-(incA*j))*radio;
                py = sin (M_PI-(incA*j))*sin (incO*i)*radio;
                px = sin (M_PI-(incA*j))*cos (incO*i)*radio;

                glVertex3f (px, py, pz);

                pz = cos (M_PI-(incA*j))*radio;
                py = sin (M_PI-(incA*j))*sin (incO*(i+1))*radio;
                px = sin (M_PI-(incA*j))*cos (incO*(i+1))*radio;

                glVertex3f (px, py, pz);
            }

            else {
                pz = cos (incA*j)*radio;
                py = sin (M_PI-(incA*j))*sin (incO*i)*radio;
                px = sin (M_PI-(incA*j))*cos (incO*i)*radio;
                glVertex3f (px, py, pz);

                pz = cos (incA*j)*radio;
                py = sin (incA*j)*sin (incO*(i+1))*radio;
                px = sin (incA*j)*cos (incO*(i+1))*radio;
                glVertex3f (px, py, pz);
            }
        }
    }
    glEnd();
    glFlush();
}

 */

void Model::makeBall(float radius, size_t step, const sf::Vector3f &center) {
    if (step < 2) {
        step = 2;
    }
    size_t rstep = 2 * step;
    //~ size_t n = (step + 1) * rstep;

    float phi = 0, theta = 0, dPhi = Pi / (step), dTheta = 2.0f * Pi / rstep;

    clearVertices();
    setPrimitive(GLTriangleStrip);

    //~ float ct, st, cp, sp;
    //~ sf::Vector3f n;

/*
#define VERTEX(T,P,R) ( \
    ct = std::cos((T)), st = std::sin((T)), \
    cp = std::cos((P)), sp = std::sin((P)), \
    n.x = sp*ct, n.y = cp, n.z = sp*st, \
    addVertex(Vertex(n, center + (R) * n)) )
*/

    //~ size_t n = step * rstep + 1;

    //~ addPolarVertex(*this, center, 0, 0, radius);
    //~ mVertices.back().color = sf::Color::Red;

    for (size_t i = 0; i < rstep; i++, theta += dTheta) {
        for (size_t j = 0; j < step; j++, phi += dPhi) {

            sf::Vector3f p;

            //~ if (i % 2 == 0){

                p.z = std::cos (Pi-(dPhi*j))*radius;
                p.y = std::sin (Pi-(dPhi*j))*std::sin (dTheta*i)*radius;
                p.x = std::sin (Pi-(dPhi*j))*std::cos (dTheta*i)*radius;
                addVertex(Vertex(p));

                p.z = std::cos (Pi-(dPhi*j))*radius;
                p.y = std::sin (Pi-(dPhi*j))*std::sin (dTheta*(i+1))*radius;
                p.x = std::sin (Pi-(dPhi*j))*std::cos (dTheta*(i+1))*radius;
                addVertex(Vertex(p));
            //~ }

            //~ else {
                //~ p.z = std::cos (dPhi*j)*radius;
                //~ p.y = std::sin (Pi-(dPhi*j))*std::sin (dTheta*i)*radius;
                //~ p.x = std::sin (Pi-(dPhi*j))*std::cos (dTheta*i)*radius;
                //~ addVertex(Vertex(p));

                //~ p.z = std::cos (dPhi*j)*radius;
                //~ p.y = std::sin (dPhi*j)*std::sin (dTheta*(i+1))*radius;
                //~ p.x = std::sin (dPhi*j)*std::cos (dTheta*(i+1))*radius;
                //~ addVertex(Vertex(p));
            //~ }


            //~ addPolarVertex(*this, center, theta, phi, radius);
            //~ mVertices.back().color = sf::Color(255*(1.0f-t),255*t,0);
            //~ addPolarVertex(*this, center, theta-d, phi+dPhi, radius);
            //~ mVertices.back().color = sf::Color(255*(1.0f-t),255*t,0);
        }
    }

    addPolarVertex(*this, center, 0, phi, radius);
    mVertices.back().color = sf::Color::Green;

/*

    //~ addVertex(Vertex(sf::Vector3f(0,-1,0), sf::Vector3f(
        //~ center.x, center.y-size, center.z
    //~ )));

    for (size_t j = 0; j < step; j++) {
        //~ theta = -(j + 0.5f) * dTheta;

        for (size_t i = 0; i < rstep; i++) {
            theta = static_cast<float>(i)/static_cast<float>(rstep) -
                0.5f * static_cast<float>(j)/static_cast<float>(step);
            phi = static_cast<float>(j)/static_cast<float>(step);

            ct = std::cos(theta-dTheta);
            st = std::sin(theta-dTheta);

            cp = std::cos(phi+dPhi);
            sp = std::sin(phi+dPhi);

            x = sp*ct; y = cp; z = sp*st;

            addVertex(Vertex(sf::Vector3f(x, y, z), sf::Vector3f(
                center.x+size*x, center.y-size*y, center.z+size*z
            )));

            ct = std::cos(theta);
            st = std::sin(theta);

            cp = std::cos(phi);
            sp = std::sin(phi);

            x = sp*ct; y = cp; z = sp*st;

            addVertex(Vertex(sf::Vector3f(x, y, z), sf::Vector3f(
                center.x+size*x, center.y-size*y, center.z+size*z
            )));

            //~ theta += dTheta;
        }

        //~ ct = std::cos(theta-0.5*dTheta);
        //~ st = std::sin(theta-0.5*dTheta);

        //~ cp = std::cos(phi+dPhi);
        //~ sp = std::sin(phi+dPhi);

        //~ x = sp*ct; y = cp; z = sp*st;

        //~ addVertex(Vertex(sf::Vector3f(x, y, z), sf::Vector3f(
            //~ center.x+size*x, center.y-size*y, center.z+size*z
        //~ )));

        //~ ct = std::cos(theta);
        //~ st = std::sin(theta);

        //~ cp = std::cos(phi);
        //~ sp = std::sin(phi);

        //~ x = sp*ct; y = cp; z = sp*st;

        //~ addVertex(Vertex(sf::Vector3f(x, y, z), sf::Vector3f(
            //~ center.x+size*x, center.y-size*y, center.z+size*z
        //~ )));

        //~ phi += dPhi;
    }

    //~ addVertex(Vertex(sf::Vector3f(0,1,0), sf::Vector3f(
        //~ center.x, center.y+size, center.z
    //~ )));

    //~ calcNormals();
*/

}

void Model::makeBall(float size, size_t step) {
    makeBall(size, step, sf::Vector3f());
}

////////////////////////////////////////////////////////////////////////////////

EntityID Entity::getID() const {
    return mID;
}

const Position &Entity::getPosition() const {
    return mPosition;
}

const Velocity &Entity::getVelocity() const {
    return mVelocity;
}

const Orientation &Entity::getOrientation() const {
    return mOrientation;
}

void Entity::update() {
    mPosition.x += mVelocity.x;
    mPosition.y += mVelocity.y;
    mPosition.z += mVelocity.z;
}

////////////////////////////////////////////////////////////////////////////////

ChunkData::ChunkData() {
    std::fill(mBlockType, std::end(mBlockType), 0);
    std::fill(mBlockData, std::end(mBlockData), 0);
    std::fill(mLightData, std::end(mLightData), 255);
}


////////////////////////////////////////////////////////////////////////////////

Chunk *ChunkGenerator::loadChunk(Chunk &chunk, const Position &position) {
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////

ChunkCache::ChunkCache(
    ChunkSource *source, size_t capacity
): mSource(source) {
    mChunkData.reserve(capacity);
}

ChunkCache::ChunkCache(
    ChunkSource &source, size_t capacity
): mSource(&source) {
    mChunkData.reserve(capacity);
}

Chunk *ChunkCache::getChunk(const Position &position) {
    auto i = mChunkMap.find(position);
    if (i != mChunkMap.end()) {
        return i->second;
    } else {
        ChunkData *data;
        if (mChunkData.size() < mChunkData.capacity()) {
            mChunkData.resize(mChunkData.size() + 1);
            data = &mChunkData.back();
        } else {
            Position oldPosition = mChunks.front().getPosition();
            fprintf(stderr, "unload chunk @ <%lld,%lld,%lld>\n",
                    oldPosition.x, oldPosition.y, oldPosition.z);
            // too many chunks in cache; remove the oldest
            data = mChunks.front().getData();
            mChunkMap.erase(oldPosition);
            mChunks.pop_front();
        }
        //~ fprintf(stderr, "load chunk @ <%lld,%lld,%lld>\n", position.x, position.y, position.z);
        mChunks.push_back(Chunk(position, data));
        Chunk &chunk = mChunks.back();
        mChunkMap.insert({position, &chunk});
        mSource->loadChunk(chunk, position);
        return &chunk;
    }
}

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

