#ifndef _IN_CHR_CHRAICOMMANDS_H
#define _IN_CHR_CHRAICOMMANDS_H
#include <ultra64.h>
#include "types.h"

/*0x0000*/ bool aiGoToNext(void);
/*0x0001*/ bool aiGoToFirst(void);
/*0x0002*/ bool aiLabel(void);
/*0x0003*/ bool aiYield(void);
/*0x0004*/ bool aiEndList(void);
/*0x0005*/ bool aiSetList(void);
/*0x0006*/ bool aiSetReturnList(void);
/*0x0007*/ bool ai0007(void);
/*0x0008*/ bool aiReturn(void);
/*0x0009*/ bool aiStop(void);
/*0x000a*/ bool aiKneel(void);
/*0x000b*/ bool ai000b(void);
/*0x000c*/ bool aiIfIdle(void);
/*0x000d*/ bool ai000d(void);
/*0x000e*/ bool ai000e(void);
/*0x000f*/ bool aiStepSideways(void);
/*0x0010*/ bool aiHopSideways(void);
/*0x0011*/ bool aiRunSideways(void);
/*0x0012*/ bool aiWalkAndFire(void);
/*0x0013*/ bool aiRunAndFire(void);
/*0x0014*/ bool aiRollAndFire(void);
/*0x0015*/ bool aiAimAndFire1(void);
/*0x0016*/ bool aiKneelAndFire(void);
/*0x0017*/ bool aiAimAndFire2(void);
/*0x0018*/ bool aiFaceEntity(void);
/*0x0019*/ bool ai0019(void);
/*0x001a*/ bool ai001a(void);
/*0x001b*/ bool aiConsiderGrenadeThrow(void);
/*0x001c*/ bool ai001c(void);
/*0x001d*/ bool aiJogToPad(void);
/*0x001e*/ bool aiGoToPadPreset(void);
/*0x001f*/ bool aiWalkToPad(void);
/*0x0020*/ bool aiRunToPad(void);
/*0x0021*/ bool aiSetPath(void);
/*0x0022*/ bool aiStartPath(void);
/*0x0023*/ bool aiIfPathStarted(void);
/*0x0024*/ bool aiSurrender(void);
/*0x0025*/ bool aiFadeOut(void);
/*0x0026*/ bool aiRemoveChr(void);
/*0x0027*/ bool ai0027(void);
/*0x0028*/ bool aiActivateAlarm(void);
/*0x0029*/ bool aiDeactivateAlarm(void);
/*0x002a*/ bool ai002a(void);
/*0x002b*/ bool ai002b(void);
/*0x002c*/ bool ai002c(void);
/*0x002d*/ bool ai002d(void);
/*0x002e*/ bool ai002e(void);
/*0x002f*/ bool aiJogToChr(void);
/*0x0030*/ bool aiWalkToChr(void);
/*0x0031*/ bool aiRunToChr(void);
/*0x0032*/ bool aiIfStopped(void);
/*0x0033*/ bool aiIfChrDying(void);
/*0x0034*/ bool aiIfChrDeathAnimationFinished(void);
/*0x0035*/ bool aiIfPlayerInSight(void);
/*0x0036*/ bool aiRandom(void);
/*0x0037*/ bool aiIfRandomLessThan(void);
/*0x0038*/ bool aiIfRandomGreaterThan(void);
/*0x0039*/ bool aiIfChrAlarmActivate(void);
/*0x003a*/ bool aiIfAlarmActive(void);
/*0x003b*/ bool aiIfAlarmInactive(void);
/*0x003c*/ bool ai003c(void);
/*0x003d*/ bool ai003d(void);
/*0x003e*/ bool aiIfSawDeath(void);
/*0x003f*/ bool aiIfSeesPlayer(void);
/*0x0040*/ bool ai0040(void);
/*0x0041*/ bool ai0041(void);
/*0x0042*/ bool aiIfInLoadedRoom(void);
/*0x0043*/ bool ai0043(void);
/*0x0044*/ bool aiIfShotAtCloseRange(void);
/*0x0045*/ bool ai0045(void);
/*0x0046*/ bool aiIfNeverBeenOnScreen(void);
/*0x0047*/ bool ai0047(void);
/*0x0048*/ bool ai0048(void);
/*0x0049*/ bool ai0049(void);
/*0x004a*/ bool ai004a(void);
/*0x004b*/ bool ai004b(void);
/*0x004c*/ bool ai004c(void);
/*0x004d*/ bool ai004d(void);
/*0x004e*/ bool ai004e(void);
/*0x004f*/ bool ai004f(void);
/*0x0050*/ bool ai0050(void);
/*0x0051*/ bool ai0051(void);
/*0x0052*/ bool aiIfDistanceToTargetLessThan(void);
/*0x0053*/ bool aiIfDistanceToTargetGreaterThan(void);
/*0x0054*/ bool aiIfChrDistanceToPadLessThan(void);
/*0x0055*/ bool aiIfChrDistanceToPadGreaterThan(void);
/*0x0056*/ bool aiIfDistanceToChrLessThan(void);
/*0x0057*/ bool aiIfDistanceToChrGreaterThan(void);
/*0x0058*/ bool ai0058(void);
/*0x0059*/ bool aiIfDistanceFromTargetToPadLessThan(void);
/*0x005a*/ bool aiIfDistanceFromTargetToPadGreaterThan(void);
/*0x005b*/ bool ai005b(void);
/*0x005c*/ bool aiIfTargetInRoom(void);
/*0x005d*/ bool aiIfChrHasObject(void);
/*0x005e*/ bool aiIfWeaponThrown(void);
/*0x005f*/ bool aiIfWeaponThrownOnObject(void);
/*0x0060*/ bool aiIfChrHasWeaponEquipped(void);
/*0x0061*/ bool aiIfGunUnclaimed(void);
/*0x0062*/ bool aiIfObjectHealthy(void);
/*0x0063*/ bool aiIfChrActivatedObject(void);
/*0x0065*/ bool ai0065(void);
/*0x0066*/ bool aiDestroyObject(void);
/*0x0067*/ bool ai0067(void);
/*0x0068*/ bool aiChrDropItems(void);
/*0x0069*/ bool aiChrDropWeapon(void);
/*0x006a*/ bool aiGiveObjectToChr(void);
/*0x006b*/ bool ai006b(void);
/*0x006c*/ bool aiOpenDoor(void);
/*0x006d*/ bool aiCloseDoor(void);
/*0x006e*/ bool aiIfDoorState(void);
/*0x006f*/ bool aiIfObjectIsDoor(void);
/*0x0070*/ bool aiLockDoor(void);
/*0x0071*/ bool aiUnlockDoor(void);
/*0x0072*/ bool ai0072(void);
/*0x0073*/ bool ai0073(void);
/*0x0074*/ bool ai0074(void);
/*0x0075*/ bool ai0075(void);
/*0x0076*/ bool ai0076(void);
/*0x0077*/ bool aiIfDifficultyLessThan(void);
/*0x0078*/ bool aiIfDifficultyGreaterThan(void);
/*0x0079*/ bool aiIfUptimeLessThan(void);
/*0x007a*/ bool aiIfUptimeGreaterThan(void);
/*0x007b*/ bool aiIfStageIdLessThan(void);
/*0x007c*/ bool aiIfStageIdGreaterThan(void);
/*0x007d*/ bool ai007d(void);
/*0x007e*/ bool ai007e(void);
/*0x007f*/ bool ai007f(void);
/*0x0080*/ bool ai0080(void);
/*0x0081*/ bool ai0081(void);
/*0x0082*/ bool ai0082(void);
/*0x0083*/ bool ai0083(void);
/*0x0084*/ bool aiSetMorale(void);
/*0x0085*/ bool aiAddMorale(void);
/*0x0086*/ bool aiChrAddMorale(void);
/*0x0087*/ bool aiSubtractMorale(void);
/*0x0088*/ bool aiIfMoraleLessThan(void);
/*0x0089*/ bool aiIfMoraleLessThanRandom(void);
/*0x008a*/ bool aiSetAlertness(void);
/*0x008b*/ bool aiAddAlertness(void);
/*0x008c*/ bool aiChrAddAlertness(void);
/*0x008d*/ bool aiSubtractAlertness(void);
/*0x008e*/ bool aiIfAlertness(void);
/*0x008f*/ bool aiIfChrAlertnessLessThan(void);
/*0x0090*/ bool aiIfAlertnessLessThanRandom(void);
/*0x0091*/ bool ai0091(void);
/*0x0092*/ bool aiSetHearDistance(void);
/*0x0093*/ bool aiSetViewDistance(void);
/*0x0094*/ bool aiSetGrenadeProbability(void);
/*0x0095*/ bool aiSetChrNum(void);
/*0x0096*/ bool ai0096(void);
/*0x0097*/ bool ai0097(void);
/*0x0098*/ bool aiSetReactionSpeed(void);
/*0x0099*/ bool aiSetRecoverySpeed(void);
/*0x009a*/ bool aiSetAccuracy(void);
/*0x009b*/ bool aiSetFlag(void);
/*0x009c*/ bool aiUnsetFlag(void);
/*0x009d*/ bool ai009d(void);
/*0x009e*/ bool aiChrSetFlag(void);
/*0x009f*/ bool aiChrUnsetFlag(void);
/*0x00a0*/ bool aiIfChrHasFlag(void);
/*0x00a1*/ bool aiSetStageFlag(void);
/*0x00a2*/ bool aiUnsetStageFlag(void);
/*0x00a3*/ bool aiIfStageFlagEq(void);
/*0x00a4*/ bool aiSetChrflag(void);
/*0x00a5*/ bool aiUnsetChrflag(void);
/*0x00a6*/ bool aiIfHasChrflag(void);
/*0x00a7*/ bool aiChrSetChrflag(void);
/*0x00a8*/ bool aiChrUnsetChrflag(void);
/*0x00a9*/ bool aiIfChrHasChrflag(void);
/*0x00aa*/ bool aiSetObjFlag(void);
/*0x00ab*/ bool aiUnsetObjFlag(void);
/*0x00ac*/ bool aiIfObjHasFlag(void);
/*0x00ad*/ bool aiSetObjFlag2(void);
/*0x00ae*/ bool aiUnsetObjFlag2(void);
/*0x00af*/ bool aiIfObjHasFlag2(void);
/*0x00b0*/ bool aiSetChrPreset(void);
/*0x00b1*/ bool aiSetChrTarget(void);
/*0x00b2*/ bool aiSetPadPreset(void);
/*0x00b3*/ bool aiChrSetPadPreset(void);
/*0x00b4*/ bool aiChrCopyPadPreset(void);
/*0x00b5*/ bool ai00b5(void);
/*0x00b6*/ bool aiRestartTimer(void);
/*0x00b7*/ bool aiResetTimer(void);
/*0x00b8*/ bool aiPauseTimer(void);
/*0x00b9*/ bool aiResumeTimer(void);
/*0x00ba*/ bool aiIfTimerStopped(void);
/*0x00bb*/ bool aiIfTimerGreaterThanRandom(void);
/*0x00bc*/ bool ai00bc(void);
/*0x00bd*/ bool ai00bd(void);
/*0x00be*/ bool aiShowCountdownTimer(void);
/*0x00bf*/ bool aiHideCountdownTimer(void);
/*0x00c0*/ bool aiSetCountdownTimerValue(void);
/*0x00c1*/ bool aiStopCountdownTimer(void);
/*0x00c2*/ bool aiStartCountdownTimer(void);
/*0x00c3*/ bool aiIfCountdownTimerStopped(void);
/*0x00c4*/ bool aiIfCountdownTimerLessThan(void);
/*0x00c5*/ bool aiIfCountdownTimerGreaterThan(void);
/*0x00c6*/ bool ai00c6(void);
/*0x00c7*/ bool ai00c7(void);
/*0x00c8*/ bool ai00c8(void);
/*0x00c9*/ bool ai00c9(void);
/*0x00ca*/ bool ai00ca(void);
/*0x00cb*/ bool aiMessage(void);
/*0x00cc*/ bool aiShowText2(void);
/*0x00cd*/ bool ai00cd(void);
/*0x00ce*/ bool ai00ce(void);
/*0x00cf*/ bool ai00cf(void);
/*0x00d0*/ bool ai00d0(void);
/*0x00d1*/ bool ai00d1(void);
/*0x00d2*/ bool ai00d2(void);
/*0x00d3*/ bool aiAudioMuteChannel(void);
/*0x00d4*/ bool ai00d4(void);
/*0x00d5*/ bool ai00d5(void);
/*0x00d6*/ bool ai00d6(void);
/*0x00d7*/ bool ai00d7(void);
/*0x00d8*/ bool ai00d8(void);
/*0x00d9*/ bool ai00d9(void);
/*0x00da*/ bool aiSetObjImage(void);
/*0x00db*/ bool ai00db(void);
/*0x00dc*/ bool ai00dc(void);
/*0x00dd*/ bool ai00dd(void);
/*0x00de*/ bool ai00de(void);
/*0x00df*/ bool ai00df(void);
/*0x00e0*/ bool aiRevokeControl(void);
/*0x00e1*/ bool aiGrantControl(void);
/*0x00e2*/ bool ai00e2(void);
/*0x00e3*/ bool ai00e3(void);
/*0x00e4*/ bool ai00e4(void);
/*0x00e5*/ bool ai00e5(void);
/*0x00e8*/ bool aiSetDoorClosed(void);
/*0x00e9*/ bool ai00e9(void);
/*0x00ea*/ bool ai00ea(void);
/*0x00eb*/ bool aiIfChrAmmoQuantityLessThan(void);
/*0x00ec*/ bool aiChrDrawWeapon(void);
/*0x00ed*/ bool aiChrDrawWeaponInCutscene(void);
/*0x00ee*/ bool ai00ee(void);
/*0x00ef*/ bool aiIfObjInRoom(void);
/*0x00f0*/ bool ai00f0(void);
/*0x00f1*/ bool ai00f1(void);
/*0x00f2*/ bool ai00f2(void);
/*0x00f3*/ bool aiChrSetInvincible(void);
/*0x00f4*/ bool ai00f4(void);
/*0x00f5*/ bool ai00f5(void);
/*0x00f6*/ bool ai00f6(void);
/*0x00f7*/ bool aiIfAllObjectivesComplete(void);
/*0x00f8*/ bool ai00f8(void);
/*0x00f9*/ bool aiAudioPlayXMusic(void);
/*0x00fa*/ bool aiAudioStopChannel(void);
/*0x00fb*/ bool aiChrExplosions(void);
/*0x00fc*/ bool aiIfKillCountGreaterThan(void);
/*0x00fd*/ bool ai00fd(void);
/*0x00fe*/ bool ai00fe(void);
/*0x00ff*/ bool ai00ff(void);
/*0x0100*/ bool ai0100(void);
/*0x0101*/ bool ai0101(void);
/*0x0102*/ bool ai0102(void);
/*0x0103*/ bool ai0103(void);
/*0x0104*/ bool ai0104(void);
/*0x0105*/ bool ai0105(void);
/*0x0106*/ bool ai0106(void);
/*0x0107*/ bool aiIfPresetsTargetIsNotMyTarget(void);
/*0x0108*/ bool ai0108(void);
/*0x0109*/ bool ai0109(void);
/*0x010a*/ bool ai010a(void);
/*0x010b*/ bool aiChrSetTeam(void);
/*0x010c*/ bool ai010c(void);
/*0x010d*/ bool ai010d(void);
/*0x010e*/ bool ai010e(void);
/*0x010f*/ bool ai010f(void);
/*0x0110*/ bool ai0110(void);
/*0x0111*/ bool ai0111(void);
/*0x0112*/ bool ai0112(void);
/*0x0113*/ bool aiIfInCutscene(void);
/*0x0114*/ bool aiShowChr(void);
/*0x0115*/ bool aiHideChr(void);
/*0x0116*/ bool aiShowObj(void);
/*0x0117*/ bool aiHideObj(void);
/*0x0118*/ bool aiSetObjFlag3(void);
/*0x0119*/ bool aiUnsetObjFlag3(void);
/*0x011a*/ bool aiIfObjHasFlag3(void);
/*0x011b*/ bool aiChrSetHiddenFlag(void);
/*0x011c*/ bool aiChrUnsetHiddenFlag(void);
/*0x011d*/ bool aiIfChrHasHiddenFlag(void);
/*0x011e*/ bool ai011e(void);
/*0x011f*/ bool ai011f(void);
/*0x0120*/ bool ai0120(void);
/*0x0121*/ bool ai0121(void);
/*0x0122*/ bool ai0122(void);
/*0x0123*/ bool ai0123(void);
/*0x0124*/ bool ai0124(void);
/*0x0125*/ bool ai0125(void);
/*0x0126*/ bool aiIfPlayerUsingCmpOrAr34(void);
/*0x0127*/ bool ai0127(void);
/*0x0128*/ bool ai0128(void);
/*0x0129*/ bool ai0129(void);
/*0x012a*/ bool ai012a(void);
/*0x012b*/ bool ai012b(void);
/*0x012c*/ bool ai012c(void);
/*0x012f*/ bool ai012f(void);
/*0x0130*/ bool ai0130(void);
/*0x0131*/ bool ai0131(void);
/*0x0132*/ bool aiSetAction(void);
/*0x0133*/ bool ai0133(void);
/*0x0134*/ bool ai0134(void);
/*0x0135*/ bool aiIfHasOrders(void);
/*0x0136*/ bool ai0136(void);
/*0x0137*/ bool ai0137(void);
/*0x0138*/ bool aiIfChannelIdle(void);
/*0x0139*/ bool ai0139(void);
/*0x013a*/ bool ai013a(void);
/*0x013b*/ bool aiSetSquadron(void);
/*0x013c*/ bool ai013c(void);
/*0x013d*/ bool ai013d(void);
/*0x013e*/ bool ai013e(void);
/*0x013f*/ bool ai013f(void);
/*0x0140*/ bool ai0140(void);
/*0x0141*/ bool aiShuffleInvestigationTerminals(void);
/*0x0142*/ bool ai0142(void);
/*0x0143*/ bool ai0143(void);
/*0x0144*/ bool ai0144(void);
/*0x0145*/ bool aiRebuildTeams(void);
/*0x0146*/ bool aiRebuildSquadrons(void);
/*0x0147*/ bool ai0147(void);
/*0x0148*/ bool aiChrSetListening(void);
/*0x0149*/ bool ai0149(void);
/*0x014a*/ bool ai014a(void);
/*0x014b*/ bool aiIfNotListening(void);
/*0x0152*/ bool ai0152(void);
/*0x0157*/ bool aiSetTintedGlassEnabled(void);
/*0x015b*/ bool aiAudioPlayMusic(void);
/*0x015c*/ bool aiAudioRestartMusic(void);
/*0x0165*/ bool aiIfChrInjured(void);
/*0x0166*/ bool aiIfAction(void);
/*0x0167*/ bool aiHovercopterFireRocket(void);
/*0x0168*/ bool ai0168(void);
/*0x0169*/ bool aiIfNaturalAnim(void);
/*0x016a*/ bool aiIfY(void);
/*0x016b*/ bool ai016b(void);
/*0x016c*/ bool ai016c(void);
/*0x016d*/ bool aiChrAdjustMotionBlur(void);
/*0x016e*/ bool ai016e(void);
/*0x016f*/ bool aiIfChrHasGun(void);
/*0x0170*/ bool ai0170(void);
/*0x0171*/ bool ai0171(void);
/*0x0172*/ bool ai0172(void);
/*0x0173*/ bool aiChrCopyProperties(void);
/*0x0174*/ bool aiIfCutsceneButtonPressed(void);
/*0x0175*/ bool ai0175(void);
/*0x0176*/ bool ai0176(void);
/*0x0177*/ bool ai0177(void);
/*0x0178*/ bool ai0178(void);
/*0x0179*/ bool ai0179(void);
/*0x017a*/ bool ai017a(void);
/*0x017b*/ bool aiIfChrUnloaded(void);
/*0x017c*/ bool ai017c(void);
/*0x017d*/ bool aiAudioSetMusicTrack(void);
/*0x017e*/ bool aiAudioRestartDefaultMusic(void);
/*0x017f*/ bool aiAudioSetSfxTrack(void);
/*0x0180*/ bool aiAudioRestartSfx(void);
/*0x0181*/ bool ai0181(void);
/*0x0182*/ bool aiPunchOrKick(void);
/*0x0183*/ bool ai0183(void);
/*0x0184*/ bool ai0184(void);
/*0x0185*/ bool ai0185(void);
/*0x0186*/ bool aiIfSoundTimer(void);
/*0x0187*/ bool ai0187(void);
/*0x0188*/ bool aiIfLiftStationary(void);
/*0x0189*/ bool ai0189(void);
/*0x018a*/ bool ai018a(void);
/*0x018b*/ bool aiConfigureRain(void);
/*0x018c*/ bool aiChrToggleProp(void);
/*0x018d*/ bool aiActivateLift(void);
/*0x018e*/ bool ai018e(void);
/*0x018f*/ bool ai018f(void);
/*0x0190*/ bool aiEepromSet(void);
/*0x0191*/ bool aiEepromUnset(void);
/*0x0192*/ bool aiIfEepromSet(void);
/*0x0193*/ bool aiIfEepromUnset(void);
/*0x019e*/ bool aiIfObjHealthLessThan(void);
/*0x019f*/ bool aiSetObjHealth(void);
/*0x01a0*/ bool aiSetChrSpecialDeathAnimation(void);
/*0x01a1*/ bool aiSetRoomToSearch(void);
/*0x01a2*/ bool ai01a2(void);
/*0x01a3*/ bool ai01a3(void);
/*0x01a4*/ bool aiShowText(void);
/*0x01a5*/ bool ai01a5(void);
/*0x01a6*/ bool aiIfTargetYDifferenceLessThan(void);
/*0x01a7*/ bool aiIfChrPropsoundcountZero(void);
/*0x01aa*/ bool ai01aa(void);
/*0x01ab*/ bool aiIfNumKnockedOutChrs(void);
/*0x01ad*/ bool aiReleaseObject(void);
/*0x01ae*/ bool ai01ae(void);
/*0x01af*/ bool aiChrGrabObject(void);
/*0x01b1*/ bool ai01b1(void);
/*0x01b2*/ bool ai01b2(void);
/*0x01b3*/ bool ai01b3(void);
/*0x01b4*/ bool ai01b4(void);
/*0x01b5*/ bool ai01b5(void);
/*0x01b6*/ bool aiConfigureSnow(void);
/*0x01b7*/ bool ai01b7(void);
/*0x01b8*/ bool aiSetAutogunType(void);
/*0x01b9*/ bool ai01b9(void);
/*0x01ba*/ bool ai01ba(void);
/*0x01bb*/ bool ai01bb(void);
/*0x01bc*/ bool ai01bc(void);
/*0x01bd*/ bool ai01bd(void);
/*0x01be*/ bool aiIfChrWeaponEquipped(void);
/*0x01bf*/ bool ai01bf(void);
/*0x01c0*/ bool ai01c0(void);
/*0x01c1*/ bool aiSetPunchDodgeList(void);
/*0x01c2*/ bool aiSetShootingAtMeList(void);
/*0x01c3*/ bool aiSetDarkRoomList(void);
/*0x01c4*/ bool aiSetPlayerDeadList(void);
/*0x01c5*/ bool ai01c5(void);
/*0x01c6*/ bool aiSetDodgeRating(void);
/*0x01c7*/ bool aiSetUnarmedDodgeRating(void);
/*0x01c8*/ bool ai01c8(void);
/*0x01c9*/ bool ai01c9(void);
/*0x01ca*/ bool ai01ca(void);
/*0x01cb*/ bool aiFadeScreen(void);
/*0x01cc*/ bool ai01cc(void);
/*0x01cd*/ bool ai01cd(void);
/*0x01ce*/ bool ai01ce(void);
/*0x01cf*/ bool ai01cf(void);
/*0x01d0*/ bool ai01d0(void);
/*0x01d1*/ bool aiSetObjPartVisible(void);
/*0x01d2*/ bool aiChrEmitSparks(void);
/*0x01d3*/ bool ai01d3(void);
/*0x01d4*/ bool ai01d4(void);
/*0x01d5*/ bool aiShowCutsceneChrs(void);
/*0x01d6*/ bool ai01d6(void);
/*0x01d7*/ bool ai01d7(void);
/*0x01d8*/ bool ai01d8(void);
/*0x01d9*/ bool ai01d9(void);
/*0x01da*/ bool ai01da(void);
/*0x01db*/ bool aiChrKill(void);
/*0x01dc*/ bool aiRemoveWeaponFromInventory(void);
/*0x01dd*/ bool ai01dd(void);
/*0x01de*/ bool ai01de(void);
/*0x01df*/ bool aiIfChrSameFloorDistanceToPadLessThan(void);
/*0x01e0*/ bool ai01e0(void);

#endif
