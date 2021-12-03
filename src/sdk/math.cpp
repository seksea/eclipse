#include "math.hpp"
#include "entity.hpp"
#include "../menu/imgui/imgui.h"

bool worldToScreen( const Vector& origin, Vector& screen ) {
    VMatrix w2sm = Interfaces::engine->worldToScreenMatrix();
	float w = w2sm[3][0] * origin.x
			  + w2sm[3][1] * origin.y
			  + w2sm[3][2] * origin.z
			  + w2sm[3][3];

	if ( w < 0.01f )
		return false;

	float inverseW = 1 / w;

	screen.x = (ImGui::GetIO().DisplaySize.x/2) + (0.5f * ((w2sm[0][0] * origin.x + w2sm[0][1] * origin.y +
						  			 w2sm[0][2] * origin.z + w2sm[0][3]) * inverseW) * ImGui::GetIO().DisplaySize.x + 0.5f);

	screen.y = (ImGui::GetIO().DisplaySize.y/2) - (0.5f * ((w2sm[1][0] * origin.x + w2sm[1][1] * origin.y +
									  w2sm[1][2] * origin.z + w2sm[1][3]) * inverseW) * ImGui::GetIO().DisplaySize.y + 0.5f);
	return true;
}

ImVec4 getBoundingBox(Entity* e) {
	Vector flb, brt, blb, frt, frb, brb, blt, flt; // think of these as Front-Left-Bottom/Front-Left-Top... Etc.

	Vector min = e->nDT_BaseEntity__m_Collision().OBBMins();
	Vector max = e->nDT_BaseEntity__m_Collision().OBBMaxs();
	matrix3x4_t coordinateFrame = e->coordinateFrame();

    Vector points[] = {
        Vector(min.x, min.y, min.z),
        Vector(min.x, max.y, min.z),
        Vector(max.x, max.y, min.z),
        Vector(max.x, min.y, min.z),
        Vector(max.x, max.y, max.z),
        Vector(min.x, max.y, max.z),
        Vector(min.x, min.y, max.z),
        Vector(max.x, min.y, max.z)
    };

    Vector pointsTransformed[8] = {};
    for (int i = 0; i < 8; i++) {
        VectorTransform(points[i], coordinateFrame, pointsTransformed[i]);
    }

    Vector screen_points[8] = {};

    for (int i = 0; i < 8; i++) {
        if (!worldToScreen(pointsTransformed[i], screen_points[i])) {
            return ImVec4(-1, -1, -1, -1);
        }
    }

    auto left = screen_points[0].x;
    auto top = screen_points[0].y;
    auto right = screen_points[0].x;
    auto bottom = screen_points[0].y;

    for (int i = 0; i < 8; i++) {
        if (left > screen_points[i].x)
            left = screen_points[i].x;
        if (top < screen_points[i].y)
            top = screen_points[i].y;
        if (right < screen_points[i].x)
            right = screen_points[i].x;
        if (bottom > screen_points[i].y)
            bottom = screen_points[i].y;
    }
    
    return ImVec4(left, top, right, bottom);
}