////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __NETWORK_HPP__
#define __NETWORK_HPP__ 1

////////////////////////////////////////////////////////////////////////////////

#include "types.hpp"

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

#endif // __NETWORK_HPP__

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

