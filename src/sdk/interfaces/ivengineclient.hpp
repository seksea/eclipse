#pragma once
#include <cstdint>
#include "../../util/memory.hpp"
#include "../math.hpp"

#define FULLUPDATE() Offsets::getLocalClient(-1)->m_nDeltaTick = -1

class BaseClientState {
public:
	char pad_0000[296]; //0x0000
	class INetChannel* m_NetChannel; //0x0128
	char pad_0130[124]; //0x0130
	float m_flNextCmdTime; //0x01AC
	int32_t m_nServerCount; //0x01B0
	int32_t m_nCurrentSequence; //0x01B4
	char pad_01B8[8]; //0x01B8
	float clockOffset1; //0x01C0
	float clockOffset2; //0x01C4
	float clockOffset3; //0x01C8
	float clockOffset4; //0x01CC
	float clockOffset5; //0x01D0
	float clockOffset6; //0x01D4
	float clockOffset7; //0x01D8
	float clockOffset8; //0x01DC
	float clockOffset9; //0x01E0
	float clockOffset10; //0x01E4
	float clockOffset11; //0x01E8
	float clockOffset12; //0x01EC
	float clockOffset13; //0x01F0
	float clockOffset14; //0x01F4
	float clockOffset15; //0x01F8
	float clockOffset16; //0x01FC
	int32_t m_iCurClockOffset; //0x0200
	int32_t m_nServerTick; //0x0204
	int32_t m_nClientTick; //0x0208
	int32_t m_nDeltaTick; //0x020C
	bool m_bPaused; //0x0210
	char pad_0211[15]; //0x0211
	char m_szLevelName[260]; //0x0220
	char m_szLevelNameShort[80]; //0x0324
	char m_szGroupName[80]; //0x0374
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
    VFUNC(void, getViewAngles, 18, (Vector& angles), (this, &angles))
    VFUNC(int, getMaxClients, 20, (), (this))
    VFUNC(bool, isInGame, 26, (), (this))
    VFUNC(bool, isConnected, 27, (), (this))
    VFUNC(const VMatrix&, worldToScreenMatrix, 37, (), (this))
    VFUNC(void*, getBSPTreeQuery, 43, (), (this))
    VFUNC(const char*, getLevelName, 53, (), (this))
    //VFUNC(NetworkChannel*, getNetworkChannel, 78, (), (this))
    VFUNC(void, clientCmdUnrestricted, 113, (const char* cmd, bool fromConsoleOrKeybind = false), (this, cmd, fromConsoleOrKeybind))
    //VFUNC_V(const SteamAPIContext*, getSteamAPIContext, 185, (), (this))
};