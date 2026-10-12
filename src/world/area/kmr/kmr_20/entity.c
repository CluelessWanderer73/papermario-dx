#include "kmr_20.h"
#include "entity.h"
// Kirby: I have to add this here
#include "sprite/player.h"
#include "effects.h"
#include "include_asset.h"
// and all the stuff below
#include "world/area/kmr/kmr_04/hammer_block_message.png.h"
INCLUDE_IMG("world/area/kmr/kmr_04/hammer_block_message.png", kmr_04_hammer_block_message_img);
INCLUDE_PAL("world/area/kmr/kmr_04/hammer_block_message.pal", kmr_04_hammer_block_message_pal);

static MessageImageData MessageImage;

API_CALLABLE(SetMessageImage_HammerBlock) {
    MessageImage.raster = kmr_04_hammer_block_message_img;
    MessageImage.palette = kmr_04_hammer_block_message_pal;
    MessageImage.width = kmr_04_hammer_block_message_img_width;
    MessageImage.height = kmr_04_hammer_block_message_img_height;
    MessageImage.format = G_IM_FMT_CI;
    MessageImage.bitDepth = G_IM_SIZ_4b;
    set_message_images(&MessageImage);
    return ApiStatus_DONE2;
}


// Kirby: Adding the upgrade song
EvtScript EVS_PlayUpgradeSong = {
    Call(PushSong, SONG_ITEM_UPGRADE, 0)
    Wait(130)
    Call(PopSong)
    Return
    End
};
// end added stuff

API_CALLABLE(SetAmbienceVolumeHalf_Entity){
    snd_ambient_set_volume(0, 1000, 63);
    return ApiStatus_DONE2;
}

EvtScript EVS_SecretPanel_FlipBack = {
    Call(ModifyColliderFlags, MODIFY_COLLIDER_FLAGS_SET_BITS, COLLIDER_o252, COLLIDER_FLAGS_UPPER_MASK)
    Call(PlaySoundAtCollider, COLLIDER_o252, SOUND_FLIP_PANEL, SOUND_SPACE_DEFAULT)
    Call(SetAmbienceVolumeHalf_Entity)
    Call(MakeLerp, 0, -2160, 60, EASING_QUADRATIC_OUT)
    Loop(0)
        Call(UpdateLerp)
        Call(RotateGroup, MODEL_g61, LVar0, 1, 0, 0)
        Wait(1)
        IfEq(LVar1, 0)
            BreakLoop
        EndIf
    EndLoop
    Call(PlaySoundAtCollider, COLLIDER_o252, SOUND_CLOSE_PANEL, SOUND_SPACE_DEFAULT)
    Call(ModifyColliderFlags, MODIFY_COLLIDER_FLAGS_CLEAR_BITS, COLLIDER_o252, COLLIDER_FLAGS_UPPER_MASK)
    Return
    End
};

EvtScript EVS_TetherCamToPlayer = {
    Loop(0)
        Call(GetPlayerPos, LVar0, LVar1, LVar2)
        Call(SetCamTarget, CAM_DEFAULT, LVar0, LVar1, LVar2)
        Wait(1)
    EndLoop
    Return
    End
};

EvtScript EVS_UseSpring_Basement = {
    Call(DisablePlayerInput, true)
    Call(DisablePlayerPhysics, true)
    Call(SetPlayerActionState, ACTION_STATE_JUMP)
    Wait(1)
    ExecGetTID(EVS_TetherCamToPlayer, LVarA)
    Call(SetPlayerJumpscale, Float(0.7))
    IfEq(AF_KMR20_SpringCanLaunch, false)
        // bounce off the spring when falling from above
        Call(PlayerJump, -110, -80, -35, 20)
        Set(AF_KMR20_SpringCanLaunch, true)
    Else
        // launch out of the basement
        Call(EnableCameraFollowPlayerY)
        Exec(EVS_SecretPanel_FlipBack)
        Call(PlayerJump, -150, 30, -90, 30)
        Call(InterpPlayerYaw, 180, 0)
        Set(AF_KMR20_SpringCanLaunch, false)
    EndIf
    Wait(30)
    KillThread(LVarA)
    Call(DisablePlayerPhysics, false)
    Call(DisablePlayerInput, false)
    Call(SetPlayerActionState, ACTION_STATE_IDLE)
    Return
    End
};

EvtScript EVS_Luigi_JumpFromBlock = {
    Call(DisablePlayerInput, true)
    Call(SetEnemyFlagBits, NPC_Luigi_1, ENEMY_FLAG_IGNORE_WORLD_COLLISION, false)
    Call(SetNpcFlagBits, NPC_Luigi_1, NPC_FLAG_GRAVITY, true)
    Call(SetNpcVar, NPC_Luigi_1, 0, 1)
    Call(SetNpcAnimation, NPC_Luigi_1, ANIM_Luigi_Jump)
    Call(SetNpcJumpscale, NPC_Luigi_1, 1)
    Call(GetNpcPos, NPC_Luigi_1, LVar0, LVar1, LVar2)
    Call(NpcJump0, NPC_Luigi_1, LVar0, 0, LVar2, 25)
    Call(SetNpcAnimation, NPC_Luigi_1, ANIM_Luigi_Idle)
    Call(SetNpcVar, NPC_Luigi_1, 0, 2)
    Call(DisablePlayerInput, false)
    Return
    End
};

EvtScript EVS_Luigi_JumpFromHighBlock = {
    Call(DisablePlayerInput, true)
    Call(SetEnemyFlagBits, NPC_Luigi_1, ENEMY_FLAG_IGNORE_WORLD_COLLISION, false)
    Call(SetNpcFlagBits, NPC_Luigi_1, NPC_FLAG_GRAVITY, true)
    Call(SetNpcVar, NPC_Luigi_1, 0, 1)
    Call(SetNpcAnimation, NPC_Luigi_1, ANIM_Luigi_Jump)
    Call(SetNpcJumpscale, NPC_Luigi_1, 1)
    Call(GetNpcPos, NPC_Luigi_1, LVar0, LVar1, LVar2)
    Add(LVar0, 50)
    Call(NpcJump0, NPC_Luigi_1, LVar0, 0, LVar2, 25)
    Call(SetNpcAnimation, NPC_Luigi_1, ANIM_Luigi_Idle)
    Call(SetNpcVar, NPC_Luigi_1, 0, 2)
    Call(DisablePlayerInput, false)
    Return
    End
};

EvtScript EVS_BlastRock = {
    Call(SetNpcAux, NPC_Luigi_1, 0)
    Exec(EVS_Luigi_JumpFromBlock)
    Return
    End
};

EvtScript EVS_SmashBlock = {
    Call(SetNpcAux, NPC_Luigi_1, 0)
    Thread
        ExecWait(EVS_Luigi_JumpFromBlock)
    EndThread
    Return
    End
};

EvtScript EVS_BreakBlock_Brick = {
    Call(SetNpcAux, NPC_Luigi_1, 0)
    Thread
        ExecWait(EVS_Luigi_JumpFromHighBlock)
    EndThread
    Return
    End
};

EvtScript EVS_MakeEntities = {
    Call(MakeEntity, Ref(Entity_ScriptSpring), -150, -80, -50, 0, MAKE_ENTITY_END)
    Call(AssignScript, Ref(EVS_UseSpring_Basement))
    Call(SetEntityCullMode, 1)
    Switch(GB_StoryProgress)
        CaseLt(STORY_CH1_STAR_SPRIT_DEPARTED)
        CaseLt(STORY_CH2_STAR_SPRIT_DEPARTED)
            Call(MakeEntity, Ref(Entity_BombableRock), 300, 0, 150, 0, MAKE_ENTITY_END)
            Call(AssignScript, Ref(EVS_BlastRock))
        CaseLt(STORY_CH3_STAR_SPIRIT_RESCUED)
        CaseLt(STORY_CH4_STAR_SPRIT_DEPARTED)
        CaseLt(STORY_CH5_STAR_SPRIT_DEPARTED)
        CaseLt(STORY_CH6_STAR_SPIRIT_RESCUED)
            Call(MakeEntity, Ref(Entity_Hammer3Block), 300, 0, 150, 55, MAKE_ENTITY_END)
            Call(AssignScript, Ref(EVS_SmashBlock))
        CaseLt(STORY_CH7_STAR_SPRIT_DEPARTED)
            Call(MakeEntity, Ref(Entity_BrickBlock), 300, 90, 150, 55, MAKE_ENTITY_END)
            Call(AssignScript, Ref(EVS_BreakBlock_Brick))
        CaseLt(STORY_EPILOGUE)
    EndSwitch
    // Kirby: Adding this cause I need to put an item you can pick up to get the hammer
    Call(MakeItemEntity, ITEM_HAMMER, 141, 51, -49, ITEM_SPAWN_MODE_KEY, GF_Unused_KMR_0E)
    // Looks like while this works for normal pick-up items, it won't work for the hammer
    // so I'll add a script to it!
    // Attempt 1: This doesn't work unfortunately, I guess because AssignScript is for entities and
    // not item entities. Whatever :/
    // Attempt 2: I just needed to change the item spawn mode to ITEM_SPAWN_MODE_KEY
    Return
    End
};
