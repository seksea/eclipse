#pragma once
#include "iclientmode.hpp"
#include "../../util/memory.hpp"
#include "../cutil.hpp"

class Entity;
class CMoveData;

class IGameMovement {
public:
    VFUNC(void, processMovement, 2, (Entity* player, CMoveData* move), (this, player, move))
    VFUNC(void, startTrackPredictionErrors, 4, (Entity* player), (this, player))
    VFUNC(void, finishTrackPredictionErrors, 5, (Entity* player), (this, player))
};

class IMoveHelper {
public:
    VFUNC(void, setHost, 1, (Entity* player), (this, player))
};

class IPrediction {
public:
	std::byte		pad0[0x4];						// 0x0000
	std::uintptr_t	hLastGround;					// 0x0004
	bool			bInPrediction;					// 0x0008
	bool			bIsFirstTimePredicted;			// 0x0009
	bool			bEnginePaused;					// 0x000A
	bool			bOldCLPredictValue;				// 0x000B
	int				iPreviousStartFrame;			// 0x000C
	int				nIncomingPacketNumber;			// 0x0010
	float			flLastServerWorldTimeStamp;		// 0x0014

	struct Split_t
	{
		bool		bIsFirstTimePredicted;			// 0x0018
		std::byte	pad0[0x3];						// 0x0019
		int			nCommandsPredicted;				// 0x001C
		int			nServerCommandsAcknowledged;	// 0x0020
		int			iPreviousAckHadErrors;			// 0x0024
		float		flIdealPitch;					// 0x0028
		int			iLastCommandAcknowledged;		// 0x002C
		bool		bPreviousAckErrorTriggersFullLatchReset; // 0x0030
		CUtlVector<std::uintptr_t> vecEntitiesWithPredictionErrorsInLastAck; // 0x0031
		bool		bPerformedTickShift;			// 0x0045
	};

	Split_t			Split[1];						// 0x0018
	
public:
    VFUNC(void, setupMove, 21, (Entity* player, CUserCmd* cmd, IMoveHelper* helper, CMoveData* move), (this, player, cmd, helper, move))
    VFUNC(void, finishMove, 22, (Entity* player, CUserCmd* cmd, CMoveData* move), (this, player, cmd, move))
};

namespace Prediction {
    inline bool inPrediction = false;
    inline float oldCurtime;
    inline float oldFrametime;

    void startPrediction(CUserCmd* cmd);

    void endPrediction();
}