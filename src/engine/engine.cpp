////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#include "engine.hpp"

#include <cstdio>

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
        //~ return sin(x)/cos(x);
        return tantbl[(x & 255)];
    }

};

static TrigHelper sTrig;

////////////////////////////////////////////////////////////////////////////////

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

Box::Box(): mDimensions() {
}

Box::Box(
    const Dimension &dim
): mDimensions(dim) {
}

const Dimension &Box::getDimensions() const {
    return mDimensions;
}

////////////////////////////////////////////////////////////////////////////////

Sphere::Sphere(): mRadius() {
}

Sphere::Sphere(
    Size radius
): mRadius(radius) {
}

Size Sphere::getRadius() const {
    return mRadius;
}

////////////////////////////////////////////////////////////////////////////////

Capsule::Capsule(): mRadius(), mHeight() {
}

Capsule::Capsule(
    Size radius, Size height
): mRadius(radius), mHeight(height) {
}

Size Capsule::getRadius() const {
    return mRadius;
}

Size Capsule::getHeight() const {
    return mHeight;
}

////////////////////////////////////////////////////////////////////////////////

const Position &Physics::Body::getPosition() const {
    return mPosition;
}

const Velocity &Physics::Body::getVelocity() const {
    return mVelocity;
}

Size Physics::Body::getMass() const {
    return mMass;
}

////////////////////////////////////////////////////////////////////////////////

Physics::Physics(
): mGravity(0,-6,0) {
}

Physics::CollisionType Physics::checkCollision(
    const Position &pa,
    const Box &a,
    const Position &pb,
    const Box &b
) {
    Position minA = pa - Position(a.getDimensions());
    Position maxA = pa + Position(a.getDimensions());
    Position minB = pb - Position(b.getDimensions());
    Position maxB = pb + Position(b.getDimensions());

    if (
        minA.x <= maxB.x && minA.y <= maxB.y && minA.z <= maxB.z &&
        minB.x <= maxA.x && minB.y <= maxA.y && minB.z <= maxA.z
    ) {

        if (
            minA.x < maxB.x && minA.y < maxB.y && minA.z < maxB.z &&
            minB.x < maxA.x && minB.y < maxA.y && minB.z < maxA.z
        ) {
            return CollisionType::Intrusion;
        }

        return CollisionType::Contact;
    }

    return CollisionType::None;
}

Physics::CollisionType Physics::checkCollision(
    const Position &pa,
    const Box &a,
    const Position &pb,
    const Sphere &b
) {
    //! @todo
    return CollisionType::None;
}

Physics::CollisionType Physics::checkCollision(
    const Position &pa,
    const Box &a,
    const Position &pb,
    const Capsule &b
) {
    //! @todo
    return CollisionType::None;
}

Physics::CollisionType Physics::checkCollision(
    const Position &pa,
    const Sphere &a,
    const Position &pb,
    const Box &b
) {
    return checkCollision(pb, b, pa, a);
}

Physics::CollisionType Physics::checkCollision(
    const Position &pa,
    const Sphere &a,
    const Position &pb,
    const Sphere &b
) {
    Position c = pb - pa;
    int32_t r = b.getRadius() + a.getRadius();
    int64_t d = c.x * c.x + c.y * c.y + c.z * c.z - r * r;

    if (d >= Epsilon) {
        return CollisionType::None;
    } else if (d > -Epsilon) {
        return CollisionType::Contact;
    } else {
        return CollisionType::Intrusion;
    }
}

Physics::CollisionType Physics::checkCollision(
    const Position &pa,
    const Sphere &a,
    const Position &pb,
    const Capsule &b
) {
    //! @todo
    return CollisionType::None;
}

Physics::CollisionType Physics::checkCollision(
    const Position &pa,
    const Capsule &a,
    const Position &pb,
    const Box &b
) {
    return checkCollision(pb, b, pa, a);
}

Physics::CollisionType Physics::checkCollision(
    const Position &pa,
    const Capsule &a,
    const Position &pb,
    const Sphere &b
) {
    return checkCollision(pb, b, pa, a);
}

Physics::CollisionType Physics::checkCollision(
    const Position &pa,
    const Capsule &a,
    const Position &pb,
    const Capsule &b
) {
    Position c = pb - pa;
    int32_t r = b.getRadius() + a.getRadius();
    //~ int32_t h = b.getHeight() + a.getHeight() - r;
    int64_t d = c.x * c.x + c.z * c.z - r * r;

    if (d >= Epsilon) {
        // out of horizontal range
        return CollisionType::None;
    } else if (d > -Epsilon) {
        //! @todo
        return CollisionType::Contact;
    } else {
        //! @todo
        return CollisionType::Intrusion;
    }
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

void Model::calcNormals(size_t start, size_t end, bool smooth) {
    if (end > mVertices.size()) {
        end = mVertices.size();
    }
    if (start > end) {
        return;
    }

    sf::err() << "start == " << start << ", end == " << end << "\n";

    switch (mPrimitive) {
        case GLTriangles: {
            for (size_t i = 0; i < mVertices.size(); i += 3) {
                sf::Vector3f p[3], n[3];

                for (size_t j = 0; j < 3; j++) {
                    p[j] = mVertices[i+j].position;
                }

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

        case GLTriangleFan: {
            break;
        }

        case GLTriangleStrip: {
            break;
        }

        case GLQuads: {
            for (size_t i = 0; i < mVertices.size(); i += 4) {
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
            break;
        }
    }
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
    /// @todo rewrite with map.find/map.insert
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

