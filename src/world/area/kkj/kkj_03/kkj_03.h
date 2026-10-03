#pragma once

/// @file kkj_03.h
/// @brief Peach's Castle - Intro Window Hallway (4F)

#include "common.h"
#include "message_ids.h"
#include "map.h"

#include "../kkj.h"
#include "mapfs/kkj_03_shape.h"
#include "mapfs/kkj_03_hit.h"

// Kirby: Adding these
#include "sprite/npc/WorldBowser.h"
#include "sprite/npc/WorldKammy.h"
#include "sprite/npc/WorldKoopatrol.h"

/*
enum {
    NPC_Peach   = 0,
};
*/
// Replacing this enum
enum {
    // shared
    NPC_Bowser_Body     = 0,
    NPC_Bowser_Prop     = 1,
    // chapter 8
    NPC_CaptivePeach    = 2,
    // intro
    NPC_Koopatrol_01    = 2,
    NPC_Koopatrol_02    = 3,
    NPC_Kammy           = 4,
    NPC_Peach           = 5,
};

#define NAMESPACE kkj_03

extern EvtScript N(EVS_Main);
extern EvtScript N(EVS_SetupMusic);
extern EvtScript N(EVS_Scene_MeetingPeach);
extern EvtScript N(EVS_Scene_Ascending);

extern NpcGroupList N(DefaultNPCs);
