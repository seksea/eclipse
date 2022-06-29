#pragma once
#include "../interfaces.hpp"

namespace Ragebot {
    // awall shit
    struct fireBulletData
	{
		Vector src;
		Trace enter_trace;
		Vector direction;
		TraceFilter filter;
		float trace_length;
		float trace_length_remaining;
		float current_damage;
		int penetrate_count;
	};

	float GetDamage(const Vector& vecPoint, bool teamCheck, fireBulletData& fData);

    bool shouldWallbang(QAngle viewAngles, int minChance, int minDamage = 0);
}