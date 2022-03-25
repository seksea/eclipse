#include "../../interfaces.hpp"
#include "../entity.hpp"

namespace Prediction {
    void startPrediction(CUserCmd* cmd) {
        inPrediction = true;
        if (!EntityCache::localPlayer)
            return;
        
        *Interfaces::predictionSeed = rand() & 0x7FFFFFFF;

        oldCurtime = Interfaces::globals->curtime;
        oldFrametime = Interfaces::globals->frametime;

        Interfaces::globals->curtime = EntityCache::localPlayer->nDT_LocalPlayerExclusive__m_nTickBase() * Interfaces::globals->interval_per_tick;
        Interfaces::globals->frametime = Interfaces::globals->interval_per_tick;

        Interfaces::movement->startTrackPredictionErrors(EntityCache::localPlayer);

        Interfaces::moveHelper->setHost(EntityCache::localPlayer);
        Interfaces::prediction->setupMove(EntityCache::localPlayer, cmd, Interfaces::moveHelper, Interfaces::moveData);
        Interfaces::movement->processMovement(EntityCache::localPlayer, Interfaces::moveData);
        Interfaces::prediction->finishMove(EntityCache::localPlayer, cmd, Interfaces::moveData);
    }

    void endPrediction() {
        if (!EntityCache::localPlayer) {
            return;
        }

        Interfaces::movement->finishTrackPredictionErrors(EntityCache::localPlayer);
        Interfaces::moveHelper->setHost(0);

        *Interfaces::predictionSeed = -1;

        Interfaces::globals->curtime = oldCurtime;
        Interfaces::globals->frametime = oldFrametime;
        inPrediction = false;
    }
}