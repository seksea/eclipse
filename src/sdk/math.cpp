#include "math.hpp"
#include "entity.hpp"
#include "../menu/imgui/imgui.h"

bool worldToScreen(const Vector& origin, Vector& screen) {
	float w = storedViewMatrix[3][0] * origin.x
			  + storedViewMatrix[3][1] * origin.y
			  + storedViewMatrix[3][2] * origin.z
			  + storedViewMatrix[3][3];

	if ( w < 0.01f )
		return false;

	float inverseW = 1 / w;

	screen.x = (ImGui::GetIO().DisplaySize.x/2) + (0.5f * ((storedViewMatrix[0][0] * origin.x + storedViewMatrix[0][1] * origin.y +
						  			 storedViewMatrix[0][2] * origin.z + storedViewMatrix[0][3]) * inverseW) * ImGui::GetIO().DisplaySize.x + 0.5f);

	screen.y = (ImGui::GetIO().DisplaySize.y/2) - (0.5f * ((storedViewMatrix[1][0] * origin.x + storedViewMatrix[1][1] * origin.y +
									  storedViewMatrix[1][2] * origin.z + storedViewMatrix[1][3]) * inverseW) * ImGui::GetIO().DisplaySize.y + 0.5f);
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
        if (bottom < screen_points[i].y)
            bottom = screen_points[i].y;
        if (right < screen_points[i].x)
            right = screen_points[i].x;
        if (top > screen_points[i].y)
            top = screen_points[i].y;
    }
    
    return ImVec4(floor(left), floor(top), floor(right), floor(bottom));
}

QAngle calcAngle(const Vector& src, const Vector& dst) {
    QAngle vAngle;
    Vector delta((src.x - dst.x), (src.y - dst.y), (src.z - dst.z));
    double hyp = sqrt(delta.x*delta.x + delta.y*delta.y);

    vAngle.x = float(atanf(float(delta.z / hyp)) * 57.295779513082f);
    vAngle.y = float(atanf(float(delta.y / delta.x)) * 57.295779513082f);
    vAngle.z = 0.0f;

    if (delta.x >= 0.0)
        vAngle.y += 180.0f;

    return vAngle;
}