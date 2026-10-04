#include "kkj_03.h"
#include "sprite/player.h"

// Kirby: Adding these
#include "effects.h"
#include "sprite.h"

// Kirby: I have to add my own API call to open the status bar
API_CALLABLE(N(OpenStatusBar)) {
    open_status_bar_quickly();
    return ApiStatus_DONE2;
}
API_CALLABLE(N(CloseStatusBar)) {
    close_status_bar();
    return ApiStatus_DONE2;
}

// Kirby: Adding all this event crap too
API_CALLABLE(N(UpdatePropellerSoundPos_Intro)) {
    Npc* npc = get_npc_safe(NPC_Bowser_Prop);
    f32 x = npc->pos.x;
    f32 y = npc->pos.y;
    f32 z = npc->pos.z;

    sfx_adjust_env_sound_pos(SOUND_LRAW_BOWSER_PROPELLER, SOUND_SPACE_DEFAULT, x, y, z);
    script->varTable[0] = y;

    return ApiStatus_DONE2;
}

EvtScript N(EVS_UpdatePropellerSounds_Intro) = {
    Call(PlaySoundAtNpc, NPC_Bowser_Prop, SOUND_LOOP_BOWSER_PROPELLER, SOUND_SPACE_DEFAULT)
    Loop(0)
        Call(N(UpdatePropellerSoundPos_Intro))
        IfLt(LVar0, 0)
            BreakLoop
        EndIf
        Wait(1)
    EndLoop
    Call(StopSound, SOUND_LOOP_BOWSER_PROPELLER)
    Return
    End
};

API_CALLABLE(N(ScreenWhiteFlashOn)) {
    if (isInitialCall) {
        script->functionTemp[1] = 0;
    }

    script->functionTemp[1] += 64;

    if (script->functionTemp[1] > 255) {
        script->functionTemp[1] = 255;
    }

    set_screen_overlay_color(SCREEN_LAYER_FRONT, 160, 160, 160);
    set_screen_overlay_params_front(OVERLAY_VIEWPORT_COLOR, script->functionTemp[1]);

    if (script->functionTemp[1] == 255) {
        return ApiStatus_DONE2;
    }

    return ApiStatus_BLOCK;
}

API_CALLABLE(N(ScreenWhiteFlashOff)) {
    if (isInitialCall) {
        script->functionTemp[1] = 255;
    }

    script->functionTemp[1] -= 64;

    if (script->functionTemp[1] <= 0) {
        script->functionTemp[1] = 0;
    }

    set_screen_overlay_color(SCREEN_LAYER_FRONT, 160, 160, 160);
    set_screen_overlay_params_front(OVERLAY_VIEWPORT_COLOR, script->functionTemp[1]);

    if (script->functionTemp[1] == 0) {
        return ApiStatus_DONE2;
    }

    return ApiStatus_BLOCK;
}

typedef struct GlassShardPosition {
    /* 0x00 */ s16 radius;
    /* 0x02 */ s16 angle;
} GlassShardPosition; // size = 0x4

GlassShardPosition N(GlassPositions)[] = {
    { .radius =  5, .angle =   0 },
    { .radius =  5, .angle =  90 },
    { .radius =  5, .angle = 180 },
    { .radius =  5, .angle = 270 },
    { .radius = 10, .angle =  30 },
    { .radius = 10, .angle =  90 },
    { .radius = 10, .angle = 150 },
    { .radius = 10, .angle = 210 },
    { .radius = 10, .angle = 270 },
    { .radius = 10, .angle = 330 },
    { .radius = 20, .angle =   0 },
    { .radius = 20, .angle =  60 },
    { .radius = 20, .angle = 120 },
    { .radius = 20, .angle = 180 },
    { .radius = 20, .angle = 240 },
    { .radius = 20, .angle = 300 },
    { .radius = 30, .angle =  15 },
    { .radius = 30, .angle =  45 },
    { .radius = 30, .angle =  75 },
    { .radius = 30, .angle = 105 },
    { .radius = 30, .angle = 135 },
    { .radius = 30, .angle = 165 },
    { .radius = 30, .angle = 195 },
    { .radius = 30, .angle = 225 },
    { .radius = 30, .angle = 255 },
    { .radius = 30, .angle = 285 },
    { .radius = 30, .angle = 315 },
    { .radius = 30, .angle = 345 },
};

API_CALLABLE(N(ShatterWindow)) {
    EffectInstance* effect;
    f32 radius;
    f32 x, y;
    s32 type;
    f32 posX;
    f32 posY;
    s32 i;

    for (i = 0; i < ARRAY_COUNT(N(GlassPositions)); i++) {
        type = (i & 1) + 2;
        radius = N(GlassPositions)[i].radius;
        x = (radius * cos_deg(N(GlassPositions)[i].angle)) * 1.3;
        y = (radius * sin_deg(N(GlassPositions)[i].angle)) * 1.5;
        posX = x + 600.0f;
        posY = y + 100.0f;
        x *= 0.1;
        y *= 0.1;
        x += cos_deg(radius * 15.0f) * 5.0f;
        effect = fx_ice_shard(type, posX, posY, -150.0f, ((i & 3) * 0.4) + 1.0, ((i & 3) * 4) + 30);
        effect->data.iceShard->animFrame = 0;
        effect->data.iceShard->animRate = (rand_int(10) * 0.2) + 0.1;
        effect->data.iceShard->rot = 35 * i;
        effect->data.iceShard->angularVel = rand_int(10) - 5;
        effect->data.iceShard->vel.x = x;
        effect->data.iceShard->vel.y = y;
        effect->data.iceShard->vel.z = 2.0f;
        effect->data.iceShard->gravAccel = -0.1f;
    }
    return ApiStatus_DONE2;
}

Vec3f N(FlyThroughWindowPath)[] = {
    {  450.0,   150.0, -300.0 },
    {  550.0,   105.0, -180.0 },
    {  650.0,    60.0,  -75.0 },
    {  850.0,    90.0,    0.0 },
};

Vec3f N(PlayerThrownPath)[] = {
    {  660.0,     0.0,   15.0 },
    {  634.0,    60.0,  -83.0 },
    {  608.0,    80.0, -150.0 },
    {  582.0,    60.0, -233.0 },
    {  556.0,     0.0, -315.0 },
    {  530.0,   -60.0, -398.0 },
};

EvtScript N(EVS_FocusCam_Everyone) = {
    Call(UseSettingsFrom, CAM_DEFAULT, 720, 0, 0)
    Call(SetPanTarget, CAM_DEFAULT, 720, 0, 0)
    Call(SetCamSpeed, CAM_DEFAULT, LVar0)
    Call(PanToTarget, CAM_DEFAULT, 0, true)
    Call(WaitForCam, CAM_DEFAULT, Float(1.0))
    Return
    End
};

EvtScript N(EVS_FocusCam_OnPeach) = {
    Call(GetNpcPos, NPC_Peach, LVar0, LVar1, LVar2)
    Call(UseSettingsFrom, CAM_DEFAULT, LVar0, LVar1, LVar2)
    Call(SetPanTarget, CAM_DEFAULT, LVar0, LVar1, LVar2)
    Call(SetCamDistance, CAM_DEFAULT, Float(300.0))
    Call(SetCamSpeed, CAM_DEFAULT, Float(90.0))
    Call(PanToTarget, CAM_DEFAULT, 0, true)
    Call(WaitForCam, CAM_DEFAULT, Float(1.0))
    Return
    End
};

EvtScript N(EVS_FocusCam_OnBowser) = {
    Call(GetNpcPos, NPC_Bowser_Body, LVar1, LVar2, LVar3)
    Call(UseSettingsFrom, CAM_DEFAULT, LVar1, LVar2, LVar3)
    Call(SetPanTarget, CAM_DEFAULT, LVar1, LVar2, LVar3)
    Call(SetCamDistance, CAM_DEFAULT, Float(300.0))
    Call(SetCamPitch, CAM_DEFAULT, Float(17.0), Float(-11.0))
    Call(SetCamSpeed, CAM_DEFAULT, LVar0)
    Call(PanToTarget, CAM_DEFAULT, 0, true)
    Call(WaitForCam, CAM_DEFAULT, Float(1.0))
    Return
    End
};
// End added stuff

EvtScript N(EVS_Scene_MeetingPeach) = {
    Loop(0)
        Wait(1)
        Call(GetPlayerPos, LVar0, LVar1, LVar2)
        IfGe(LVar0, 525)
            BreakLoop
        EndIf
    EndLoop
    Call(DisablePlayerInput, true)
    Call(FadeOutMusic, 0, 666)
    Wait(20 * DT)
    Call(SetMusic, 0, SONG_PRISONER_PEACH_THEME, 0, VOL_LEVEL_FULL)
    Call(PlayerFaceNpc, NPC_Peach, false)
    Call(SetNpcAnimation, NPC_Peach, ANIM_Peach1_Walk)
    Call(SetNpcSpeed, NPC_Peach, Float(2.0 / DT))
    Call(NpcMoveTo, NPC_Peach, 625, 0, 0)
    Call(SetNpcAnimation, NPC_Peach, ANIM_Peach1_Idle)
    Wait(10 * DT)
    Call(SetNpcAnimation, NPC_Peach, ANIM_Peach2_RaiseArms)
    Wait(10 * DT)
    Call(SetNpcAnimation, NPC_Peach, ANIM_Peach2_TalkIdle)
    Call(SpeakToPlayer, NPC_Peach, ANIM_Peach2_Talk, ANIM_Peach2_TalkIdle, 0, MSG_Intro_004B)
    Call(SetNpcAnimation, NPC_Peach, ANIM_Peach2_LowerArms)
    Wait(10 * DT)
    Call(SetNpcAnimation, NPC_Peach, ANIM_Peach1_Idle)
    Thread
        Call(SetCamProperties, CAM_DEFAULT, Float(5.0 / DT), 600, 0, -12, Float(325.0), Float(17.0), Float(-9.0))
    EndThread
    Call(PlayerMoveTo, 575, 0, 20 * DT)
    Wait(10 * DT)
    Call(SetNpcAnimation, NPC_Peach, ANIM_Peach2_RaiseArms)
    Wait(10 * DT)
    Call(SetNpcAnimation, NPC_Peach, ANIM_Peach2_TalkIdle)
    Call(SpeakToPlayer, NPC_Peach, ANIM_Peach2_Talk, ANIM_Peach2_TalkIdle, 0, MSG_Intro_004C)
    Call(SetPlayerAnimation, ANIM_Mario1_NodYes)
    Wait(20 * DT)
    Call(SetPlayerAnimation, ANIM_Mario1_Idle)
    Wait(10 * DT)
    Call(SpeakToPlayer, NPC_Peach, ANIM_Peach2_Talk, ANIM_Peach2_TalkIdle, 0, MSG_Intro_004D)
    Call(SetNpcAnimation, NPC_Peach, ANIM_Peach2_LowerArms)
    Wait(10 * DT)
    Call(SetNpcAnimation, NPC_Peach, ANIM_Peach1_Idle)
    Wait(10 * DT)
    Call(FadeOutMusic, 0, 1000)
    // Kirby: Splicing here
    /*
    Thread
        Wait(10 * DT)
        Call(PlayerMoveTo, 590, 0, 10 * DT)
        Wait(5 * DT)
        Call(PlaySoundAtPlayer, SOUND_EMOTE_IDEA, SOUND_SPACE_DEFAULT)
        Call(ShowEmote, 0, EMOTE_EXCLAMATION, -45, 20, EMOTER_PLAYER, 0, 0, 0, 0)
    EndThread
    Thread
        Call(InterpNpcYaw, NPC_Peach, 90, 7 * DT)
        Call(SetNpcAnimation, NPC_Peach, ANIM_Peach1_Walk)
        Call(NpcMoveTo, NPC_Peach, 640, 0, 10 * DT)
        Call(SetNpcAnimation, NPC_Peach, ANIM_Peach1_Idle)
        Call(PlaySoundAtNpc, NPC_Peach, SOUND_EMOTE_IDEA, SOUND_SPACE_DEFAULT)
        Call(ShowEmote, NPC_Peach, EMOTE_EXCLAMATION, -45, 20, EMOTER_NPC, 0, 0, 0, 0)
        Wait(20 * DT)
        Call(SetNpcAnimation, NPC_Peach, ANIM_Peach2_LookAround)
        Wait(60 * DT)
        Call(InterpNpcYaw, NPC_Peach, 270, 7 * DT)
    EndThread
    Wait(17 * DT)
    Thread
        Call(PlaySound, SOUND_LOOP_KKJ_RUMBLING)
        Call(ShakeCam, CAM_DEFAULT, 0, 200 * DT, Float(1.0))
    EndThread
    Wait(60 * DT)
    Call(GotoMap, Ref("osr_04"), osr_04_ENTRY_0)
    Wait(100 * DT)
    Call(DisablePlayerInput, false)
    Return
    End
};
*/
/*
EvtScript N(EVS_Scene_BowserAttacks) = {
    Call(SetMusic, 0, SONG_BOWSER_ATTACKS, 0, VOL_LEVEL_FULL)
    Call(DisablePlayerInput, true)
    Call(SetNpcPos, NPC_Peach, 680, 0, -15)
    Call(UseSettingsFrom, CAM_DEFAULT, 600, 0, -15)
    Call(SetPanTarget, CAM_DEFAULT, 600, 0, -15)
    Call(SetCamDistance, CAM_DEFAULT, Float(300.0))
    Call(SetCamPitch, CAM_DEFAULT, Float(17.0), Float(-10.0))
    Call(SetCamSpeed, CAM_DEFAULT, Float(90.0))
    Call(PanToTarget, CAM_DEFAULT, 0, true)
    Thread
        Call(PlaySound, SOUND_LOOP_KKJ_RUMBLING)
#if VERSION_PAL
        Call(ShakeCam, CAM_DEFAULT, 0, 60 * DT, Float(0.5))
#else
        Call(ShakeCam, CAM_DEFAULT, 0, 30, Float(1.0))
        Call(ShakeCam, CAM_DEFAULT, 0, 30, Float(0.5))
#endif
        Call(PlaySound, SOUND_LRAW_KKJ_RUMBLING | SOUND_ID_TRIGGER_CHANGE_SOUND)
        Call(StopTrackingSoundPos, SOUND_LRAW_KKJ_RUMBLING)
    EndThread
    Thread
        Call(SetPlayerJumpscale, 0)
        Loop(2)
            Call(InterpPlayerYaw, 270, 0)
            Call(PlayerJump1, 550, 0, 0, 20 * DT)
            Call(InterpPlayerYaw, 90, 0)
            Call(PlayerJump1, 630, 0, 0, 20 * DT)
        EndLoop
        Call(PlayerMoveTo, 550, 0, 30 * DT)
        Call(PlayerMoveTo, 575, 0, 30 * DT)
        Call(InterpPlayerYaw, 90, 7 * DT)
    EndThread
    Thread
        Loop(2)
            Call(SetPlayerAnimation, ANIM_MarioW1_CarryAboveRun)
            Wait(15 * DT)
            Call(SetPlayerAnimation, ANIM_Mario1_Run)
            Wait(15 * DT)
        EndLoop
    EndThread
    Thread
        Call(SetNpcAnimation, NPC_Peach, ANIM_Peach1_RunLiftDress)
        Loop(2)
            Call(NpcMoveTo, NPC_Peach, 600, -2, 20 * DT)
            Call(NpcMoveTo, NPC_Peach, 680, -2, 20 * DT)
        EndLoop
        Call(SetNpcAnimation, NPC_Peach, ANIM_Peach1_Walk)
        Call(NpcMoveTo, NPC_Peach, 600, -2, 30 * DT)
        Call(NpcMoveTo, NPC_Peach, 625, -2, 30 * DT)
        Call(SetNpcAnimation, NPC_Peach, ANIM_Peach1_Idle)
        Call(InterpNpcYaw, NPC_Peach, 270, 7 * DT)
    EndThread
    Wait(160 * DT)
    Call(SetNpcAnimation, NPC_Peach, ANIM_Peach2_RaiseArms)
    */
    // Kirby: I think I have to splice it to here(?)
    Wait(10 * DT)
    Call(SetNpcAnimation, NPC_Peach, ANIM_Peach2_TalkIdle)
    Call(SpeakToPlayer, NPC_Peach, ANIM_Peach2_Talk,ANIM_Peach2_TalkIdle, 0, MSG_Intro_004E)
    Call(SetNpcAnimation, NPC_Peach, ANIM_Peach2_LowerArms)
    Wait(10 * DT)
    Call(SetNpcAnimation, NPC_Peach, ANIM_Peach1_Idle)
    Wait(10 * DT)
    Call(SetNpcAnimation, NPC_Peach, SPRITE_ID_BACK_FACING | ANIM_Peach1_Idle)
    Wait(20 * DT)
    Call(GetNpcPos, NPC_Peach, LVar0, LVar1, LVar2)
    Call(NpcJump0, NPC_Peach, LVar0, LVar1, LVar2, 10)
    Wait(10 * DT)
    Call(InterpPlayerYaw, 0, 7 * DT)
    Call(UseSettingsFrom, CAM_DEFAULT, 600, 0, -15)
    Call(SetPanTarget, CAM_DEFAULT, 600, 0, -15)
    Call(SetCamSpeed, CAM_DEFAULT, Float(5.0 * DT))
    Call(PanToTarget, CAM_DEFAULT, 0, true)
    Call(WaitForCam, CAM_DEFAULT, Float(1.0))
    Call(SpeakToPlayer, NPC_Peach, SPRITE_ID_BACK_FACING | ANIM_Peach1_Idle, SPRITE_ID_BACK_FACING | ANIM_Peach1_Idle, 5, MSG_Intro_004F)
    Wait(30 * DT)
    Thread
        Call(SetNpcAnimation, NPC_Bowser_Body, ANIM_WorldBowser_ClownCarIdle)
        Call(SetNpcAnimation, NPC_Bowser_Prop, ANIM_WorldBowser_ClownCarPropeller)
        Call(SetNpcYaw, NPC_Bowser_Body, 90)
        Call(SetNpcYaw, NPC_Bowser_Prop, 90)
        Exec(N(EVS_UpdatePropellerSounds_Intro))
        Call(LoadPath, 50 * DT, Ref(N(FlyThroughWindowPath)), ARRAY_COUNT(N(FlyThroughWindowPath)), EASING_LINEAR)
        Loop(0)
            Call(GetNextPathPos)
            Call(SetNpcPos, NPC_Bowser_Body, LVar1, LVar2, LVar3)
            Call(SetNpcPos, NPC_Bowser_Prop, LVar1, LVar2, LVar3)
            Wait(1)
            IfEq(LVar0, 0)
                BreakLoop
            EndIf
        EndLoop
        Call(SetNpcAnimation, NPC_Bowser_Body, ANIM_WorldBowser_Idle)
        Call(SetNpcPos, NPC_Bowser_Body, 820, 20, 0)
        Call(SetNpcYaw, NPC_Bowser_Body, 270)
    EndThread
    Thread
        Call(SetNpcAnimation, NPC_Kammy, ANIM_WorldKammy_FlySlow)
        Call(SetNpcYaw, NPC_Kammy, 90)
        Wait(10 * DT)
        Call(LoadPath, 50 * DT, Ref(N(FlyThroughWindowPath)), ARRAY_COUNT(N(FlyThroughWindowPath)), EASING_LINEAR)
        Loop(0)
            Call(GetNextPathPos)
            Call(SetNpcPos, NPC_Kammy, LVar1, LVar2, LVar3)
            Wait(1)
            IfEq(LVar0, 0)
                BreakLoop
            EndIf
        EndLoop
        Call(SetNpcAnimation, NPC_Kammy, ANIM_WorldKammy_Idle)
        Call(SetNpcPos, NPC_Kammy, 920, 0, 0)
        Call(SetNpcYaw, NPC_Kammy, 270)
    EndThread
    Thread
        Wait(20 * DT)
        Call(N(ScreenWhiteFlashOn))
        // Kirby: I have to disable these for now since I don't have the models in this map
        // I'll keep the call to disable g133 though cause that will make the window disappear
        Call(EnableGroup, MODEL_g133, false)
        //Call(EnableGroup, MODEL_g147, false)
        Call(EnableGroup, MODEL_g152, true)
        //Call(EnableGroup, MODEL_g154, true)
        Call(PlaySoundAt, SOUND_GLASS_SHATTER, SOUND_SPACE_DEFAULT, 600, 60, -150)
        PlayEffect(EFFECT_SPARKLES, 0, 600, 60, -150, 120)
        Call(N(ShatterWindow))
        Wait(1)
        Call(N(ScreenWhiteFlashOff))
    EndThread
    Thread
        Wait(20 * DT)
        Call(ShakeCam, CAM_DEFAULT, 0, 10 * DT, Float(2.0))
    EndThread
    // Adding a thread to change peach to horror at the same time as glass shattering
    Thread
        Wait(25 * DT)
        Call(SetNpcAnimation, NPC_Peach, ANIM_Peach2_GaspStill)
    EndThread
    Loop(70 * DT)
        Wait(1)
        Call(PlayerFaceNpc, NPC_Bowser_Body, false)
        Call(NpcFaceNpc, NPC_Peach, NPC_Bowser_Body, 0)
    EndLoop
    //Call(SetNpcAnimation, NPC_Peach, ANIM_Peach1_Idle)
    Wait(30 * DT)
    Call(SetNpcPos, NPC_Bowser_Prop, NPC_DISPOSE_LOCATION)
    Call(SetMusic, 0, SONG_BOWSER_THEME, 0, VOL_LEVEL_FULL)
    Thread
        Wait(5)
        Call(SetNpcJumpscale, NPC_Bowser_Body, Float(0.0))
        Call(NpcJump0, NPC_Bowser_Body, 810, 0, 0, 5)
        Call(PlaySoundAtNpc, NPC_Bowser_Body, SOUND_LARGE_NPC_IMPACT, SOUND_SPACE_DEFAULT)
        Call(SetNpcAnimation, NPC_Bowser_Body, ANIM_WorldBowser_PostJump)
        Call(ShakeCam, CAM_DEFAULT, 0, 20 * DT, Float(2.0))
        Call(SetNpcAnimation, NPC_Bowser_Body, ANIM_WorldBowser_Idle)
    EndThread
    Call(SetPanTarget, CAM_DEFAULT, 740, 0, 0)
    Call(SetCamSpeed, CAM_DEFAULT, Float(2.0 / DT))
    Call(WaitForCam, CAM_DEFAULT, Float(1.0))
    Wait(30 * DT)
    Thread
        Call(SetNpcAnimation, NPC_Kammy, ANIM_WorldKammy_Walk)
        Call(NpcMoveTo, NPC_Kammy, 880, 0, 20 * DT)
        Call(SetNpcAnimation, NPC_Kammy, ANIM_WorldKammy_Idle)
    EndThread
    Call(SpeakToNpc, NPC_Bowser_Body, ANIM_WorldBowser_Talk, ANIM_WorldBowser_Idle, 0, NPC_Peach, MSG_Intro_0050)
    Call(SetPanTarget, CAM_DEFAULT, 625, 0, -15)
    Call(SetCamDistance, CAM_DEFAULT, Float(300.0))
    Call(SetCamPitch, CAM_DEFAULT, Float(17.0), Float(-11.0))
    Call(SetCamSpeed, CAM_DEFAULT, Float(90.0))
    Call(PanToTarget, CAM_DEFAULT, 0, true)
    // Kirby: Changed to armscrossed instead of shout
    Call(SetNpcAnimation, NPC_Peach, ANIM_Peach2_ArmsCrossedIdle)
    Call(SpeakToNpc, NPC_Peach, ANIM_Peach2_ArmsCrossedTalk, ANIM_Peach2_ArmsCrossedIdle, 0, NPC_Bowser_Body, MSG_Intro_0051)
    Set(LVar0, Float(1.0))
    ExecWait(N(EVS_FocusCam_OnBowser))
    Call(SpeakToNpc, NPC_Bowser_Body, ANIM_WorldBowser_TalkEyesClosed, ANIM_WorldBowser_Idle, 0, NPC_Peach, MSG_Intro_0052)
    Thread
        Call(SetNpcAnimation, NPC_Bowser_Body, ANIM_WorldBowser_Walk)
        Call(NpcMoveTo, NPC_Bowser_Body, 770, 0, 30 * DT)
        Call(SetNpcAnimation, NPC_Bowser_Body, ANIM_WorldBowser_Idle)
    EndThread
    Thread
        Call(SetNpcAnimation, NPC_Kammy, ANIM_WorldKammy_Walk)
        Call(NpcMoveTo, NPC_Kammy, 840, 0, 30 * DT)
        Call(SetNpcAnimation, NPC_Kammy, ANIM_WorldKammy_Idle)
    EndThread
    Wait(5 * DT)
    Set(LVar0, Float(90.0))
    ExecWait(N(EVS_FocusCam_Everyone))
    Wait(5 * DT)
    Call(PlayerMoveTo, 660, 0, 15 * DT)
    Call(SetPlayerAnimation, ANIM_Mario1_BeforeJump)
    Wait(15 * DT)
    Call(SpeakToPlayer, NPC_Bowser_Body, ANIM_WorldBowser_Talk, ANIM_WorldBowser_Idle, 0, MSG_Intro_0053)
    Call(SpeakToPlayer, NPC_Peach, ANIM_Peach2_ArmsCrossedTalk, ANIM_Peach2_ArmsCrossedIdle, 5, MSG_Intro_0054)
    Wait(10 * DT)
    Call(SetNpcAnimation, NPC_Bowser_Body, ANIM_WorldBowser_RearUpLaugh)
    Call(SpeakToPlayer, NPC_Bowser_Body, ANIM_WorldBowser_RearUpLaugh, ANIM_WorldBowser_RearUpLaugh, 0, MSG_Intro_0055)
    Set(LVar0, Float(90.0))
    ExecWait(N(EVS_FocusCam_OnBowser))
    Call(SpeakToPlayer, NPC_Bowser_Body, ANIM_WorldBowser_RearUpTalk, ANIM_WorldBowser_RearUpTalk, 0, MSG_Intro_0056)
    Set(LVar0, Float(5.0))
    ExecWait(N(EVS_FocusCam_Everyone))
    Call(SetNpcVar, NPC_Bowser_Body, 0, 0)
    Wait(5)
    Call(PlayerMoveTo, 695, 0, 10)
    Call(SetPlayerJumpscale, Float(0.75))
    Call(PlayerJump, 720, 80, 0, 20 * DT)
    Call(DisablePlayerPhysics, true)
    Loop(0)
        Wait(1)
        Call(GetNpcVar, NPC_Bowser_Body, 0, LVar0)
        IfEq(LVar0, 1)
            BreakLoop
        EndIf
    EndLoop
    Call(SetPlayerPos, 660, 0, 15)
    // Kirby: Flipping Mario
    Call(InterpPlayerYaw, 270, 0)
    Wait(1)
    // Kirby: Changing this from falldown to dustoff
    Call(SetPlayerAnimation, ANIM_Mario1_DustOff)
    Call(SetNpcAnimation, NPC_Peach, ANIM_Peach2_GaspStill)
    // Kirby: And changing this to defeated
    Call(SetNpcAnimation, NPC_Bowser_Body, ANIM_WorldBowser_DefeatedIdle)
    ExecWait(N(EVS_FocusCam_OnPeach))
    Wait(10 * DT)
    Call(SpeakToPlayer, NPC_Peach, ANIM_Peach2_GaspStill, ANIM_Peach2_GaspStill, 5, MSG_Intro_005D)
    // Kirby: Flipping Mario
    Call(InterpPlayerYaw, 90, 0)
    Call(SetNpcAnimation, NPC_Peach, ANIM_Peach1_Idle)
    Wait(10 * DT)
    Call(UseSettingsFrom, CAM_DEFAULT, 790, 0, 0)
    Call(SetPanTarget, CAM_DEFAULT, 790, 0, 0)
    Call(SetCamDistance, CAM_DEFAULT, Float(300.0))
    Call(SetCamPitch, CAM_DEFAULT, Float(17.0), Float(-11.0))
    Call(SetCamSpeed, CAM_DEFAULT, Float(90.0))
    Call(PanToTarget, CAM_DEFAULT, 0, true)
    Wait(10 * DT)
    // Kirby: Change this to defeated instead of rearup laugh
    Call(SpeakToPlayer, NPC_Bowser_Body, ANIM_WorldBowser_DefeatedTalk, ANIM_WorldBowser_DefeatedIdle, 5, MSG_Intro_005E)
    Wait(10 * DT)
    Call(SpeakToPlayer, NPC_Kammy, ANIM_WorldKammy_Talk, ANIM_WorldKammy_Idle, 5, MSG_Intro_005F)
    Call(SetNpcAnimation, NPC_Bowser_Body, ANIM_WorldBowser_Idle)
    Call(InterpNpcYaw, NPC_Bowser_Body, 90, 5)
    Wait(10 * DT)
    // Kirby: Removing brandish animations
    //Call(SetNpcAnimation, NPC_Bowser_Body, ANIM_WorldBowser_Brandish)
    //Wait(20 * DT)
    //Call(SetNpcAnimation, NPC_Bowser_Body, ANIM_WorldBowser_BrandishIdle)
    Call(SpeakToNpc, NPC_Bowser_Body, ANIM_WorldBowser_Talk, ANIM_WorldBowser_Idle, 0, NPC_Kammy, MSG_Intro_0060)
    Wait(10 * DT)
    Set(LVar0, Float(90.0))
    ExecWait(N(EVS_FocusCam_Everyone))
    Call(SetNpcAnimation, NPC_Bowser_Body, ANIM_WorldBowser_Idle)
    Call(InterpNpcYaw, NPC_Bowser_Body, 270, 10)
    Wait(20 * DT)
    // Kirby: Now I gotta add a bunch of my own scripting here
    Call(SpeakToPlayer, NPC_Bowser_Body, ANIM_WorldBowser_AngryTalk, ANIM_WorldBowser_Idle, 5, MSG_Intro_0065)
    Call(SetMusic, 0, SONG_BOWSER_THEME, 0, VOL_LEVEL_FULL)
    Call(ContinueSpeech, NPC_Bowser_Body, ANIM_WorldBowser_TalkEyesClosed, ANIM_WorldBowser_Idle, 5, MSG_Intro_006A)
    Call(SetNpcAnimation, NPC_Bowser_Body, ANIM_WorldBowser_RearUpLaugh)
    Call(SetNpcAnimation, NPC_Peach, ANIM_Peach2_GaspStill)
    Call(SetPlayerAnimation, ANIM_MarioW2_PanicStill) //Not sure if this is the best one to use here.
    Call(ContinueSpeech, NPC_Bowser_Body, ANIM_WorldBowser_TalkEyesClosed, ANIM_WorldBowser_Idle, 5, MSG_Intro_0066)
    Call(AddStarPoints, 30)
    Call(N(OpenStatusBar))
    Wait(30 * DT)
    Loop(30)
        Call(AddStarPoints, -1)
        Call(PlaySound, SOUND_LRAW_QUIZ_TICKING)
        Wait(10 * DT)
    EndLoop
    Call(StopSound, SOUND_LRAW_QUIZ_TICKING)
    Call(N(CloseStatusBar))
    Wait(10 * DT)
    Call(SpeakToPlayer, NPC_Bowser_Body, ANIM_WorldBowser_RearUpLaugh, ANIM_WorldBowser_RearUpLaugh, 5, MSG_Intro_0067)
    // Kirby: And after this bowser leaves
    Call(SpeakToPlayer, NPC_Bowser_Body, ANIM_WorldBowser_Talk, ANIM_WorldBowser_Idle, 5, MSG_Intro_0061)
    //Call(SetNpcAnimation, NPC_Bowser_Body, ANIM_WorldBowser_Brandish)
    Wait(10 * DT)
    Call(FadeOutMusic, 0, 500)
    // Kirby: Cutting everything after this basically
    /*
    Call(PlaySoundAtNpc, NPC_Bowser_Body, SOUND_BOWSER_STAR_ROD_USE, SOUND_SPACE_DEFAULT)
    PlayEffect(EFFECT_GATHER_ENERGY_PINK, 0, 780, 95, 27, 1, 30)
    Wait(40 * DT)
    Thread
        Call(ShakeCam, CAM_DEFAULT, 0, 10 * DT, Float(4.0))
    EndThread
    Call(PlaySoundAtPlayer, SOUND_BOWSER_LIGHTNING, SOUND_SPACE_DEFAULT)
    Call(GetPlayerPos, LVar0, LVar1, LVar2)
    Add(LVar2, 5)
    PlayEffect(EFFECT_LIGHTNING, 4, LVar0, LVar1, LVar2, 0, 0)
    Thread
        Call(PlaySoundAtPlayer, SOUND_TOSS_PLAYER, SOUND_SPACE_DEFAULT)
        Call(SetPlayerAnimation, ANIM_Mario1_Hurt)
        Call(LoadPath, 30 * DT, Ref(N(PlayerThrownPath)), ARRAY_COUNT(N(PlayerThrownPath)), EASING_LINEAR)
        Set(LVar4, 0)
        Loop(0)
            Call(GetNextPathPos)
            Call(SetPlayerPos, LVar1, LVar2, LVar3)
            Add(LVar4, 45)
            Call(InterpPlayerYaw, LVar4, 0)
            Wait(1)
            IfEq(LVar0, 0)
                BreakLoop
            EndIf
        EndLoop
    EndThread
    Wait(30 * DT)
    Call(GotoMap, Ref("osr_03"), osr_03_ENTRY_2)
    */
    // Kirby: Add I change the gotomap call here
    Call(GotoMap, Ref("kkj_01"), kkj_01_ENTRY_1)
    Wait(100 * DT)
    Call(DisablePlayerPhysics, false)
    Call(DisablePlayerInput, false)
    Return
    End
};

EvtScript N(EVS_TexPan_Clouds) = {
    Call(SetTexPanner, MODEL_o718, TEX_PANNER_2)
    Thread
        TEX_PAN_PARAMS_ID(TEX_PANNER_2)
        TEX_PAN_PARAMS_STEP( -200,    0,    0,    0)
        TEX_PAN_PARAMS_FREQ(    1,    0,    0,    0)
        TEX_PAN_PARAMS_INIT(    0,    0,    0,    0)
        Exec(EVS_UpdateTexturePan)
    EndThread
    Return
    End
};

EvtScript N(EVS_MoveClouds) = {
    Set(LVar0, 0)
    Loop(0)
        Call(TranslateGroup, MODEL_g159, 0, LVar0, 0)
        Add(LVar0, -2)
        Wait(1)
    EndLoop
    Return
    End
};

EvtScript N(EVS_Scene_Ascending) = {
    Call(DisablePlayerInput, true)
    Call(SetNpcPos, NPC_Peach, 680, 0, -15)
    Call(EnableGroup, MODEL_g156, true)
    Exec(N(EVS_TexPan_Clouds))
    Exec(N(EVS_MoveClouds))
    Call(UseSettingsFrom, CAM_DEFAULT, 600, 0, -15)
    Call(SetPanTarget, CAM_DEFAULT, 600, 0, -15)
    Call(SetCamSpeed, CAM_DEFAULT, Float(90.0))
    Call(PanToTarget, CAM_DEFAULT, 0, true)
    Thread
        Call(SetPlayerJumpscale, 0)
        Loop(0)
            Call(InterpPlayerYaw, 270, 0)
            Call(PlayerJump1, 550, 0, 0, 20 * DT)
            Call(InterpPlayerYaw, 90, 0)
            Call(PlayerJump1, 630, 0, 0, 20 * DT)
        EndLoop
    EndThread
    Thread
        Loop(0)
            Call(SetPlayerAnimation, ANIM_Mario1_Run)
            Wait(15 * DT)
            Call(SetPlayerAnimation, ANIM_MarioW1_CarryAboveRun)
            Wait(15 * DT)
        EndLoop
    EndThread
    Thread
        Call(SetNpcAnimation, NPC_Peach, ANIM_Peach1_RunLiftDress)
        Loop(0)
            Call(NpcMoveTo, NPC_Peach, 600, 0, 20 * DT)
            Call(NpcMoveTo, NPC_Peach, 680, 0, 20 * DT)
        EndLoop
    EndThread
    Thread
        Call(PlaySound, SOUND_LOOP_KKJ_RUMBLING)
        Call(ShakeCam, CAM_DEFAULT, 0, 200, Float(1.0))
    EndThread
    Wait(100 * DT)
    Call(GotoMap, Ref("osr_03"), osr_03_ENTRY_1)
    Wait(100 * DT)
    Call(DisablePlayerInput, false)
    Return
    End
};
