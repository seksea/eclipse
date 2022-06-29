#pragma once
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include "../math.hpp"
#include "../../util/memory.hpp"

class Entity;

enum HitGroups {
	HITGROUP_GENERIC = 0,
	HITGROUP_HEAD,
	HITGROUP_CHEST,
	HITGROUP_STOMACH,
	HITGROUP_LEFTARM,
	HITGROUP_RIGHTARM,
	HITGROUP_LEFTLEG,
	HITGROUP_RIGHTLEG,
	HITGROUP_GEAR
};

struct Plane {
	Vector normal;
	float dist;
	unsigned char type;
	unsigned char signbits;
	unsigned char pad[2];
};

struct Surface
{
	const char *name;
	short surfaceProps;
	unsigned short flags;
};

struct Trace
{
	Vector startpos;
	Vector endpos;
	Plane plane;

	float fraction;

	int contents;
	unsigned int dispFlags;

	bool allsolid;
	bool startsolid;

	float fractionleftsolid;

	Surface surface;

	HitGroups hitgroup;
	short physicsbone;

	unsigned short worldSurfaceIndex;
	Entity* m_pEntityHit;
	int hitbox;
};

struct Ray
{
	VectorAligned m_Start;
	VectorAligned m_Delta;
	VectorAligned m_StartOffset;
	VectorAligned m_Extents;

	const matrix3x4_t *m_pWorldAxisTransform;

	bool m_IsRay;
	bool m_IsSwept;

	Ray() : m_pWorldAxisTransform(nullptr)
	{}

	void Init(Vector vecStart, Vector vecEnd)
	{
		m_Delta = vecEnd - vecStart;
		m_IsSwept = (m_Delta.LengthSqr() != 0);
		m_Extents.x = m_Extents.y = m_Extents.z = 0.0f;
		m_pWorldAxisTransform = nullptr;
		m_IsRay = true;
		m_StartOffset.x = m_StartOffset.y = m_StartOffset.z = 0.0f;
		m_Start = vecStart;
	}

	void Init(Vector const &start, Vector const &end, Vector const &mins, Vector const &maxs)
	{
		m_Delta = end - start;

		m_pWorldAxisTransform = NULL;
		m_IsSwept = (m_Delta.LengthSqr() != 0);

		m_Extents = maxs - mins;
		m_Extents *= 0.5f;
		m_IsRay = (m_Extents.LengthSqr() < 1e-6);

		// Offset m_Start to be in the center of the box...
		m_StartOffset = maxs + mins;
		m_StartOffset *= 0.5f;
		m_Start = start + m_StartOffset;
		m_StartOffset *= -1.0f;
	}
};

class ITraceFilter
{
public:
	virtual bool shouldHitEntity(Entity* pEntity, int contentsMask) = 0;

	virtual int getTraceType() const = 0;
};

class TraceFilter : public ITraceFilter
{
public:
	bool shouldHitEntity(Entity* pEntityHandle, int contentsMask)
	{
		return !(pEntityHandle == pSkip);
	}

	virtual int getTraceType() const
	{
		return 0;
	}

	void *pSkip;
};

class IEngineTrace {
public:
	// Returns the contents mask + entity at a particular world-space position
	virtual int getPointContents(const Vector &vecAbsPosition, int contentsMask = 4294967295U/*MASK_ALL*/, Entity** ppEntity = nullptr) = 0;

	const char* clipRayToEntity(Ray &ray, unsigned int fMask, Entity *pEntity,  Trace  *pTrace) {
		typedef const char* (*Fn)(void*, Ray&, unsigned int, Entity*, Trace*);
		return ((Fn)Memory::getVTable(this)[3])(this, ray, fMask, pEntity, pTrace);
	}
    //VFUNC(const char*, traceRay, 5, (Ray &ray, unsigned int fMask, ITraceFilter *pTraceFilter,  Trace  *pTrace), (this, ray, fMask, pTraceFilter, pTrace));
	const char* traceRay(Ray &ray, unsigned int fMask, ITraceFilter *pTraceFilter,  Trace  *pTrace) {
		typedef const char* (*Fn)(void*, Ray&, unsigned int, ITraceFilter*, Trace*);
		return ((Fn)Memory::getVTable(this)[5])(this, ray, fMask, pTraceFilter, pTrace);
	}
};

