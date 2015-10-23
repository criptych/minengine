////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __WORLD_HPP__
#define __WORLD_HPP__ 1

////////////////////////////////////////////////////////////////////////////////

#include <deque>
#include <map>
#include <string>
#include <vector>

#include "types.hpp"

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

#endif // __WORLD_HPP__

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

