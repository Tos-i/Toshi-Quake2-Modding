#include "g_local.h"

void ChasecamTrack(edict_t* ent);

void ChasecamStart(edict_t* ent) {
	
	edict_t* chasecam;

	ent->client->chasetoggle = 1;
	ent->client->ps.gunindex = 0;

	chasecam = G_Spawn();
	chasecam->owner = ent;
	chasecam->solid = SOLID_NOT;
	chasecam->movetype = MOVETYPE_FLYMISSILE;
	ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
	ent->svflags |= SVF_NOCLIENT;

	VectorCopy(ent->s.angles, chasecam->s.angles);
	VectorClear(chasecam->mins);
	VectorClear(chasecam->maxs);
	VectorCopy(ent->s.origin, chasecam->s.origin);

	chasecam->classname = "chasecam";
    chasecam->nextthink = level.time + 0.100;
	chasecam->think = ChasecamTrack;

	ent->client->chasecam = chasecam;
	ent->client->oldplayer = G_Spawn();
}

void ChasecamRestart(edict_t* ent){
    if (ent->owner->health <= 0){
        G_FreeEdict(ent);
        return;
    }

    if (ent->owner->waterlevel) return;
    ChasecamStart(ent->owner);
    G_FreeEdict(ent);
}

void ChasecamRemove(edict_t* ent, char* opt){

    VectorClear(ent->client->chasecam->velocity);
    ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
    ent->s.modelindex = ent->client->oldplayer->s.modelindex;

    if (!strcmp(opt, "background")){
        ent->client->chasetoggle = 3;
        ent->client->chasecam->nextthink = level.time + 0.100;
        ent->client->chasecam->think = ChasecamRestart;
    }
    else if (!strcmp(opt, "off")){
        ent->client->chasetoggle = 0;
        G_FreeEdict(ent->client->oldplayer);
        G_FreeEdict(ent->client->chasecam);
    }
}

void ChasecamTrack(edict_t* ent){
    trace_t      tr;
    vec3_t       spot1, spot2, dir;
    vec3_t       forward, right, up;
    int          distance;
    int          tot;

    ent->nextthink = level.time + 0.100;

    if (ent->owner->waterlevel)
    {
        ChasecamRemove(ent, "background");
        return;
    }
    AngleVectors(ent->owner->client->v_angle, forward, right, up);

    VectorMA(ent->owner->s.origin, ent->chasedist1, forward, spot2);

    spot2[2] = (spot2[2] + 20.000);

    if (ent->owner->client->v_angle[0] < 0.000) VectorMA(spot2, (ent->owner->client->v_angle[0] * 0.6), up, spot2);

    else if (ent->owner->client->v_angle[0] > 0.000) VectorMA(spot2, (ent->owner->client->v_angle[0] * 0.6), up, spot2);

    tr = gi.trace(ent->owner->s.origin, NULL, NULL, spot2, ent->owner, false);

    VectorSubtract(tr.endpos, ent->owner->s.origin, spot1);
    ent->chasedist1 = -VectorLength(spot1);

    VectorMA(tr.endpos, 2, forward, spot2);

    VectorCopy(spot2, spot1);

    spot1[2] -= 32;

    tr = gi.trace(spot2, NULL, NULL, spot1, ent->owner, false);

    if (tr.fraction < 1.000)
    {
        VectorCopy(tr.endpos, spot2);
        spot2[2] += 32;
    }

    VectorSubtract(spot2, ent->s.origin, dir);

    VectorNormalize(dir);

    VectorSubtract(spot2, ent->s.origin, spot1);

    distance = VectorLength(spot1);

    tr = gi.trace(ent->s.origin, NULL, NULL, spot2, ent->owner, false);

    if (tr.fraction == 1.000)
    {
        VectorSubtract(ent->s.origin, ent->owner->s.origin, spot1);
        VectorNormalize(spot1);
        VectorCopy(spot1, ent->s.angles);
        tot = (distance * -0.400);

        if (tot > 5.200)
        {
            ent->velocity[0] = ((dir[0] * distance) * 5.2);
            ent->velocity[1] = ((dir[1] * distance) * 5.2);
            ent->velocity[2] = ((dir[2] * distance) * 5.2);
        }
        else
        {
            if ((tot > 1.000))
            {
                ent->velocity[0] = ((dir[0] * distance) * tot);
                ent->velocity[1] = ((dir[1] * distance) * tot);
                ent->velocity[2] = ((dir[2] * distance) * tot);
            }
            else
            {
                ent->velocity[0] = (dir[0] * distance);
                ent->velocity[1] = (dir[1] * distance);
                ent->velocity[2] = (dir[2] * distance);
            }
        }
        VectorSubtract(ent->owner->s.origin, ent->s.origin, spot1);

        if (VectorLength(spot1) < 20)
        {
            ent->velocity[0] *= 2;
            ent->velocity[1] *= 2;
            ent->velocity[2] *= 2;
        }
    }
    else VectorCopy(spot2, ent->s.origin);

    ent->chasedist1 += 2;

    if (ent->chasedist1 > 60.00) ent->chasedist1 = 60.000;
    if (ent->movedir == ent->s.origin)
    {
        if (distance > 20) ent->chasedist2++;
    }

    if (ent->chasedist2 > 3)
    {
        ChasecamStart(ent->owner);
        G_FreeEdict(ent);
        return;
    }

    VectorCopy(ent->s.origin, ent->movedir);
}

void Cmd_Chasecam_Toggle(edict_t *ent)
{
    // Added by WarZone - Begin
    if (!ent->waterlevel && !ent->deadflag)
    {
        if (ent->client->chasetoggle)
            ChasecamRemove(ent, "off");
        else
            ChasecamStart(ent);
    }
    else if (ent->waterlevel && !ent->deadflag) gi.cprintf(ent, PRINT_HIGH, "Camera cannot be modified while in water\n");
    // Added by WarZone - End
}

void CheckChasecam_Viewent(edict_t* ent)
{
    // Added by WarZone - Begin
    gclient_t* cl;
    if (!ent->client->oldplayer->client)
    {
        cl = (gclient_t*)malloc(sizeof(gclient_t));
        ent->client->oldplayer->client = cl;
    }
    // Added by WarZone - End
    if ((ent->client->chasetoggle == 1) && (ent->client->oldplayer))
    {
        ent->client->oldplayer->s.frame = ent->s.frame;
        VectorCopy(ent->s.origin, ent->client->oldplayer->s.origin);
        VectorCopy(ent->velocity, ent->client->oldplayer->velocity);
        VectorCopy(ent->s.angles, ent->client->oldplayer->s.angles);
        ent->client->oldplayer->s.modelindex = ent->s.modelindex;
        ent->client->oldplayer->s.modelindex2 = ent->s.modelindex2;

        gi.linkentity(ent->client->oldplayer);
    }
}

