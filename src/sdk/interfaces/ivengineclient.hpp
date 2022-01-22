#pragma once
#include <cstdint>
#include "../../util/memory.hpp"
#include "../math.hpp"

class NetChannel {
public:
	enum
	{
		GENERIC = 0,	// must be first and is default group
		LOCALPLAYER,	// bytes for local player entity update
		OTHERPLAYERS,	// bytes for other players update
		ENTITIES,		// all other entity bytes
		SOUNDS,			// game sounds
		EVENTS,			// event messages
		TEMPENTS,		// temp entities
		USERMESSAGES,	// user messages
		ENTMESSAGES,	// entity messages
		VOICE,			// voice data
		STRINGTABLE,	// a stringtable update
		MOVE,			// client move cmds
		STRINGCMD,		// string command
		SIGNON,			// various signondata
		PAINTMAP,		// paintmap data
		ENCRYPTED,		// encrypted data
		TOTAL,			// must be last and is not a real group
	};

	virtual const char* getName() const = 0;			// get channel name
	virtual const char* getAddress() const = 0;			// get channel IP address as string
	virtual float		getTime() const = 0;			// current net time
	virtual float		getTimeConnected() const = 0;	// get connection time in seconds
	virtual int			getBufferSize() const = 0;		// netchannel packet history size
	virtual int			getDataRate() const = 0;		// send data rate in byte/sec

	virtual bool		isLoopback() const = 0;			// true if loopback channel
	virtual bool		isTimingOut() const = 0;		// true if timing out
	virtual bool		isPlayback() const = 0;			// true if demo playback
	virtual float		getLatency(int iFlow) const = 0; // current latency (RTT), more accurate but jittering
	virtual float		getAvgLatency(int iFlow) const = 0; // average packet latency in seconds
	virtual float		getAvgLoss(int iFlow) const = 0; // avg packet loss[0..1]
	virtual float		getAvgChoke(int iFlow) const = 0; // avg packet choke[0..1]
	virtual float		getAvgData(int iFlow) const = 0; // data flow in bytes/sec
	virtual float		getAvgPackets(int iFlow) const = 0; // avg packets/sec
	virtual int			getTotalData(int iFlow) const = 0; // total flow in/out in bytes
	virtual int			getTotalPackets(int iFlow) const = 0;
	virtual int			getSequenceNr(int iFlow) const = 0; // last send seq number
	virtual bool		isValidPacket(int iFlow, int nFrame) const = 0; // true if packet was not lost/dropped/chocked/flushed
	virtual float		getPacketTime(int iFlow, int nFrame) const = 0; // time when packet was send
	virtual int			getPacketBytes(int iFlow, int nFrame, int group) const = 0; // group size of this packet
	virtual bool		getStreamProgress(int iFlow, int* piReceived, int* piTotal) const = 0; // TCP progress if transmitting
	virtual float		getTimeSinceLastReceived() const = 0; // get time since last received packet in seconds
	virtual	float		getCommandInterpolationAmount(int iFlow, int nFrame) const = 0;
	virtual void		getPacketResponseLatency(int iFlow, int frame_number, int* pnLatencyMsecs, int* pnChoke) const = 0;
	virtual void		getRemoteFramerate(float* pflFrameTime, float* pflFrameTimeStdDeviation, float* pflFrameStartTimeStdDeviation) const = 0;
	virtual float		getTimeoutSeconds() const = 0;
};

// Engine player info, no game related infos here
struct PlayerInfo
{
    int64_t __pad0;
    union {
        int64_t xuid;
        struct {
            int xuidlow;
            int xuidhigh;
        };
    };
    char name[128];
    int userid;
    char guid[33];
    unsigned int friendsid;
    char friendsname[128];
    bool fakeplayer;
    bool ishltv;
    unsigned int customfiles[4];
    unsigned char filesdownloaded;
};


class IVEngineClient {
public:
    VFUNC(void, getScreenSize, 5, (int& w, int& h), (this, &w, &h))
    VFUNC(bool, getPlayerInfo, 8, (int entityIndex, PlayerInfo& playerInfo), (this, entityIndex, &playerInfo))
    VFUNC(int, getPlayerForUserID, 9, (int userId), (this, userId))
    VFUNC(int, getLocalPlayer, 12, (), (this))
    VFUNC(void, getViewAngles, 18, (QAngle& angles), (this, &angles))
    VFUNC(void, setViewAngles, 19, (QAngle& angles), (this, &angles))
    VFUNC(int, getMaxClients, 20, (), (this))
    VFUNC(bool, isInGame, 26, (), (this))
    VFUNC(bool, isConnected, 27, (), (this))
    VFUNC(const VMatrix&, worldToScreenMatrix, 37, (), (this))
    VFUNC(void*, getBSPTreeQuery, 43, (), (this))
    VFUNC(const char*, getLevelName, 53, (), (this))
    VFUNC(NetChannel*, getNetworkChannel, 78, (), (this))
    VFUNC(void, clientCmdUnrestricted, 113, (const char* cmd, bool fromConsoleOrKeybind = false), (this, cmd, fromConsoleOrKeybind))
    //VFUNC_V(const SteamAPIContext*, getSteamAPIContext, 185, (), (this))
};