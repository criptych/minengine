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

