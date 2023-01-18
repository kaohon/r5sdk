//======== Copyright (c) Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//===========================================================================//

#ifndef PLAYER_H
#define PLAYER_H

#include "public/baseentity.h"
#include "mathlib/mathlib.h"
#include <public/studio.h>
#include <public/playerstate.h>

#include "game/shared/animation.h"
#include "game/shared/takedamageinfo.h"
#include "playerlocaldata.h"
#include <mathlib/vector4d.h>


// TODO: Move to separate header file!!
struct __declspec(align(8)) PredictedAnimEventData
{
	char gap_0[8];
	float m_predictedAnimEventTimes[8];
	int m_predictedAnimEventIndices[8];
	int m_predictedAnimEventCount;
	int m_predictedAnimEventTarget;
	int m_predictedAnimEventSequence;
	int m_predictedAnimEventModel;
	float m_predictedAnimEventsReadyToFireTime;
};

struct WeaponDropInfo
{
	Vector3D weaponPosition;
	char prevDropFrameCounter;
	char dropFrameCounter;
	char gap_e[2];
	Vector3D weaponAngles;
	float weaponPositionTime;
};

/* 1410 */
struct WeaponInventory
{
	char gap_0[8];
	int weapons[9];
	int offhandWeapons[6];
	int activeWeapons[3];
};
struct CTether
{
	char gap_0[8];
	Vector3D pos;
	float health;
	float nextSoundTime;
	float creationTime;
	int scriptID;
};

struct ThirdPersonViewData
{
	char gap_0[8];
	Vector3D m_thirdPersonEntViewOffset;
	bool m_thirdPersonEntShouldViewAnglesFollowThirdPersonEnt;
	bool m_thirdPersonEntPitchIsFreelook;
	bool m_thirdPersonEntYawIsFreelook;
	bool m_thirdPersonEntUseFixedDist;
	bool m_thirdPersonEntFixedClientOnly;
	bool m_thirdPersonEntPushedInByGeo;
	bool m_thirdPersonEntDrawViewmodel;
	char gap_1b[1];
	float m_thirdPersonEntBlendTotalDuration;
	float m_thirdPersonEntBlendEaseInDuration;
	float m_thirdPersonEntBlendEaseOutDuration;
	float m_thirdPersonEntFixedPitch;
	float m_thirdPersonEntFixedYaw;
	float m_thirdPersonEntFixedDist;
	float m_thirdPersonEntFixedHeight;
	float m_thirdPersonEntMinYaw;
	float m_thirdPersonEntMaxYaw;
	float m_thirdPersonEntMinPitch;
	float m_thirdPersonEntMaxPitch;
	float m_thirdPersonEntSpringToCenterRate;
	float m_thirdPersonEntLookaheadLowerEntSpeed;
	float m_thirdPersonEntLookaheadUpperEntSpeed;
	float m_thirdPersonEntLookaheadMaxAngle;
	float m_thirdPersonEntLookaheadLerpAheadRate;
	float m_thirdPersonEntLookaheadLerpToCenterRate;
};
struct GrappleData
{
	char gap_0[8];
	Vector3D m_grappleVel;
	Vector3D m_grapplePoints[4];
	int m_grapplePointCount;
	bool m_grappleAttached;
	bool m_grapplePulling;
	bool m_grappleSwinging;
	bool m_grappleRetracting;
	bool m_grappleForcedRetracting;
	bool m_grappleGracePeriodFinished;
	char gap_4e[2];
	float m_grappleUsedPower;
	float m_grappleActivateTime;
	float m_grapplePullTime;
	float m_grappleAttachTime;
	float m_grappleDetachTime;
	int m_grappleMeleeTarget;
	int m_grappleAutoAimTarget;
	bool m_grappleHasGoodVelocity;
	char gap_6d[3];
	float m_grappleLastGoodVelocityTime;
	float m_grappleSwingDetachLowSpeed;
	float m_grappleSwingHoldTime;
	char gap_7c[4];
};
struct PlayerZiplineData
{
	char gap_0[8];
	bool m_ziplineReenableWeapons;
	char gap_9[3];
	float m_mountingZiplineDuration;
	float m_mountingZiplineAlpha;
	float m_ziplineStartTime;
	float m_ziplineEndTime;
	Vector3D m_mountingZiplineSourcePosition;
	Vector3D m_mountingZiplineSourceVelocity;
	Vector3D m_mountingZiplineTargetPosition;
	Vector3D m_ziplineUsePosition;
	float m_slidingZiplineAlpha;
	Vector3D m_lastMoveDir2D;
	bool m_ziplineReverse;
};
struct Player_ViewOffsetEntityData
{
	char gap_0[8];
	int viewOffsetEntityHandle;
	float lerpInDuration;
	float lerpOutDuration;
	bool stabilizePlayerEyeAngles;
};
struct CurrentData_Player
{
	char gap_0[8];
	float m_flHullHeight;
	float m_traversalAnimProgress;
	float m_sprintTiltFrac;
	Vector3D m_angEyeAngles;
	int m_ammoPoolCount[8];
};
struct CurrentData_LocalPlayer
{
	char gap_0[8];
	Vector3D m_viewConeAngleMin;
	Vector3D m_viewConeAngleMax;
	Vector3D m_stepSmoothingOffset;
	Vector3D m_vecPunchBase_Angle;
	Vector3D m_vecPunchBase_AngleVel;
	Vector3D m_vecPunchWeapon_Angle;
	Vector3D m_vecPunchWeapon_AngleVel;
	int m_pushedFixedPointOffset[3];
	int m_pushedFixedPointOffsetReplayCompensated[3];
	Quaternion m_localGravityRotation;
};
struct __declspec(align(1)) MatchMetrics
{
	bool hotDropped;
	bool relinquished;
	char gap[2];
	int allPings;
	int locationPings;
	int enemyPings;
	int shotsFired;
	int shotsHit;
	int level;
	int matches;
	int wins;
	int winsWithFriends;
	int timesJumpmaster;
	int winsAsJumpmaster;
	int damage;
	int damageTaken;
	int kills;
	int teamworkKills;
	int timesRevivedAlly;
	int characterPickOrder;
	char characterName[32];
	char rankedPeriodName[32];
	int rankedScore;
};
struct PlayerMelee_PlayerData
{
	char gap_0[8];
	int meleeAttackParity;
	bool attackActive;
	bool attackRecoveryShouldBeQuick;
	bool isSprintAttack;
	char gap_f[1];
	float attackStartTime;
	int attackHitEntity;
	float attackHitEntityTime;
	float attackLastHitNonWorldEntity;
	int scriptedState;
	bool pendingMeleePress;
	char gap_25[3];
	Vector3D lungeBoost;
};

struct CPlayerShared // !TODO: MOVE INTO SHARED!!!
{
	char gap_0[8];
	int m_nPlayerCond;
	bool m_bLoadoutUnavailable;
	char gap_d[3];
	float m_nextWaterCheck;
	float m_flCondExpireTimeLeft[2];
	char gap_1c[4];
	void* m_pOuter;
	float m_flNextCritUpdate;
	float m_flTauntRemoveTime;
	CTakeDamageInfo m_damageInfo;
};

struct StatusEffectTimedData
{
	char gap_0[8];
	int seComboVars;
	float seTimeEnd;
	float seEaseOut;
	float sePausedTimeRemaining;
};

struct StatusEffectEndlessData
{
	char gap_0[8];
	int seComboVars;
	char gap_c[4];
};

struct PushHistoryEntry
{
	float time;
	Vector3D pushed;
};

struct PredictableServerEvent
{
	int type;
	float deadlineTime;
	int fullSizeOfUnion[4];
};

struct CKnockBack
{
	char gap_0[8];
	Vector3D velocity;
	float beginTime;
	float endTime;
};

struct SpeedChangeHistoryEntry
{
	float time;
	float maxSpeedScaled;
	float maxSpeedChange;
};


class CPlayer : public CBaseEntity // !TODO: proper inheritance; separate into classes according to RTTI.
{
	char gap_b04[12];
	bool m_markedForServerInterpolation;
	bool m_animRemoveFromServerInterpolationNextFrame;
	char gap_b12[2];
	float m_flGroundSpeed;
	float m_flLastEventCheck;
	int m_nForceBone;
	Vector3D m_vecForce;
	int m_nSkin;
	__int16 m_skinMod;
	char gap_b32[2];
	int m_nBody;
	int m_camoIndex;
	int m_decalIndex;
	int m_nHitboxSet;
	float m_flModelScale;
	int m_nRagdollImpactFXTableId;
	float m_flSkyScaleStartTime;
	float m_flSkyScaleEndTime;
	float m_flSkyScaleStartValue;
	float m_flSkyScaleEndValue;
	char gap_b5c[4];
	char m_SequenceTransitioner[368];
	float m_flIKGroundContactTime;
	float m_flIKGroundMinHeight;
	float m_flIKGroundMaxHeight;
	float m_flEstIkFloor;
	float m_flEstIkOffset;
	char gap_ce4[4];
	char m_pIk[8];
	int m_ikPrevSequence;
	bool m_bSequenceFinished;
	bool m_bSequenceLooped;
	bool m_bSequenceLoops;
	bool m_continueAnimatingAfterRagdoll;
	float m_lockedAnimDeltaYaw;
	bool m_threadedBoneSetup;
	bool m_settingUpBones;
	char gap_cfe[2];
	float m_flDissolveStartTime;
	int m_baseAnimatingActivity;
	float m_flPoseParameter[12];
	bool m_poseParameterOverTimeActive;
	char gap_d39[3];
	float m_poseParameterGoalValue[12];
	float m_poseParameterEndTime[12];
	float m_lastTimeSetPoseParametersSameAs;
	bool m_bClientSideAnimation;
	bool m_bReallyClientSideAnimation;
	char gap_da2[2];
	int m_nNewSequenceParity;
	int m_nResetEventsParity;
	char gap_dac[4];
	__int64 m_boneCacheHandle;
	__int16 m_fBoneCacheFlags;
	char gap_dba[2];
	int m_animNetworkFlags;
	bool m_animActive;
	bool m_animCollisionEnabled;
	bool m_animPlantingEnabled;
	bool m_animInitialCorrection;
	bool m_animWaitingForCleanup;
	char gap_dc5[3];
	int m_animWaitingForCleanupTime;
	char gap_dcc[4];
	__int64 m_recordedAnim;
	int m_recordedAnimIndex;
	int m_recordedAnimCachedFrameIndex;
	float m_recordedAnimPlaybackRate;
	float m_recordedAnimPlaybackTime;
	matrix3x4_t m_recordedAnimTransform;
	int m_recordedAnimPlaybackEnt;
	float m_recordedAnimBlendTime;
	Vector3D m_recordedAnimBlendOffset;
	Vector3D m_recordedAnimBlendAngles;
	AnimRelativeData m_animRelativeData;
	int m_syncingWithEntity;
	char gap_ec4[4];
	PredictedAnimEventData m_predictedAnimEventData;
	int m_animRefEntityAttachmentIndex;
	int m_fireAttachmentSmartAmmoIndex;
	int m_fireAttachmentChestFocusIndex;
	int m_fireAttachmentModelIndex;
	char m_keyHitboxes[160];
	CStudioHdr* m_pStudioHdr;
	int m_animSequence;
	float m_animCycle;
	int m_animModelIndex;
	float m_animStartTime;
	float m_animStartCycle;
	float m_animPlaybackRate;
	bool m_animFrozen;
	char gap_ff9[7];
	__int64 m_createdProp[8];
	int m_numCreatedProps;
	char gap_1044[4];
	__int64 m_currentFramePropEvents[16];
	int m_numCurrentFramePropEvents;
	char gap_10cc[4];
	__int64 m_activeScriptAnimWindows[8];
	int m_numActiveScriptAnimWindows;
	char gap_1114[4];
	__int64 m_currentFrameWindowEvents[16];
	int m_numCurrentFrameWindowEvents;
	char gap_119c[4];
	__int64 m_AnimSyncScriptProps[8];
	int m_numAnimSyncScriptProps;
	char gap_11E4[12];
	int m_maxOverlays;
	char gap_11f4[4];
	CAnimationLayer m_AnimOverlay;
	char gap_1228[384];
	int m_AnimOverlayCount;
	char m_overlayEventParity[9];
	bool m_animOverlayIsActive[9];
	char gap_13be[2];
	int m_animOverlayModelIndex[9];
	int m_animOverlaySequence[9];
	float m_animOverlayCycle[9];
	float m_animOverlayStartTime[9];
	float m_animOverlayStartCycle[9];
	float m_animOverlayPlaybackRate[9];
	float m_animOverlayWeight[9];
	int m_animOverlayOrder[9];
	float m_animOverlayAnimTime[9];
	float m_animOverlayFadeInDuration[9];
	float m_animOverlayFadeOutDuration[9];
	bool m_localAnimOverlayIsActive[4];
	int m_localAnimOverlayModelIndex[4];
	int m_localAnimOverlaySequence[4];
	float m_localAnimOverlayStartTime[4];
	float m_localAnimOverlayWeight[4];
	float m_localAnimOverlayFadeInDuration[4];
	float m_localAnimOverlayFadeOutDuration[4];
	bool m_bPreventWeaponPickup;
	char gap_15b1[3];
	float m_phaseShiftTimeStart;
	float m_phaseShiftTimeEnd;
	float m_flNextAttack;
	float m_lastFiredTime;
	int m_lastFiredWeapon;
	float m_raiseFromMeleeEndTime;
	float m_nextFlamethrowerStatusEffectUpdateTime;
	int m_lastFlamethrowerStatusEffectInflictor;
	int m_lastFlamethrowerStatusEffectAttacker;
	int m_sharedEnergyCount;
	int m_sharedEnergyTotal;
	int m_sharedEnergyLockoutThreshold;
	float m_lastSharedEnergyRegenTime;
	float m_sharedEnergyRegenRate;
	float m_sharedEnergyRegenDelay;
	float m_lastSharedEnergyTakeTime;
	int m_eHull;
	float m_fieldOfViewCos;
	Vector3D m_HackedGunPos;
	float m_impactEnergyScale;
	WeaponDropInfo m_weaponDropInfo;
	float m_physicsMass;
	float m_flDamageAccumulator;
	int m_prevHealth;
	float m_healthChangeRate;
	float m_healthChangeAmount;
	float m_healthChangeStartTime;
	float m_lastHealthChangeTime;
	int m_lastHitGroup;
	Vector3D m_lastDamageDir;
	Vector3D m_lastDamageForce;
	int m_deathPackage;
	Vector3D m_deathDirection2DInverse;
	int m_CurrentWeaponProficiency;
	float m_flEnemyAccurcyMultiplier;
	int m_npcPriorityOverride;
	int m_healthPerSegment;
	char gap_1684[4];
	WeaponInventory m_inventory;
	char m_selectedWeapons[2];
	char gap_16da[2];
	int m_latestPrimaryWeapons[2];
	int m_latestPrimaryWeaponsIndexZeroOrOne[2];
	char m_latestNonOffhandWeapons[2];
	char m_selectedOffhands[3];
	char m_selectedOffhandsPendingHybridAction[3];
	char m_lastCycleSlot;
	char gap_16f5[3];
	int m_weaponGettingSwitchedOut[2];
	bool m_showActiveWeapon3p[2];
	char gap_1702[2];
	int m_weaponPermission;
	float m_weaponDelayEnableTime;
	char m_weaponDisabledFlags;
	bool m_hudInfo_visibilityTestAlwaysPasses;
	bool m_weaponDisabledInScript;
	char gap_170f[1];
	int m_removeWeaponOnSelectSwitch;
	int m_latestMeleeWeapon;
	bool m_doOffhandAnim;
	bool m_wantInventoryChangedScriptCall;
	bool m_doInventoryChangedScriptCall;
	char gap_171b[1];
	float m_cloakReactEndTime;
	CTether m_tethers[2];
	char gap_1768[8];
	int m_titanSoul;
	Vector3D m_lastFootstepDamagePos;
	bool m_lastFoostepDamageOnGround;
	char gap_1781[3];
	int m_muzzleAttachment[2];
	int m_weaponHandAttachment;
	int m_weaponAltHandAttachment;
	int m_prevNearestNode;
	int m_nearestNode;
	float m_nearestNodeCheckTime;
	Vector3D m_nearestNodeCheckPos;
	int m_nearestPolyRef[5];
	Vector3D m_nearestPolyCheckPos[5];
	int m_contextAction;
	char m_weaponAnimEvents[16648];
	char m_targetInfoIconName[64];
	bool m_titanStepDamage;
	char gap_5949[3];
	int m_latest3pWeaponGettingEquipped[2];
	char gap_5954[12];
	char m_szNetname[256];
	bool m_zoomViewdriftDebounceEnabled;
	bool m_bZooming;
	char gap_5a62[2];
	float m_zoomToggleOnStartTime;
	float m_zoomBaseFrac;
	float m_zoomBaseTime;
	float m_zoomFullStartTime;
	int m_physicsSolidMask;
	int m_rightHandAttachment;
	int m_leftHandAttachment;
	int m_headAttachment;
	int m_chestAttachment;
	int m_StuckLast;
	char gap_5a8c[4];
	CPlayerLocalData m_Local;
	int m_hColorCorrectionCtrl;
	char gap_5dbc[4];
	char m_hTriggerSoundscapeList[32];
	CPlayerState pl;
	int m_ammoPoolCapacity;
	bool m_hasBadReputation;
	char m_communityName[64];
	char m_communityClanTag[16];
	char m_factionName[16];
	char m_hardwareIcon[16];
	bool m_happyHourActive;
	char gap_5ee6[2];
	__int64 m_platformUserId;
	char m_hardware;
	char gap_5ef1[7];
	__int64 m_classModsActive;
	__int64 m_classModsActiveOld;
	char m_classSettings[32];
	int m_classSettingsScriptRefCount;
	char m_modInventory[128];
	__int16 m_consumableInventory[32];
	char gap_5fec[4];
	__int64 m_passives[2];
	int m_bleedoutState;
	float m_damageComboLatestUpdateTime;
	int m_damageComboStartHealth;
	float m_healResources_nextTickTime[10];
	float m_healResources_healRatePerSecond[10];
	__int16 m_healResources_remainingHeals[10];
	__int16 m_healResources_totalHeals[10];
	__int16 m_healResources_generation[10];
	__int16 m_healResources_refId[10];
	int m_healResources_healSource[10];
	int m_healResources_healthTarget;
	int m_communityId;
	int m_nButtons;
	int m_afButtonPressed;
	int m_afButtonReleased;
	int m_afButtonLast;
	int m_afButtonDisabled;
	int m_afButtonForced;
	float m_forwardMove;
	float m_sideMove;
	float m_prevForwardMove;
	float m_prevSideMove;
	bool m_bLagCompensation;
	bool m_bPredictWeapons;
	bool m_bPredictionEnabled;
	char gap_6107[1];
	int m_lagRecordValidationCounter;
	bool m_wantedToMatchmake;
	bool m_playerWeaponSwitchOnEquipIsDisabled;
	char gap_610e[2];
	int m_skyCamera;
	Vector3D m_originalPlayerOrigin;
	int m_playerVehicle;
	int m_entitySyncingWithMe;
	int m_playerFlags;
	bool m_hasMic;
	bool m_inPartyChat;
	char gap_612e[2];
	float m_playerMoveSpeedScale;
	float m_cachedMoveScale;
	int m_gestureSequences[8];
	float m_gestureStartTimes[8];
	float m_gestureBlendInDuration[8];
	float m_gestureBlendOutDuration[8];
	float m_gestureFadeOutStartTime[8];
	float m_gestureFadeOutDuration[8];
	int m_gestureAutoKillBitfield;
	float m_flFallVelocityForLandAnims;
	bool m_bDropEnabled;
	bool m_bDuckEnabled;
	char gap_6202[2];
	int m_iRespawnFrames;
	int m_afPhysicsFlags;
	float m_flTimeLastTouchedWall;
	float m_timeJetpackHeightActivateCheckPassed;
	float m_flTimeLastTouchedGround;
	float m_flTimeLastJumped;
	float m_flTimeLastLanded;
	float m_flLastLandFromHeight;
	Vector3D m_upDirWhenLastTouchedGround;
	bool m_bHasJumpedSinceTouchedGround;
	bool m_bDoMultiJumpPenalty;
	bool m_bWasGrounded;
	char gap_6233[1];
	float m_usePressedTime;
	float m_lastUseTime;
	Vector3D m_lastFakeFloorPos;
	Vector3D m_accumDamageImpulseVel;
	float m_accumDamageImpulseTime;
	float m_damageImpulseNoDecelEndTime;
	int m_hDmgEntity;
	float m_DmgTake;
	int m_bitsDamageType;
	int m_bitsHUDDamage;
	float m_xpRate;
	int m_gamepadIntegrityFailCount_RSPN;
	float m_lastKillTime;
	float m_flDeathTime;
	float m_flDeathAnimTime;
	int m_iObserverMode;
	int m_iObserverLastMode;
	int m_hObserverTarget;
	Vector3D m_observerModeStaticPosition;
	Vector3D m_observerModeStaticAngles;
	float m_observerModeStaticFOVOverride;
	bool m_isValidChaseObserverTarget;
	bool m_observerModeToggleEnabled;
	char gap_62aa[2];
	int m_vphysicsCollisionState;
	bool m_bHasVPhysicsCollision;
	char gap_62b1[3];
	float m_fNextSuicideTime;
	int m_iSuicideCustomKillFlags;
	int m_preNoClipPhysicsFlags;
	char m_Commands[8];
	void* m_pPhysicsController;
	void* m_pShadowStand;
	void* m_pShadowCrouch;
	Vector3D m_oldOrigin;
	Vector3D m_vecSmoothedVelocity;
	bool m_bTouchedPhysObject;
	bool m_bPhysicsWasFrozen;
	bool m_fInitHUD;
	bool m_fGameHUDInitialized;
	int m_iUpdateTime;
	int m_iConnected;
	int m_iPlayerLocked;
	int m_gameStats[12];
	int m_firstPersonProxy;
	int m_predictedFirstPersonProxy;
	int m_grappleHook;
	int m_petTitan;
	int m_petTitanMode;
	int m_xp;
	int m_generation;
	int m_rank;
	int m_serverForceIncreasePlayerListGenerationParity;
	bool m_isPlayingRanked;
	char gap_635d[3];
	float m_skill_mu;
	int m_hardpointEntity;
	float m_nextTitanRespawnAvailable;
	bool m_activeViewmodelModifiers[35];
	bool m_activeViewmodelModifiersChanged;
	int m_hViewModels[3];
	char m_LastCmd[476];
	void* m_pCurrentCommand;
	float m_flStepSoundTime;
	int m_hThirdPersonEnt;
	bool m_thirdPersonShoulderView;
	char gap_6589[7];
	ThirdPersonViewData m_thirdPerson;
	int m_duckState;
	int m_leanState;
	bool m_doingHalfDuck;
	bool m_canStand;
	char gap_65fa[2];
	Vector3D m_StandHullMin;
	Vector3D m_StandHullMax;
	Vector3D m_DuckHullMin;
	Vector3D m_DuckHullMax;
	Vector3D m_upDir;
	Vector3D m_upDirPredicted;
	Vector3D m_lastWallRunStartPos;
	float m_wallRunStartTime;
	float m_wallRunClearTime;
	int m_wallRunCount;
	bool m_wallRunWeak;
	bool m_shouldBeOneHanded;
	char gap_665e[2];
	float m_oneHandFraction;
	float m_animLastUpdateTime;
	float m_animAimPitch;
	float m_animAimYaw;
	float m_wallRunPushAwayTime;
	float m_wallrunFrictionScale;
	float m_groundFrictionScale;
	float m_wallrunRetryTime;
	Vector3D m_wallrunRetryPos;
	Vector3D m_wallrunRetryNormal;
	bool m_wallClimbSetUp;
	bool m_wallHanging;
	char gap_669a[2];
	float m_wallHangStartTime;
	float m_wallHangTime;
	int m_traversalType;
	int m_traversalState;
	Vector3D m_traversalBegin;
	Vector3D m_traversalMid;
	Vector3D m_traversalEnd;
	float m_traversalMidFrac;
	Vector3D m_traversalForwardDir;
	Vector3D m_traversalRefPos;
	float m_traversalProgress;
	float m_traversalStartTime;
	float m_traversalHandAppearTime;
	float m_traversalReleaseTime;
	float m_traversalBlendOutStartTime;
	Vector3D m_traversalBlendOutStartOffset;
	float m_traversalYawDelta;
	int m_traversalYawPoseParameter;
	float m_wallDangleJumpOffTime;
	bool m_wallDangleMayHangHere;
	bool m_wallDangleForceFallOff;
	bool m_wallDangleLastPushedForward;
	char gap_671b[1];
	int m_wallDangleDisableWeapon;
	float m_wallDangleClimbProgressFloor;
	float m_prevMoveYaw;
	float m_sprintTiltVel;
	int m_sprintTiltPoseParameter;
	int m_sprintFracPoseParameter;
	char gap_6734[4];
	GrappleData m_grapple;
	bool m_grappleActive;
	bool m_grappleNeedWindowCheck;
	char gap_67ba[2];
	int m_grappleNextWindowHint;
	int m_landingType;
	bool m_slowMoEnabled;
	bool m_sliding;
	bool m_slideLongJumpAllowed;
	char gap_67c7[1];
	float m_lastSlideTime;
	float m_lastSlideBoost;
	bool m_ziplineAllowed;
	char gap_67d1[3];
	int m_activeZipline;
	int m_lastZipline;
	float m_lastZiplineDetachTime;
	bool m_ziplineValid3pWeaponLayerAnim;
	char gap_67e1[3];
	int m_ziplineState;
	PlayerZiplineData m_zipline;
	Vector3D m_ziplineViewOffsetPosition;
	Vector3D m_ziplineViewOffsetVelocity;
	int m_ziplineGrenadeEntity;
	int m_ziplineGrenadeBeginStationEntity;
	int m_ziplineGrenadeBeginStationAttachmentIndex;
	char gap_686c[4];
	Player_ViewOffsetEntityData m_viewOffsetEntity;
	Player_AnimViewEntityData m_animViewEntity;
	bool m_highSpeedViewmodelAnims;
	char gap_6911[3];
	int m_playAnimationType;
	bool m_detachGrappleOnPlayAnimationEnd;
	char gap_6919[3];
	int m_playAnimationNext[2];
	int m_playAnimationEntityBlocker;
	bool m_playAnimationEntityBlockerCanTeleport;
	bool m_playAnimationEntityBlockerDucking;
	char gap_692a[2];
	float m_lastRunPlayerAnimationTime;
	int m_gravityGrenadeStatusEffect;
	float m_onSlopeTime;
	Vector3D m_lastWallNormal;
	bool m_dodgingInAir;
	bool m_dodging;
	char gap_6946[2];
	float m_lastDodgeTime;
	float m_airSpeed;
	float m_airAcceleration;
	bool m_iSpawnParity;
	bool m_boosting;
	bool m_activateBoost;
	bool m_repeatedBoost;
	float m_boostMeter;
	bool m_jetpack;
	bool m_activateJetpack;
	bool m_jetpackAfterburner;
	bool m_gliding;
	float m_glideMeter;
	float m_glideRechargeDelayAccumulator;
	bool m_hovering;
	bool m_isPerformingBoostAction;
	char gap_696a[2];
	float m_lastJumpHeight;
	char m_touchingSlipTriggers[72];
	Vector3D m_slipAirRestrictDirection;
	float m_slipAirRestrictTime;
	float m_lastTimeDamagedByOtherPlayer;
	float m_lastTimeDamagedByNPC;
	float m_lastTimeDidDamageToOtherPlayer;
	float m_lastTimeDidDamageToNPC;
	char m_predictedTriggers[72];
	int m_lastJumpPadTouched;
	int m_launchCount;
	char m_lastBodySound1p[32];
	char m_lastBodySound3p[32];
	char m_lastFinishSound1p[32];
	char m_lastFinishSound3p[32];
	char m_primedSound1p[32];
	char m_primedSound3p[32];
	int m_replayImportantSounds_networkTableSoundID[4];
	float m_replayImportantSounds_beginTime[4];
	CurrentData_Player m_currentFramePlayer;
	CurrentData_LocalPlayer m_currentFrameLocalPlayer;
	int m_nImpulse;
	int m_nNumCrateHudHints;
	bool m_needStuckCheck;
	char gap_6bd9[3];
	float m_totalFrameTime;
	float m_joinFrameTime;
	int m_lastUCmdSimulationTicks;
	float m_lastUCmdSimulationRemainderTime;
	float m_totalExtraClientCmdTimeAttempted;
	bool m_atLeastOneCommandRunThisServerFrame;
	bool m_bGamePaused;
	bool m_bPlayerUnderwater;
	bool m_wasPhaseShiftedForTriggers;
	int m_hPlayerViewEntity;
	bool m_bShouldDrawPlayerWhileUsingViewEntity;
	char gap_6bf9[3];
	int m_hConstraintEntity;
	Vector3D m_vecConstraintCenter;
	float m_flConstraintRadius;
	float m_flConstraintWidth;
	float m_flConstraintSpeedFactor;
	bool m_bConstraintPastRadius;
	char gap_6c19[7];
	__int64 m_twitchRewardBits;
	bool m_twitchRewardsUnfulfilled;
	char gap_6c29[3];
	float m_lastActiveTime;
	float m_flLaggedMovementValue;
	float m_lastMoveInputTime;
	int m_ignoreEntityForMovementUntilNotTouching;
	Vector3D m_vNewVPhysicsPosition;
	Vector3D m_vNewVPhysicsVelocity;
	Vector3D m_vNewVPhysicsWishVel;
	Vector3D m_vecPreviouslyPredictedOrigin;
	Vector3D m_prevAbsOrigin;
	Vector3D m_preMoveThinkAbsOrigin;
	int m_nBodyPitchPoseParam;
	char m_szNetworkIDString[64];
	__int64 m_squad;
	__int64 m_SquadName;
	char m_gameMovementUtil[56];
	float m_flTimeAllSuitDevicesOff;
	bool m_bIsStickySprinting;
	char gap_6d15[3];
	float m_fStickySprintMinTime;
	bool m_bPlayedSprintStartEffects;
	char gap_6d1d[3];
	int m_autoSprintForced;
	bool m_fIsSprinting;
	bool m_fIsWalking;
	char gap_6d26[2];
	float m_sprintStartedTime;
	float m_sprintStartedFrac;
	float m_sprintEndedTime;
	float m_sprintEndedFrac;
	float m_stickySprintStartTime;
	bool m_bSinglePlayerGameEnding;
	char gap_6d3d[3];
	int m_ubEFNoInterpParity;
	bool m_viewConeActive;
	bool m_viewConeParented;
	char gap_6d46[2];
	int m_viewConeParity;
	int m_lastViewConeParityTick;
	float m_viewConeLerpTime;
	bool m_viewConeSpecificEnabled;
	char gap_6d55[3];
	Vector3D m_viewConeSpecific;
	Vector3D m_viewConeRelativeAngleMin;
	Vector3D m_viewConeRelativeAngleMax;
	int m_hReservedSpawnPoint;
	int m_hLastSpawnPoint;
	MatchMetrics m_matchMetrics;
	bool m_autoKickDisabled;
	char gap_6e11[3];
	int m_stuckCharacter;
	char m_title[32];
	bool sentHUDScriptChecksum;
	bool m_bIsFullyConnected;
	char gap_6e3a[2];
	CTakeDamageInfo m_lastDeathInfo;
	char gap_6ebc[4];
	PlayerMelee_PlayerData m_melee;
	int m_lungeTargetEntity;
	bool m_isLungingToPosition;
	char gap_6efd[3];
	Vector3D m_lungeTargetPosition;
	Vector3D m_lungeStartPositionOffset;
	Vector3D m_lungeStartPositionOffset_notLagCompensated;
	Vector3D m_lungeEndPositionOffset;
	float m_lungeStartTime;
	float m_lungeEndTime;
	bool m_lungeCanFly;
	bool m_lungeLockPitch;
	char gap_6f3a[2];
	float m_lungeStartPitch;
	float m_lungeSmoothTime;
	float m_lungeMaxTime;
	float m_lungeMaxEndSpeed;
	bool m_useCredit;
	char gap_6f4d[3];
	__int64 m_smartAmmoNextTarget;
	__int64 m_smartAmmoPrevTarget;
	float m_smartAmmoHighestLocksOnMeFractionValues[4];
	int m_smartAmmoHighestLocksOnMeEntities[4];
	float m_smartAmmoPreviousHighestLockOnMeFractionValue;
	float m_smartAmmoPendingHighestLocksOnMeFractionValues[4];
	char gap_6f94[4];
	void* m_smartAmmoPendingHighestLocksOnMeEntities[4];
	bool m_smartAmmoRemoveFromTargetList;
	char gap_6fb9[3];
	int m_delayedFlinchEvents;
	__int64 m_delayedFlinchEventCount;
	char m_extraWeaponModNames[512];
	char m_extraWeaponModNamesArray[64];
	__int64 m_extraWeaponModNameCount;
	char m_pPlayerAISquad[8];
	float m_flAreaCaptureScoreAccumulator;
	float m_flCapPointScoreRate;
	float m_flConnectionTime;
	float m_fullyConnectedTime;
	float m_connectedForDurationCallback_duration;
	float m_flLastForcedChangeTeamTime;
	int m_iBalanceScore;
	char gap_7234[4];
	void* m_PlayerAnimState;
	Vector3D m_vWorldSpaceCenterHolder;
	Vector3D m_vPrevGroundNormal;
	int m_threadedPostProcessJob;
	char gap_725c[4];
	CPlayerShared m_Shared;
	StatusEffectTimedData m_statusEffectsTimedPlayerNV[10];
	StatusEffectEndlessData m_statusEffectsEndlessPlayerNV[10];
	int m_pilotClassIndex;
	char m_pilotClassActivityModifier[2];
	bool m_pilotClassActivityModifierInitialized;
	char gap_74a7[1];
	int m_latestCommandRun;
	int m_latestCommandQueued;
	float m_lastSimulateTime;
	char gap_74b4[4];
	char m_nearbyPushers[480];
	int m_nearbyPusherCount;
	PushHistoryEntry m_pushHistory[16];
	int m_pushHistoryEntryIndex;
	float m_baseVelocityLastServerTime;
	Vector3D m_pushedThisFrame;
	Vector3D m_pushedThisSnapshotAccum;
	int m_pusher;
	float m_pusherTouchTime;
	float m_pusherUntouchTime;
	Vector3D m_originRelativeToPusher;
	int m_predictingEnts[4];
	int m_predictingEntCount;
	float m_lastCommandCountWarnTime;
	Vector3D m_pushAwayFromTopAcceleration;
	float m_trackedState[52];
	int m_prevTrackedState;
	Vector3D m_prevTrackedStatePos;
	char m_recordingAnim[8];
	char m_animRecordFile[8];
	char m_animRecordButtons[4];
	float m_minimapTargetZoomScale;
	float m_minimapTargetLerpTime;
	bool m_sendMovementCallbacks;
	bool m_sendInputCallbacks;
	char gap_78f6[2];
	PredictableServerEvent m_predictableServerEvents[16];
	int m_predictableServerEventCount;
	int m_predictableServerEventAcked;
	int m_playerScriptNetDataGlobal;
	int m_playerScriptNetDataExclusive;
	int m_armorType;
	int m_helmetType;
	bool m_controllerModeActive;
	bool m_freefallNoCollisionFoundAroundPlayer;
	char gap_7a92[2];
	Vector3D m_freefallCapsuleStart;
	Vector3D m_freefallCapsuleEnd;
	Vector3D m_freefallCapsuleHalfHeight;
	float m_freefallCapsuleRadius;
	Vector3D m_freefallCapsuleCorners[4];
	char gap_7aec[4];
	Vector4D m_freefallCapsuleSweepPlane;
	Vector4D m_freefallCapsuleEdgePlanes[4];
	float m_freefallJobSearchRange;
	float m_skydiveForwardPoseValueVelocity;
	float m_skydiveForwardPoseValueTarget;
	float m_skydiveForwardPoseValueCurrent;
	float m_skydiveSidePoseValueVelocity;
	float m_skydiveSidePoseValueTarget;
	float m_skydiveSidePoseValueCurrent;
	float m_skydiveYawVelocity;
	int m_freefallState;
	float m_freefallStartTime;
	float m_freefallEndTime;
	float m_freefallAnticipateStartTime;
	float m_freefallAnticipateEndTime;
	float m_freefallDistanceToLand;
	float m_skydiveDiveAngle;
	bool m_skydiveIsDiving;
	char gap_7b7d[3];
	float m_skydiveSpeed;
	float m_skydiveStrafeAngle;
	bool m_skydiveFreelookEnabled;
	char gap_7b89[3];
	Vector3D m_skydiveFreelookLockedAngle;
	float m_skydivePlayerPitch;
	float m_skydivePlayerYaw;
	bool m_skydiveFollowing;
	char gap_7ba1[3];
	Vector3D m_skydiveUnfollowVelocity;
	bool m_skydiveIsNearLeviathan;
	char gap_7bb1[3];
	Vector3D m_skydiveLeviathanHitPosition;
	Vector3D m_skydiveLeviathanHitNormal;
	Vector3D m_skydiveSlipVelocity;
	bool m_forceWeaponReload;
	char gap_7bd9[7];
	CKnockBack m_playerKnockBacks[4];
	char gap_7c50[16];
	SpeedChangeHistoryEntry m_speedChangeHistory[32];
	int m_speedChangeHistoryIndex;
	bool m_hasValidTraceToKnockBackEyePosition;
	char m_bodyModelOverride[256];
	char gap_7ee5[3];
	int m_armsModelIndex;
};

#endif // PLAYER_H