////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __ENGINE_HPP__
#define __ENGINE_HPP__ 1

////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <deque>
#include <limits>
#include <map>
#include <vector>

#include <cinttypes>
#include <cmath>

#include <SFML/System.hpp>

////////////////////////////////////////////////////////////////////////////////

typedef uint64_t EntityID;

typedef int64_t Coord;
typedef int16_t Delta;
typedef int8_t Angle;

typedef sf::Vector3<Coord> Position;
typedef sf::Vector3<Delta> Velocity;
typedef sf::Vector2<Angle> Orientation;

////////////////////////////////////////////////////////////////////////////////

struct Vertex {
    uint8_t r, g, b, a; // color
    uint16_t s, t;      // texcoord
    float u, v, w;      // normal
    float x, y, z;      // vertex

    Vertex(
        float x, float y, float z
    ): x(x), y(y), z(z) {
    }

    Vertex(
        float u, float v, float w,
        float x, float y, float z
    ): u(u), v(v), w(w), x(x), y(y), z(z) {
    }

    Vertex(
        uint16_t s, uint16_t t,
        float u, float v, float w,
        float x, float y, float z
    ): s(s), t(t), u(u), v(v), w(w), x(x), y(y), z(z) {
    }

    Vertex(
        uint8_t r, uint8_t g, uint8_t b, uint8_t a,
        uint16_t s, uint16_t t,
        float u, float v, float w,
        float x, float y, float z
    ): r(r), g(g), b(b), a(a), s(s), t(t), u(u), v(v), w(w), x(x), y(y), z(z) {
    }

};

class Model {
    uint32_t mPrimitive;
    std::vector<Vertex> mVertices;

public:
    Model(
        uint32_t primitive
    ): mPrimitive(primitive) {
    }

    template <typename I>
    Model(
        uint32_t primitive,
        const I &start,
        const I &end
    ): mPrimitive(primitive), mVertices(start, end) {
    }

    template <typename A>
    Model(
        uint32_t primitive,
        const A &array
    ): mPrimitive(primitive), mVertices(std::begin(array), std::end(array)) {
    }

    uint32_t getPrimitive() const {
        return mPrimitive;
    }

    const std::vector<Vertex> &getVertices() const {
        return mVertices;
    }

    void setPrimitive(uint32_t primitive) {
        mPrimitive = primitive;
    }

    void clearVertices() {
        mVertices.clear();
    }

    void addVertex(const Vertex &vertex) {
        mVertices.push_back(vertex);
    }

    void calcNormals(bool smooth = false);
};

////////////////////////////////////////////////////////////////////////////////

class Entity {
    EntityID mID;
    Position mPosition;
    Velocity mVelocity;
    Orientation mOrientation;

public:
    void update() {
        mPosition.x += mVelocity.x;
        mPosition.y += mVelocity.y;
        mPosition.z += mVelocity.z;
    }
};

////////////////////////////////////////////////////////////////////////////////

typedef uint16_t BlockType;
typedef uint8_t  BlockData;
typedef uint8_t  LightData;

class Block {
    BlockType *mType;
    BlockData *mData;
    LightData *mLight;

    Block(
        BlockType *type,
        BlockData *data,
        LightData *light
    ): mType(type), mData(data), mLight(light) {
    }

    friend class ChunkData;

public:
    BlockType getType() const { return *mType; }
    void setType(BlockType type) { *mType = type; }

    BlockData getData() const { return *mData; }
    void setData(BlockData data) { *mData = data; }

    LightData getLight() const { return *mLight; }
    void setLight(LightData light) { *mLight = light; }
};

class ChunkData {
    static const unsigned int Size = 16;

    BlockType mBlockType[Size * Size * Size];
    BlockData mBlockData[Size * Size * Size];
    LightData mLightData[Size * Size * Size];

public:
    ChunkData();

    //~ ~ChunkData() {}

    Block getBlock(const Position &pos) {
        uint16_t i = (pos.z % Size) * Size * Size + (pos.y % Size) * Size + (pos.x % Size);
        return Block(&mBlockType[i], &mBlockData[i], &mLightData[i]);
    }

    Block operator[](const Position &pos) {
        return getBlock(pos);
    }
};

/**
 * Binds chunk location to its data and active entities.
 */
class Chunk {
    Position mPosition;
    ChunkData *mData;

public:
    Chunk(): mPosition(), mData() {}
    Chunk(const Position &pos, ChunkData *data): mPosition(pos), mData(data) {}

    //~ ~Chunk() {}

    const Position &getPosition() const {
        return mPosition;
    }

    ChunkData *getData() {
        return mData;
    }

    const ChunkData *getData() const {
        return mData;
    }

    Block getBlock(const Position &pos) {
        return mData->getBlock(pos);
    }

    const Block getBlock(const Position &pos) const {
        return mData->getBlock(pos);
    }

    Block operator[](const Position &pos) {
        return getBlock(pos);
    }

    const Block operator[](const Position &pos) const {
        return getBlock(pos);
    }
};

/**
 * Provides a source of chunk data.
 */
class ChunkSource {
protected:
    ChunkSource() {}

public:
    virtual ~ChunkSource() {}

    virtual Chunk *loadChunk(Chunk &chunk, const Position &pos) {
        return nullptr;
    }
};

/**
 * Generates randomized chunk data.
 */
class ChunkGenerator : public ChunkSource {

public:
    Chunk *loadChunk(Chunk &chunk, const Position &pos);
};

/**
 * Loads chunk data from a network source.
 */
class ChunkServer : public ChunkSource {

public:
    Chunk *loadChunk(Chunk &chunk, const Position &pos);
};

/**
 * Provides a server interface to a local instance (avoiding network overhead).
 */
class ChunkLocalServer : public ChunkServer {

public:
    Chunk *loadChunk(Chunk &chunk, const Position &pos);
};

/**
 * Provides a server interface to a remote (network) instance.
 */
class ChunkRemoteServer : public ChunkServer {

public:
    Chunk *loadChunk(Chunk &chunk, const Position &pos);
};

/**
 * Loads and saves chunk data in permanent storage.
 */
class ChunkStore : public ChunkSource {
public:
    virtual void saveChunk(const Chunk &chunk) {}
};

/**
 * Keeps chunk data temporarily in memory.
 *
 * For clients, only one ChunkCache is necessary, with a recommended capacity of
 * 4096 (4096 = 16^3 chunks, about 64MB of memory); for servers, one instance
 * per source is recommended, with a capacity of at least 4096 per player.
 * Modern systems with 1GB of RAM or more should have no trouble, and the
 * larger the cache, the better (up to system limitations of course).
 *
 * The capacity MUST be at least as large as the visible radius around the
 * player to prevent "cache thrashing.".
 */
class ChunkCache {
    ChunkSource *mSource;

    std::vector<ChunkData> mChunkData;
    std::deque<Chunk> mChunks;
    std::map<Position, Chunk*> mChunkMap;

public:
    ChunkCache(ChunkSource *source, size_t capacity = 4096);
    ChunkCache(ChunkSource &source, size_t capacity = 4096);

    Chunk *getChunk(const Position &pos);
};

////////////////////////////////////////////////////////////////////////////////

class World {
    ChunkSource *mUpstream;

public:
    World(ChunkSource *upstream): mUpstream(upstream) {}
};

////////////////////////////////////////////////////////////////////////////////

typedef uint16_t PacketSize;

/**
 * Packet type codes.
 *
 * Packet types 0-127 (0x00-0x7f) are reserved for internal use.
 * Packet types 128-255 (0x80-0xff) are available for custom implementations,
 * and are ignored by the default/internal handler.
 */
enum class PacketType : uint8_t {
    /* string (message) */
    PlayerChat,

    /* string (sender), string (message) */
    ServerChat,

    /* (no payload) */
    ServerInformationRequest,

    /* uint32 (num players), uint32 (max players), string (info message) */
    ServerInformationResponse,

    /* string (player name) */
    ServerLoginRequest,

    /* uint8 (login result), string (message) */
    ServerLoginResponse,

    /* blob (auth token) */
    ServerAuthRequest,
    ServerAuthResponse,

    /* (no payload) */
    ServerLogout,

    /* int64 (x), int64 (y), int64 (z), uint16 (size), uint8[] (data)
     *      if size != 16384, data is deflate-compressed
     *      if size == 0, chunk is empty */
    ChunkSingle,

    /* int64 (x), int64 (y), int64 (z), uint8 num, uint16 (size), uint8[] (data)
     *      if size != 16384 * num, data is deflate-compressed
     *      if size == 0, chunks are empty */
    ChunkColumn,

    /* (no payload) */
    PlayerSpawn,

    /* int64 (x), int64 (y), int64 (z), int8 (pitch), int8 (yaw) */
    PlayerLocationRotation,

    /* int16 (dx), int16 (dy), int16 (dz) */
    PlayerMove,

    /* int8 (pitch), int8 (yaw) */
    PlayerLook,

    /* uint64 (id), uint32 (type), uint32 (data) */
    EntitySpawn,

    /* uint64 (id), int64 (x), int64 (y), int64 (z), int8 (pitch), int8 (yaw) */
    EntityLocationRotation,

    /* uint64 (id), int16 (dx), int16 (dy), int16 (dz) */
    EntityMove,

    /* uint64 (id), int8 (pitch), int8 (yaw) */
    EntityLook,

    CustomPacket = 0x80,
};

struct Packet {
    /* Size (in bytes) of the remaining packet data; allows implementations to
     * ignore unknown or invalid packet types */
    PacketSize size;
    /* packet type for dispatch */
    PacketType type;
    uint8_t data[];
};

////////////////////////////////////////////////////////////////////////////////

#endif // __ENGINE_HPP__

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

