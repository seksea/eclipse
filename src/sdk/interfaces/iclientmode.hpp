#pragma once
#include "../math.hpp"

#define TIME_TO_TICKS( t )		( static_cast<int>( 0.5f + static_cast<float>( t ) / Interfaces::globals->interval_per_tick ) )
#define TICKS_TO_TIME( t )		( Interfaces::globals->interval_per_tick * static_cast<float>( t ) )
#define ROUND_TO_TICKS( t )		( Interfaces::globals->interval_per_tick * TIME_TO_TICKS( t ) )

#define FL_ONGROUND				(1<<0)	// At rest / on the ground
#define FL_DUCKING				(1<<1)	// Player flag -- Player is fully crouched
#define FL_WATERJUMP			(1<<2)	// player jumping out of water
#define FL_ONTRAIN				(1<<3)	// Player is _controlling_ a train, so movement commands should be ignored on client during prediction.
#define FL_INRAIN				(1<<4)	// Indicates the entity is standing in rain
#define FL_FROZEN				(1<<5)	// Player is frozen for 3rd person camera
#define FL_ATCONTROLS			(1<<6)	// Player can't move, but keeps key inputs for controlling another entity
#define FL_CLIENT				(1<<7)	// Is a player
#define FL_FAKECLIENT			(1<<8)	// Fake client, simulated server side; don't send network messages to them
#define FL_PARTIALGROUND		(1<<18) // Partially on the ground
// NON-PLAYER SPECIFIC (i.e., not used by GameMovement or the client .dll ) -- Can still be applied to players, though
#define FL_INWATER				(1<<9)	// In water

#define IN_ATTACK		(1 << 0)
#define IN_JUMP			(1 << 1)
#define IN_DUCK			(1 << 2)
#define IN_FORWARD		(1 << 3)
#define IN_BACK			(1 << 4)
#define IN_USE			(1 << 5)
#define IN_CANCEL		(1 << 6)
#define IN_LEFT			(1 << 7)
#define IN_RIGHT		(1 << 8)
#define IN_MOVELEFT		(1 << 9)
#define IN_MOVERIGHT	(1 << 10)
#define IN_ATTACK2		(1 << 11)
#define IN_RUN			(1 << 12)
#define IN_RELOAD		(1 << 13)
#define IN_ALT1			(1 << 14)
#define IN_ALT2			(1 << 15)
#define IN_SCORE		(1 << 16)
#define IN_SPEED		(1 << 17)
#define IN_WALK			(1 << 18)
#define IN_ZOOM			(1 << 19)
#define IN_WEAPON1		(1 << 20)
#define IN_WEAPON2		(1 << 21)
#define IN_BULLRUSH		(1 << 22)
#define IN_GRENADE1		(1 << 23)
#define IN_GRENADE2		(1 << 24)
#define	IN_ATTACK3		(1 << 25)

struct CUserCmd {
	virtual ~CUserCmd() {};
	int commandnumber;
	int tickcount;
	QAngle viewangles;
	QAngle aimdirection;
	float forwardmove;
	float sidemove;
	float upmove;
	int buttons;
	unsigned char impulse;
	int weaponselect;
	int weaponsubtype;
	int randomseed;
	short mousedx;
	short mousedy;
	bool hasbeenpredicted;
	QAngle headangles;
	Vector headoffset;
};

struct ViewSetup {
	int x;
	int oldX;
	int y;
	int oldY;
	int32_t width;
	int32_t oldWidth;
	int32_t height;
	int32_t oldHeight;
	char pad_0020[152];
	float fov;
	float fovViewModel;
	Vector origin;
	Vector angles;
	float zNear;
	float zFar;
	float zNearViewmodel;
	float zFarViewmodel;
	float m_flAspectRatio;
	float m_flNearBlurDepth;
	float m_flNearFocusDepth;
	float m_flFarFocusDepth;
	float m_flFarBlurDepth;
	float m_flNearBlurRadius;
	float m_flFarBlurRadius;
	float m_nDoFQuality;
	int32_t m_nMotionBlurMode;

	float m_flShutterTime;
	Vector m_vShutterOpenPosition;
	QAngle m_shutterOpenAngles;
	Vector m_vShutterClosePosition;
	QAngle m_shutterCloseAngles;

	float m_flOffCenterTop;
	float m_flOffCenterBottom;
	float m_flOffCenterLeft;
	float m_flOffCenterRight;

	bool m_bOffCenter : 1;
	bool m_bRenderToSubrectOfLargerScreen : 1;
	bool m_bDoBloomAndToneMapping : 1;
	bool m_bDoDepthOfField : 1;
	bool m_bHDRTarget : 1;
	bool m_bDrawWorldNormal : 1;
	bool m_bCullFrontFaces : 1;
	bool m_bCacheFullSceneState : 1;
	bool m_bRenderFlashlightDepthTranslucents : 1;
};

class IClientMode {};