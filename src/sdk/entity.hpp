#pragma once
#include <cstdint>
#include <vector>
#include <array>
#include <mutex>
#include "math.hpp"
#include "../util/memory.hpp"
#include "../interfaces.hpp"
#include "netvars.hpp"
#include "../features/visuals.hpp"
#include "../features/skinchanger.hpp"

#include "allnetvars.hpp"
class ICollideable {
public:
	virtual void pad0();
	virtual Vector& OBBMins();
	virtual Vector& OBBMaxs();
};

namespace EntityCache {
	inline Entity* localPlayer;
}

using WeaponArr = std::array<unsigned long, 48>;

class CHudTexture;

class FileWeaponInfo_t
{
public:
	FileWeaponInfo_t();

	// Each game can override this to get whatever values it wants from the script.
	virtual void Parse(KeyValues *pKeyValuesData, const char *szWeaponName);

	bool bParsedScript;
	bool bLoadedHudElements;

	char szClassName[80];
	char szPrintName[80];

	char szViewModel[80];
	char szWorldModel[80];
	char szAmmo1[32];
	char szWorldDroppedModel[80];
	char szAnimationPrefix[16];
	int iSlot;
	int iPosition;
	int iMaxClip1;
	int iMaxClip2;
	int iDefaultClip1;
	int iDefaultClip2;
	int iWeight;
	int iRumbleEffect;
	bool bAutoSwitchTo;
	bool bAutoSwitchFrom;
	int iFlags;
	char szAmmo2[32];
	char szAIAddOn[80];

	// Sound blocks
	char aShootSounds[17][80];

	int iAmmoType;
	int iAmmo2Type;
	bool m_bMeleeWeapon;

	// This tells if the weapon was built right-handed (defaults to true).
	// This helps cl_righthand make the decision about whether to flip the model or not.
	bool m_bBuiltRightHanded;
	bool m_bAllowFlipping;

	// Sprite data, read from the data file
	int iSpriteCount;
	CHudTexture* iconActive;
	CHudTexture* iconInactive;
	CHudTexture* iconAmmo;
	CHudTexture* iconAmmo2;
	CHudTexture* iconCrosshair;
	CHudTexture* iconAutoaim;
	CHudTexture* iconZoomedCrosshair;
	CHudTexture* iconZoomedAutoaim;
	CHudTexture* iconSmall;
};

class CCSWeaponInfo : public FileWeaponInfo_t {
public:
	char* GetConsoleName() {
		return *( char** ) ( ( uintptr_t )this + 0x8);
	}

	int GetClipSize() {
		return *( int* ) ( ( uintptr_t )this + 0x20);
	}
/*
	CSWeaponType GetWeaponType() {
		return *( CSWeaponType* ) ( ( uintptr_t )this + 0x140);
	}

	void SetWeaponType( CSWeaponType type ) {
		*( CSWeaponType* ) ( ( uintptr_t )this + 0x140) = type;
	}
*/
	int GetDamage() {
		return *( int* ) ( ( uintptr_t )this + 0x16C);
	}

	float GetWeaponArmorRatio() {
		return *( float* ) ( ( uintptr_t )this + 0x174);
	}

	float GetPenetration() {
		return *( float* ) ( ( uintptr_t )this + 0x17C);
	}

	float GetRange() {
		return *( float* ) ( ( uintptr_t )this + 0x188);
	}

	float GetRangeModifier() {
		return *( float* ) ( ( uintptr_t )this + 0x18C);
	}

	float GetMaxPlayerSpeed() {
		return *( float* ) ( ( uintptr_t )this + 0x1B8);
	}

	// i haven't updated the offset of this since 22 september 2021 (shark operation or whatever)
	// should be 0x8 to 0x10 higher
	int GetZoomLevels() { // Doesn't work correctly on some weapons.
		return *( int* ) ( ( uintptr_t )this + 0x23C); // DT_WeaponCSBaseGun.m_zoomLevel ?
	}

	char* GetTracerEffect() {
		return *( char** ) ( ( uintptr_t )this + 0x290);
	}

	int* GetTracerFrequency() {
		return ( int* ) ( ( uintptr_t )this + 0x298);
	}
};

class Entity {
    public:
	ALL_NETVARS;

	NETVAR("DT_BaseCombatCharacter", "m_hMyWeapons", weapons, WeaponArr);
	NETVAR("DT_BaseEntity", "m_flSimulationTime", simtime, float);
	
	void* networkable() {
		return reinterpret_cast<void*>(uintptr_t(this) + 16);
	}

	bool visCheck();

	bool canShoot();

	void* renderable() {
		return reinterpret_cast<void*>(uintptr_t(this) + 0x8);
	}

    VFUNC(ClientClass*, clientClass, 2, (), (networkable()))
    VFUNC(void, onPreDataChanged, 4, (int updateType), (networkable(), updateType))
    VFUNC(void, onDataChanged, 5, (int updateType), (networkable(), updateType))
    VFUNC(void, preDataUpdate, 6, (int updateType), (networkable(), updateType))
    VFUNC(void, postDataUpdate, 7, (int updateType), (networkable(), updateType))
    VFUNC(bool, dormant, 9, (), (networkable()))
    VFUNC(int, index, 10, (), (networkable()))

    VFUNC(bool, shouldDraw, 5, (), (renderable()))
    VFUNC(bool, setupBones, 13, (matrix3x4_t* boneMatrix, int maxBones, int boneMask, float curTime = 0), (renderable(), boneMatrix, maxBones, boneMask, curTime))

    VFUNC(Vector&, origin, 12, (), (this))
    VFUNC(void, setModelIndex, 111, (int index), (this, index))

	VFUNC(float, getSpread, 521, (), (this))
	VFUNC(float, getInaccuracy, 551, (), (this))

	VFUNC(CCSWeaponInfo*, getCSWpnData, 529, (), (this))

	bool teammate() {
		if (strstr(Interfaces::engine->getLevelName(), "dz_")) return false;
		static Convar* mp_teammates_are_enemies = Interfaces::cvar->findVar("mp_teammates_are_enemies");
		if (mp_teammates_are_enemies->getInt()) return false;
		return this->nDT_BaseEntity__m_iTeamNum() == EntityCache::localPlayer->nDT_BaseEntity__m_iTeamNum();
	}
	
	Vector eyepos() {
		return EntityCache::localPlayer->origin() + Vector(0, 0, (EntityCache::localPlayer->nDT_BasePlayer__m_fFlags() & (1 << 1)) ? 46 : 64);
	}

	const matrix3x4_t coordinateFrame() {
		return *(matrix3x4_t*)((uintptr_t)this + 0x518);
	}

	const int moveType() {
		return *reinterpret_cast<int*>((uintptr_t)this + Netvars::netvars.at({"DT_BaseEntity", "m_nRenderMode"}).second + 1);
	}
};

namespace EntityCache {
	enum class EntityType {
		PLAYER,
		DROPPEDWEAPON,
		INVALID
	};

	struct CachedEntity {
		int index;
		EntityType type = EntityType::INVALID;
		Vector origin;
		int classID;
		ImVec4 boundingBox;
		int health;
		bool teammate;
		bool visible;
		bool spotted;
		PlayerInfo info;
		std::string_view weaponName;
		CachedEntity(Entity* e) {
			this->index = e->index();
			this->origin = e->origin();
			this->classID = e->clientClass()->m_ClassID;
			this->boundingBox = getBoundingBox(e);

			if ((e->clientClass()->m_ClassID != ClassId::CBaseWeaponWorldModel && strstr(e->clientClass()->m_pNetworkName, "Weapon")) || e->clientClass()->m_ClassID == ClassId::CDEagle || e->clientClass()->m_ClassID == ClassId::CC4 || e->clientClass()->m_ClassID == ClassId::CAK47) { // if is weapon
				if (e->nDT_BaseCombatWeapon__m_hOwner() == -1) {
					this->type = EntityType::DROPPEDWEAPON;
					if (SkinChanger::itemIndexToNameMap.find((ItemIndex)(e->nDT_ScriptCreatedItem__m_iItemDefinitionIndex() & 0xFFF)) != SkinChanger::itemIndexToNameMap.end()) {
						this->weaponName = SkinChanger::itemIndexToNameMap.at((ItemIndex)(e->nDT_ScriptCreatedItem__m_iItemDefinitionIndex() & 0xFFF));
					}
					else {
						this->weaponName = "invalid item";
					}
				}
			}

			if (this->index <= 64) { // if player
				this->type = EntityType::PLAYER;
				this->health = e->nDT_BasePlayer__m_iHealth();
				this->teammate = e->teammate();
				Interfaces::engine->getPlayerInfo(this->index, this->info);
				visible = e->visCheck();
				spotted = e->nDT_BaseEntity__m_bSpotted();
			}
		}
	};
	
	inline std::vector<CachedEntity> entityCache;
	inline std::mutex entityCacheLock;

	void cacheEntities();
}
