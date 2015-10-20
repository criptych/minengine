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
 *   o  Chunk coordinates are signed 64-bit integer values, in meters/16.
 *   o  Block coordinates are signed 64-bit integer values, in meters.
 *   o  Entity coordinates are signed 64-bit fixed-point values with
 *      8 fractional bits, in meters.
 *   o  Velocities are signed 16-bit fixed-point values with 8 fractional bits,
 *      in meters/tick.
 *   o  Sizes are unsigned 16-bit fixed-point values with 8 fractional bits,
 *      in meters.
 *   o  Angles are signed 8-bit integer values, in a 256-point scale analogous
 *      to degrees, with -128 = 180deg.
 *
 *  These formats were chosen to allow high-performance integer calculations
 *  for most operations and compact representation for transmission between
 *  client and server, with enough precision for reasonably smooth physics.
 *  The format of angles in particular allows for efficient wraparound handling
 *  and implementing most trigonometry operations as simple table lookups.
 */

////////////////////////////////////////////////////////////////////////////////

typedef uint64_t EntityID;

const long double Pi    = 3.141592653589793238462643383279;
const long double TwoPi = 6.283185307179586476925286766559;

class Angle {
    int8_t mValue;

    explicit Angle(int8_t value): mValue(value) {}

public:
    Angle(): mValue() {}

    float asDegrees() const;
    float asRadians() const;
    int8_t asByte() const;

    float sin() const;
    float cos() const;
    float tan() const;

    void sincos(float &s, float &c) const;

    static const Angle Zero;
    static const Angle Right;

    static Angle fromDegrees(float angle);
    static Angle fromRadians(float angle);
    static Angle fromByte(int8_t angle);
};

bool operator == (const Angle &a, const Angle &b);
bool operator != (const Angle &a, const Angle &b);
bool operator <  (const Angle &a, const Angle &b);
bool operator >  (const Angle &a, const Angle &b);
bool operator <= (const Angle &a, const Angle &b);
bool operator >= (const Angle &a, const Angle &b);

Angle operator + (const Angle &a);
Angle operator - (const Angle &a);
Angle operator + (const Angle &a, const Angle &b);
Angle operator - (const Angle &a, const Angle &b);

Angle &operator += (Angle &a, const Angle &b);
Angle &operator -= (Angle &a, const Angle &b);

////////////////////////////////////////////////////////////////////////////////

typedef int64_t Coord;
typedef uint16_t Size;
typedef int16_t Delta;
typedef int32_t LargeDelta;
typedef int64_t HugeDelta;

typedef sf::Vector2<Angle> Orientation;
typedef sf::Vector3<Coord> Position;
typedef sf::Vector3<Size>  Dimension;
typedef sf::Vector3<Delta> Velocity;
typedef sf::Vector3<LargeDelta> Acceleration;
typedef sf::Vector3<LargeDelta> Force;

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

struct Vertex {
    sf::Color color;
    sf::Vector2<sf::Int16> texCoord;
    sf::Vector3f normal;
    sf::Vector3f position;

    Vertex(
    ): color(sf::Color::White) {
    }

    explicit Vertex(
        const sf::Vector3f &position
    ): color(sf::Color::White), position(position) {
    }

    Vertex(
        const sf::Vector3f &normal,
        const sf::Vector3f &position
    ): color(sf::Color::White), normal(normal), position(position) {
    }

    Vertex(
        const sf::Vector2<sf::Int16> &texCoord,
        const sf::Vector3f &position
    ): color(sf::Color::White), texCoord(texCoord), position(position) {
    }

    Vertex(
        const sf::Vector2<sf::Int16> &texCoord,
        const sf::Vector3f &normal,
        const sf::Vector3f &position
    ): color(sf::Color::White), texCoord(texCoord), normal(normal), position(position) {
    }

    Vertex(
        const sf::Color &color,
        const sf::Vector3f &position
    ): color(color), position(position) {
    }

    Vertex(
        const sf::Color &color,
        const sf::Vector3f &normal,
        const sf::Vector3f &position
    ): color(color), normal(normal), position(position) {
    }

    Vertex(
        const sf::Color &color,
        const sf::Vector2<sf::Int16> &texCoord,
        const sf::Vector3f &position
    ): color(color), texCoord(texCoord), position(position) {
    }

    Vertex(
        const sf::Color &color,
        const sf::Vector2<sf::Int16> &texCoord,
        const sf::Vector3f &normal,
        const sf::Vector3f &position
    ): color(color), texCoord(texCoord), normal(normal), position(position) {
    }

    Vertex(
        float x, float y, float z
    ): color(sf::Color::White), position(x, y, z) {
    }

    Vertex(
        float u, float v, float w,
        float x, float y, float z
    ): color(sf::Color::White), normal(u, v, w), position(x, y, z) {
    }

    Vertex(
        uint16_t s, uint16_t t,
        float x, float y, float z
    ): color(sf::Color::White), texCoord(s, t), position(x, y, z) {
    }

    Vertex(
        uint16_t s, uint16_t t,
        float u, float v, float w,
        float x, float y, float z
    ): color(sf::Color::White), texCoord(s, t), normal(u, v, w), position(x, y, z) {
    }

    Vertex(
        uint8_t r, uint8_t g, uint8_t b, uint8_t a,
        float x, float y, float z
    ): color(r, g, b, a), position(x, y, z) {
    }

    Vertex(
        uint8_t r, uint8_t g, uint8_t b, uint8_t a,
        float u, float v, float w,
        float x, float y, float z
    ): color(r, g, b, a), normal(u, v, w), position(x, y, z) {
    }

    Vertex(
        uint8_t r, uint8_t g, uint8_t b, uint8_t a,
        uint16_t s, uint16_t t,
        float x, float y, float z
    ): color(r, g, b, a), texCoord(s, t), position(x, y, z) {
    }

    Vertex(
        uint8_t r, uint8_t g, uint8_t b, uint8_t a,
        uint16_t s, uint16_t t,
        float u, float v, float w,
        float x, float y, float z
    ): color(r, g, b, a), texCoord(s, t), normal(u, v, w), position(x, y, z) {
    }

};

////////////////////////////////////////////////////////////////////////////////

class Model {
    uint32_t mPrimitive;
    std::vector<Vertex> mVertices;
    std::vector<uint16_t> mIndices;

public:
    Model();
    explicit Model(uint32_t primitive);

    template <typename I>
    Model(
        uint32_t primitive,
        const I &start,
        const I &end
    ): mPrimitive(primitive) {
        addVertices(start, end);
    }

    template <typename A>
    Model(
        uint32_t primitive,
        const A &array
    ): mPrimitive(primitive) {
        addVertices(array);
    }

    uint32_t getPrimitive() const;

    void setPrimitive(uint32_t primitive);

    const std::vector<Vertex> &getVertices() const;
    std::vector<Vertex> &getVertices();

    void clearVertices();
    void reserveVertices(size_t count);
    void addVertex(const Vertex &vertex);
    void addTriangle(const Vertex &a, const Vertex &b, const Vertex &c);
    void addQuad(const Vertex &a, const Vertex &b, const Vertex &c, const Vertex &d);

    void addVertices(const Vertex *verts, size_t count);

    template <typename I>
    void addVertices(
        const I &start,
        const I &end
    ) {
        mVertices.insert(mVertices.end(), start, end);
    }

    template <typename A>
    void addVertices(
        const A &array
    ) {
        for (const Vertex &v : array) {
            addVertex(v);
        }
    }

    const std::vector<uint16_t> &getIndices() const;
    std::vector<uint16_t> &getIndices();

    void clearIndices();
    void reserveIndices(size_t count);
    void addIndex(uint16_t index);
    void addTriangle(uint16_t a, uint16_t b, uint16_t c);
    void addQuad(uint16_t a, uint16_t b, uint16_t c, uint16_t d);

    void addIndices(const uint16_t *indices, size_t count);

    template <typename I>
    void addIndices(
        const I &start,
        const I &end
    ) {
        mIndices.insert(mIndices.end(), start, end);
    }

    template <typename A>
    void addIndices(
        const A &array
    ) {
        for (uint16_t v : array) {
            addIndex(v);
        }
    }

    void setColor(const sf::Color &color);

    void calcNormals(bool smooth = false);
    void calcNormals(size_t start, size_t end, bool smooth = false);

    void makeBox(const sf::Vector3f &size, const sf::Vector3f &center);
    void makeBox(const sf::Vector3f &size);
    void makeBox();

    void makeBall(float radius, size_t step, size_t rstep, const sf::Vector3f &center);
    void makeBall(float radius, size_t step, size_t rstep);
    void makeBall(float radius, size_t step, const sf::Vector3f &center);
    void makeBall(float radius, size_t step);
};

////////////////////////////////////////////////////////////////////////////////

class Entity {
    EntityID mID;
    Position mPosition;
    Velocity mVelocity;
    Orientation mOrientation;

public:
    EntityID getID() const;
    const Position &getPosition() const;
    const Velocity &getVelocity() const;
    const Orientation &getOrientation() const;

    void update();
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

public:
    enum class Attributes : uint8_t {
        Default     = 0,
        Transparent = 1,
    };

private:
    static std::vector<Attributes> mAttrs;
    static std::map<std::string, BlockType> mNames;
    static std::map<BlockType, std::string> mNamesInv;

public:
    static std::string getName(BlockType type);
    static BlockType getType(const std::string &name);
    static Attributes getAttributes(BlockType type);
    static Attributes getAttributes(const std::string &name);

    static void enlist(BlockType type, const std::string &name, Attributes attr = Attributes::Default);
    static void delist(BlockType type);
};

class ChunkData {
    static const unsigned int Count = 16;

    BlockType mBlockType[Count * Count * Count];
    BlockData mBlockData[Count * Count * Count];
    LightData mLightData[Count * Count * Count];

public:
    ChunkData();

    //~ ~ChunkData() {}

    Block getBlock(const Position &pos) {
        uint16_t i = (pos.z % Count) * Count * Count +
                     (pos.y % Count) * Count +
                     (pos.x % Count);
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
    explicit ChunkCache(ChunkSource *source, size_t capacity = 4096);
    explicit ChunkCache(ChunkSource &source, size_t capacity = 4096);

    Chunk *getChunk(const Position &pos);
};

////////////////////////////////////////////////////////////////////////////////

class World {
    ChunkSource *mUpstream;

    uint32_t mTicksPerSecond;
    uint64_t mTicksPerDay;

public:
    explicit World(ChunkSource *upstream): mUpstream(upstream) {}
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
/*
template <typename T>
sf::Vector3<T> cross(const sf::Vector3<T> &a, const sf::Vector3<T> &b) {
    return sf::Vector3<T>(a.y * b.z - a.z * b.y,
                          a.z * b.x - a.x * b.z,
                          a.x * b.y - a.y * b.x);
}

template <typename T>
T dot(const sf::Vector3<T> &a, const sf::Vector3<T> &b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

template <typename T>
sf::Vector3<T> normalize(const sf::Vector3<T> &v) {
    T length = std::sqrt(dot(v, v));

    if (length == 0) {
        return v;
    } else {
        return v / length;
    }
}
*/

inline float quadratic(float t) {
    if (t >= 0.5f) {
        t = 1.0f - t;
        return 1.0f - (t * t);
    } else {
        return t * t;
    }
}

inline float cubic(float t) {
    if (t >= 0.5f) {
        t = 1.0f - t;
        return 1.0f - (t * t * t);
    } else {
        return t * t * t;
    }
}

inline float quartic(float t) {
    if (t >= 0.5f) {
        t = 1.0f - t;
        return 1.0f - (t * t * t * t);
    } else {
        return t * t * t * t;
    }
}

inline float quintic(float t) {
    if (t >= 0.5f) {
        t = 1.0f - t;
        return 1.0f - (t * t * t * t * t);
    } else {
        return t * t * t * t * t;
    }
}

template <typename T>
T lerp(float t, T start, T end) {
    return (1.0f - t) * start + t * end;
}

template <typename T>
T easeQuadratic(float t, T start, T end) {
    return lerp(quadratic(t), start, end);
}

template <typename T>
T easeCubic(float t, T start, T end) {
    return lerp(cubic(t), start, end);
}

template <typename T>
T easeQuartic(float t, T start, T end) {
    return lerp(quartic(t), start, end);
}

template <typename T>
T easeQuintic(float t, T start, T end) {
    return lerp(quintic(t), start, end);
}

////////////////////////////////////////////////////////////////////////////////

#endif // __ENGINE_HPP__

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

