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

#ifndef GL_VERSION_1_1
# define GL_POINTS 0x0000
# define GL_LINES 0x0001
# define GL_LINE_LOOP 0x0002
# define GL_LINE_STRIP 0x0003
# define GL_TRIANGLES 0x0004
# define GL_TRIANGLE_STRIP 0x0005
# define GL_TRIANGLE_FAN 0x0006
# define GL_QUADS 0x0007
# define GL_QUAD_STRIP 0x0008
# define GL_POLYGON 0x0009
#endif

void Model::calcNormals(size_t start, size_t end, bool smooth) {
    if (end > mVertices.size()) {
        end = mVertices.size();
    }
    if (start > end) {
        return;
    }

    sf::err() << "start == " << start << ", end == " << end << "\n";

    switch (mPrimitive)
    {
        case GL_TRIANGLES:
        {
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

        case GL_TRIANGLE_FAN:
        {
            break;
        }

        case GL_TRIANGLE_STRIP:
        {
            break;
        }

        case GL_QUADS:
        {
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

        case GL_QUAD_STRIP:
        {
            break;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

Box::Box(): mCenter(), mDimensions() {
}

Box::Box(
    const Position &center, const Dimension &dim
): mCenter(center), mDimensions(dim) {
}

Box::Box(
    const Box &box, const Position &center
): mCenter(center), mDimensions(box.mDimensions) {
}

Box::Box(
    const Box &box, const Dimension &dim
): mCenter(box.mCenter), mDimensions(dim) {
}

const Position &Box::getCenter() const {
    return mCenter;
}

const Dimension &Box::getDimensions() const {
    return mDimensions;
}

////////////////////////////////////////////////////////////////////////////////

Sphere::Sphere(): mCenter(), mRadius() {
}

Sphere::Sphere(
    const Position &center, Size radius
): mCenter(center), mRadius(radius) {
}

Sphere::Sphere(
    const Sphere &sphere, const Position &center
): mCenter(center), mRadius(sphere.mRadius) {
}

Sphere::Sphere(
    const Sphere &sphere, Size radius
): mCenter(sphere.mCenter), mRadius(radius) {
}

const Position &Sphere::getCenter() const {
    return mCenter;
}

Size Sphere::getRadius() const {
    return mRadius;
}

////////////////////////////////////////////////////////////////////////////////

Capsule::Capsule(): mBase(), mRadius(), mHeight() {
}

Capsule::Capsule(
    const Position &base, Size radius, Size height
): mBase(base), mRadius(radius), mHeight(height) {
}

Capsule::Capsule(
    const Capsule &capsule, const Position &base
): mBase(base), mRadius(capsule.mRadius), mHeight(capsule.mHeight) {
}

Capsule::Capsule(
    const Capsule &capsule, Size radius, Size height
): mBase(capsule.mBase), mRadius(radius), mHeight(height) {
}

const Position &Capsule::getBase() const {
    return mBase;
}

Size Capsule::getRadius() const {
    return mRadius;
}

Size Capsule::getHeight() const {
    return mHeight;
}

////////////////////////////////////////////////////////////////////////////////

Physics::CollisionType Physics::checkCollision(
    const Box &a, const Box &b
) {
    //! @todo
    return CollisionType::None;
}

Physics::CollisionType Physics::checkCollision(
    const Box &a, const Box &b
) {
    //! @todo
    return CollisionType::None;
}

Physics::CollisionType Physics::checkCollision(
    const Box &a, const Box &b
) {
    //! @todo
    return CollisionType::None;
}

Physics::CollisionType Physics::checkCollision(
    const Box &a, const Box &b
) {
    //! @todo
    return CollisionType::None;
}

Physics::CollisionType Physics::checkCollision(
    const Sphere &a, const Sphere &b
) {
    Position c = b.getCenter() - a.getCenter();
    int32_t r = b.getRadius() + a.getRadius();
    int64_t d = c.x * c.x + c.y * c.y + c.z * c.z - r * r;

    if (d <= -Epsilon) {
        return CollisionType::Intrusion;
    } else if (d < Epsilon) {
        return CollisionType::Contact;
    } else {
        return CollisionType::None;
    }
}

Physics::CollisionType Physics::checkCollision(
    const Box &a, const Box &b
) {
    //! @todo
    return CollisionType::None;
}

Physics::CollisionType Physics::checkCollision(
    const Box &a, const Box &b
) {
    //! @todo
    return CollisionType::None;
}

Physics::CollisionType Physics::checkCollision(
    const Box &a, const Box &b
) {
    //! @todo
    return CollisionType::None;
}

Physics::CollisionType Physics::checkCollision(
    const Box &a, const Box &b
) {
    //! @todo
    return CollisionType::None;
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

