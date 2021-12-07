#pragma once
#include "../../util/memory.hpp"
#include "../keyvalues.hpp"

//-----------------------------------------------------------------------------
// Shader state flags can be read from the FLAGS materialvar
// Also can be read or written to with the Set/GetMaterialVarFlags() call
// Also make sure you add/remove a string associated with each flag below to CShaderSystem::ShaderStateString in ShaderSystem.cpp
//-----------------------------------------------------------------------------
enum MaterialVarFlags {
	MATERIAL_VAR_DEBUG					  = (1 << 0),
	MATERIAL_VAR_NO_DEBUG_OVERRIDE		  = (1 << 1),
	MATERIAL_VAR_NO_DRAW				  = (1 << 2),
	MATERIAL_VAR_USE_IN_FILLRATE_MODE	  = (1 << 3),

	MATERIAL_VAR_VERTEXCOLOR			  = (1 << 4),
	MATERIAL_VAR_VERTEXALPHA			  = (1 << 5),
	MATERIAL_VAR_SELFILLUM				  = (1 << 6),
	MATERIAL_VAR_ADDITIVE				  = (1 << 7),
	MATERIAL_VAR_ALPHATEST				  = (1 << 8),
	MATERIAL_VAR_MULTIPASS				  = (1 << 9),
	MATERIAL_VAR_ZNEARER				  = (1 << 10),
	MATERIAL_VAR_MODEL					  = (1 << 11),
	MATERIAL_VAR_FLAT					  = (1 << 12),
	MATERIAL_VAR_NOCULL					  = (1 << 13),
	MATERIAL_VAR_NOFOG					  = (1 << 14),
	MATERIAL_VAR_IGNOREZ				  = (1 << 15),
	MATERIAL_VAR_DECAL					  = (1 << 16),
	MATERIAL_VAR_ENVMAPSPHERE			  = (1 << 17),
	MATERIAL_VAR_NOALPHAMOD				  = (1 << 18),
	MATERIAL_VAR_ENVMAPCAMERASPACE	      = (1 << 19),
	MATERIAL_VAR_BASEALPHAENVMAPMASK	  = (1 << 20),
	MATERIAL_VAR_TRANSLUCENT              = (1 << 21),
	MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK = (1 << 22),
	MATERIAL_VAR_NEEDS_SOFTWARE_SKINNING  = (1 << 23),
	MATERIAL_VAR_OPAQUETEXTURE			  = (1 << 24),
	MATERIAL_VAR_ENVMAPMODE				  = (1 << 25),
	MATERIAL_VAR_SUPPRESS_DECALS		  = (1 << 26),
	MATERIAL_VAR_HALFLAMBERT			  = (1 << 27),
	MATERIAL_VAR_WIREFRAME                = (1 << 28),
	MATERIAL_VAR_ALLOWALPHATOCOVERAGE     = (1 << 29),
	MATERIAL_VAR_IGNORE_ALPHA_MODULATION  = (1 << 30),

	// NOTE: Only add flags here that either should be read from
	// .vmts or can be set directly from client code. Other, internal
	// flags should to into the flag enum in imaterialinternal.h
};

class IMaterialVar {
public:
    VFUNC(void, setVecValue, 12, (float x, float y, float z), (this, x, y, z))
};

class IMaterial {
public:
	virtual const char*	GetName();
	virtual const char*	GetTextureGroupName();

    VFUNC(IMaterialVar*, findVar, 11, (const char *varName, bool *found, bool complain = true), (this, varName, found, complain))
    VFUNC(void, alphaModulate, 27, (float alpha), (this, alpha))
    VFUNC(void, colorModulate, 28, (float r, float g, float b), (this, r, g, b))
    VFUNC(void, setMaterialVarFlag, 29, (MaterialVarFlags flags, bool enabled), (this, flags, enabled))
};

class IMaterialSystem {
public:
    VFUNC(IMaterial*, createMaterial, 83, (const char* materialName, KeyValues* keyValues), (this, materialName, keyValues))
    VFUNC(IMaterial*, findMaterial, 84, (const char* materialName, const char* textureGroupName, bool complain = true), (this, materialName, textureGroupName, complain))
};