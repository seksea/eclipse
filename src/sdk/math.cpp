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

	Vector min = e->nDT_BaseEntity__m_Collision().OBBMins() + e->origin();
	Vector max = e->nDT_BaseEntity__m_Collision().OBBMaxs() + e->origin();

	Vector points[] = { Vector( min.x, min.y, min.z ),
						Vector( min.x, max.y, min.z ),
						Vector( max.x, max.y, min.z ),
						Vector( max.x, min.y, min.z ),
						Vector( max.x, max.y, max.z ),
						Vector( min.x, max.y, max.z ),
						Vector( min.x, min.y, max.z ),
						Vector( max.x, min.y, max.z ) };

	// Get screen positions
	if ( !worldToScreen( points[3], flb ) || !worldToScreen( points[5], brt )
		 || !worldToScreen( points[0], blb ) || !worldToScreen( points[4], frt )
		 || !worldToScreen( points[2], frb ) || !worldToScreen( points[1], brb )
		 || !worldToScreen( points[6], blt ) || !worldToScreen( points[7], flt ) )
		return ImVec4(-1000, -1000, -1000, -1000);

	Vector arr[] = { flb, brt, blb, frt, frb, brb, blt, flt };

	float left = flb.x;
	float top = flb.y;
	float right = flb.x;
	float bottom = flb.y;

	for ( int i = 1; i < 8; i++ ) {
		if (left > arr[i].x)
			left = arr[i].x;
		if (bottom < arr[i].y)
			bottom = arr[i].y;
		if (right < arr[i].x)
			right = arr[i].x;
		if (top > arr[i].y)
			top = arr[i].y;
	}

	return ImVec4(left, top, right, bottom);
}