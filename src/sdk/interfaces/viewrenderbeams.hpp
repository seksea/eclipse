#pragma once
#include <cstddef>
#include "../math.hpp"
#include "../../util/memory.hpp"

struct BeamInfo {
    int	type = 0;
    Entity* startEnt = nullptr;
    int startAttachment = 0;
    Entity* endEnt = nullptr;
    int	endAttachment = 0;
    Vector start = Vector(0, 0, 0);
    Vector end = Vector(0, 0, 0);
    int modelIndex = 0;
    const char* modelName;
    int haloIndex = 0;
    const char* haloName = nullptr;
    float haloScale = 0.f;
    float life = 0.f;
    float width = 0.f;
    float endWidth = 0.f;
    float fadeLength = 0.f;
    float amplitude = 0.f;
    float brightness = 0.f;
    float speed = 0.f;
    int	startFrame = 0;
    float frameRate = 0.f;
    float red = 0.f;
    float green = 0.f;
    float blue = 0.f;
    bool renderable = 0;
    int segments = 0;
    int	flags = 0;
    Vector ringCenter = Vector(0, 0, 0);
    float ringStartRadius = 0.f;
    float ringEndRadius = 200.f;
};

struct Beam {
    char a[76];
    int flags;
    char b[144];
    float die;
};

class ViewRenderBeams {
public:
    VFUNC(Beam*, createBeamPoints, 9, (BeamInfo* beamInfo), (this, beamInfo));
    VFUNC(Beam*, createBeamRingPoints, 11, (BeamInfo* beamInfo), (this, beamInfo));
};