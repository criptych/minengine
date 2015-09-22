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

#include <SFML/Graphics/Color.hpp>
#include <SFML/System.hpp>

////////////////////////////////////////////////////////////////////////////////

/**
 *  A note on coordinates and angles:
 *   o  Absolute coordinates are given in blocks, as 64-bit fixed-point values
 *      with 8 fractional bits.
 *   o  Chunk coordinates are given as above, in whole blocks with no fractional
 *      bits.
 *   o  Angles use a 256-point scale analogous to degrees, with -128 = 180deg,
 *      reducing modular angle calculations to a mask operation and most
 *      trigonometry to a simple lookup.
 */

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
    //~ uint8_t r, g, b, a; // color
    //~ uint16_t s, t;      // texcoord
    //~ float u, v, w;      // normal
    //~ float x, y, z;      // vertex
    sf::Color color;
    sf::Vector2<sf::Int16> texCoord;
    sf::Vector3f normal;
    sf::Vector3f position;

    Vertex(
    //~ ): r(255), g(255), b(255), a(255), s(), t(), u(), v(), w(), x(), y(), z() {
    ): color(255, 255, 255, 255) {
    }

    Vertex(
        float x, float y, float z
    //~ ): r(255), g(255), b(255), a(255), s(), t(), u(), v(), w(), x(x), y(y), z(z) {
    ): color(255, 255, 255, 255), position(x, y, z) {
    }

    Vertex(
        float u, float v, float w,
        float x, float y, float z
    //~ ): r(255), g(255), b(255), a(255), s(), t(), u(u), v(v), w(w), x(x), y(y), z(z) {
    ): color(255, 255, 255, 255), normal(u, v, w), position(x, y, z) {
    }

    Vertex(
        uint16_t s, uint16_t t,
        float u, float v, float w,
        float x, float y, float z
    //~ ): r(255), g(255), b(255), a(255), s(s), t(t), u(u), v(v), w(w), x(x), y(y), z(z) {
    ): color(255, 255, 255, 255), texCoord(s, t), normal(u, v, w), position(x, y, z) {
    }

    Vertex(
        uint8_t r, uint8_t g, uint8_t b, uint8_t a,
        uint16_t s, uint16_t t,
        float u, float v, float w,
        float x, float y, float z
    //~ ): r(r), g(g), b(b), a(a), s(s), t(t), u(u), v(v), w(w), x(x), y(y), z(z) {
    ): color(r, g, b, a), texCoord(s, t), normal(u, v, w), position(x, y, z) {
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
 *  Packet type codes.
 *
 *  Packet types 0-127 (0x00-0x7f) are reserved for internal use.
 *  Packet types 128-255 (0x80-0xff) are available for custom implementations,
 *  and are ignored by the default/internal handler.
 *
 *  Many packets follow the scheme used by the Minecraft protocol, mainly to
 *  support the same range of features.  In addition, new packets are available
 *  to support custom game definitions and more flexible modding.
 *
 *  Parameters
 *  -----
 *
 *  All multi-byte values are in network/big-endian order.
 *
 *  Types:
 *      "int8" - signed 8-bit integer
 *      "uint8" - unsigned 8-bit integer
 *      "int16" - signed 16-bit integer
 *      "uint16" - unsigned 16-bit integer
 *      "int32" - signed 32-bit integer
 *      "uint32" - unsigned 32-bit integer
 *      "int64" - signed 64-bit integer
 *      "uint64" - unsigned 64-bit integer
 *      "type[n]" - array of 'type' with length 'n'
 *      "type[]" - array of 'type' with varying length
 *      "blob8" - uint8 (length) followed by uint8[length]
 *      "blob16" - uint16 (length) followed by uint8[length]
 *      "blob32" - uint32 (length) followed by uint8[length]
 *      "string" - NUL-terminated UTF-8 string with uint16 prefix
 *                 (equivalent to blob16; length includes NUL)
 */

enum class PacketType : uint8_t {

    /**
     *  Initiate login to server. (Client -> Server)
     *
     *  Parameters:
     *      uint8[16] (UUID)
     *      string (player name)
     */
    ServerLoginRequest,

    /**
     *
     *  Parameters:
     *      uint8 (login result), string (message)
     */
    ServerLoginResponse,

    /**
     *
     *  Parameters:
     *      blob (auth token)
     */
    ServerAuthRequest,
    ServerAuthResponse,

    /**
     *
     *  Parameters:
     *      (no payload)
     */
    ServerLogout,

    /**
     *  Request server information. (Client -> Server)
     *
     *  Sends a request for information about the server.  The server should
     *  reply with a ServerInformationResponse command.
     *
     *  Parameters:
     *      none
     */
    ServerInformationRequest,

    /**
     *  Send server information. (Server -> Client)
     *
     *  Returns information about the server to the client.
     *
     *  Parameters:
     *      uint32 (num players)
     *      uint32 (max players)
     *      uint32 (reserved flags)
     *      string (info message)
     */
    ServerInformationResponse,

    /**
     *  Send player chat message. (Client -> Server)
     *
     *  The server may handle the message in any way.  Specifically, many
     *  servers will likely interpret messages starting with '/' as commands.
     *
     *  Parameters:
     *      string (message)
     */
    PlayerChat,

    /**
     *  Send server chat message. (Server -> Client)
     *
     *  Like PlayerChat, plus indicates the originator of the message.
     *
     *  Parameters:
     *      string (sender), string (message)
     */
    ServerChat,

    /**
     *
     *  Parameters:
     *      int64 (x), int64 (y), int64 (z), blob16 (size, data)
     *      if size != 16384, data is deflate-compressed
     *      if size == 0, chunk is empty
     */
    ChunkSingle,

    /**
     *
     *  Parameters:
     *      int64 (x), int64 (y), int64 (z), uint8 num, blob16 (size, data)
     *      if size != 16384 * num, data is deflate-compressed
     *      if size == 0, chunks are empty
     */
    ChunkColumn,

    /**
     *  Check Resource (Client -> Server)
     *
     *      Sends the timestamp of the client's version of the given resource.
     *      If the client does not have the resource, set timestamp to zero.
     *      If server decides the resource is outdated, it should send a
     *      LoadResource command with the new version.
     *
     *
     *  Parameters:
     *      uint32 (id), uint64 (timestamp)
     */
    CheckResource,

    /**
     *  Server -> Client
     *      Sends resource data to the client.  The client should cache this
     *      data for future connections to the server.
     *
     *  Parameters:
     *      uint32 (id), uint64 (timestamp), uint16 (type), blob32 (size, data)
     *      resource format described elsewhere; depends on type
     */
    LoadResource,

    /**
     *
     *  Parameters:
     *      (no payload)
     */
    PlayerSpawn,

    /**
     *
     *  Parameters:
     *      uint32 (image id)
     *      skin image layout is described elsewhere
     */
    PlayerSkin,

    /**
     *
     *  Parameters:
     *      int64 (x), int64 (y), int64 (z), int8 (pitch), int8 (yaw)
     */
    PlayerMoveTo,

    /**
     *
     *  Parameters:
     *      int16 (dx), int16 (dy), int16 (dz)
     */
    PlayerMove,

    /**
     *
     *  Parameters:
     *      int8 (pitch), int8 (yaw)
     */
    PlayerLook,

    /**
     *
     *  Parameters:
     *      uint64 (eid), uint32 (image id)
     *      skin image layout is described elsewhere
     */
    EntitySkin,

    /**
     *
     *  Parameters:
     *      uint64 (id), uint32 (type), uint32 (data)
     */
    EntitySpawn,

    /**
     *
     *  Parameters:
     *      uint64 (id), int64 (x), int64 (y), int64 (z), int8 (pitch), int8 (yaw)
     */
    EntityMoveTo,

    /**
     *
     *  Parameters:
     *      uint64 (id), int16 (dx), int16 (dy), int16 (dz)
     */
    EntityMove,

    /**
     *
     *  Parameters:
     *      uint64 (id), int8 (pitch), int8 (yaw)
     */
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

