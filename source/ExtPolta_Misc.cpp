#include "ExtPolta_Misc.h"


#include "revolution/types.h"
#include "Kamek/hooks.h"
#include "Game/System/NerveExecutor.h"
#include "Game/Util/ActorAnimUtil.h"
#include "Game/Util/MathUtil.h"
#include "Game/Util/ActorStateUtil.h"
#include "Game/Util/ActorSwitchUtil.h"
#include "Game/Util/ObjUtil.h"
#include "Game/Boss/Polta.h"
#include "Game/Boss/PoltaFunction.h"
#include "Game/Boss/PoltaBattleLv1.h"
#include "Game/Boss/PoltaBattleLv2.h"

#include "ExtPoltaFunction.h"
#include "PoltaStateHideBombTeresa.h"
#include "PoltaStateHideGroundRock.h"
#include "PoltaStateHideChase.h"
#include "ModuleData_PoltaBattleLv1_Ext.h"
#include "ModuleData_PoltaBattleLv2_Ext.h"

#define ROCK_COUNT 0x0040

kmWrite16(0x800F90B0 + 2, ROCK_COUNT);
kmWrite16(0x800F9104 + 2, ROCK_COUNT);

#if defined(TWN)
kmWrite32(0x80731F18, 0x40A00000); // 5.0f
#elif defined(KOR)
kmWrite32(0x80730AB8, 0x40A00000); // 5.0f
#elif defined(JPN)
kmWrite32(0x807D80B8, 0x40A00000); // 5.0f
#elif defined(PAL)
kmWrite32(0x807DDFF8, 0x40A00000); // 5.0f
#elif defined(USA)
kmWrite32(0x807D88F8, 0x40A00000); // 5.0f
#endif
kmWrite16(0x800F4D30 + 2, 5);
kmWrite16(0x800F4D38 + 2, 5);
kmWrite16(0x800F578C + 2, 5);
kmWrite16(0x800F5794 + 2, 5);


// INIT
// ==================================================

// Init SW_B as well as SW_A
bool initNewSwitch(LiveActor* pPolta, const JMapInfoIter& rIter)
{
	bool r = MR::initUseStageSwitchReadA(pPolta, rIter);
	MR::initUseStageSwitchReadB(pPolta, rIter);
	return r;
}
kmCall(0x800F2894, initNewSwitch);



// Hand Damage fix
void onHandRepairOnDamage(Polta* pPolta, f32 v)
{
	MR::setBvaFrameAndStop(pPolta, v);

	PoltaFunction::onArmRepair(pPolta);
}
kmCall(0x800F71C8, onHandRepairOnDamage);



// LEVEL 1
// ==================================================

// Add new attack(s)
class PoltaStateStagger;
void initNewStates_PoltaBattleLv1(PoltaStateStagger* pStateStagger)
{
	register PoltaBattleLv1_Ext* pPoltaBattleLv1;
	register Polta* pPolta;
	__asm {
		mr pPoltaBattleLv1, r30
		mr pPolta, r31
	}

	pPoltaBattleLv1->mStateStagger = pStateStagger;

	pPoltaBattleLv1->mStateHideBombTeresa = new PoltaStateHideBombTeresa(pPolta);
	pPoltaBattleLv1->mStateHideGroundRock = new PoltaStateHideGroundRock(pPolta);
	pPoltaBattleLv1->mStateHideChase = new PoltaStateHideChase(pPolta);
}
kmCall(0x800F4C60, initNewStates_PoltaBattleLv1);

// Start the battle with a specific attack

void setNewNerveLv1(NerveExecutor* pExe)
{
	pExe->setNerve(&ExtPoltaBattleLv1::NrvHideGroundRock::sInstance);
}
kmCall(0x800F4D60, setNewNerveLv1);

void DecideAttackLv1(PoltaBattleLv1* pExe, const Nerve* pGenerateGroundRockNerve)
{
	s32 v = MR::getRandom((s32)0, 101);

	if (v < 25)
	{
		pExe->setNerve(&ExtPoltaBattleLv1::NrvHideChase::sInstance);
	}
	else if (v < 60 && PoltaFunction::getCountDeadGroundRock(pExe->mPoltaPtr) >= 4)
	{
		pExe->setNerve(pGenerateGroundRockNerve);
	}
	else
	{
		if (MR::getRandom((s32)0, 101) < 30)
			pExe->setNerve(&ExtPoltaBattleLv1::NrvHideBombTeresa::sInstance);
		else
			pExe->setNerve(&ExtPoltaBattleLv1::NrvHideGroundRock::sInstance);
	}
}
kmWrite16(0x800F5190 + 2, 0); // Only one rock throwing cycle between attacks
kmWrite32(0x800F5194, 0x41820024);
kmWrite32(0x800F51A4, 0x60000000); // Take out the check for having enough ground rocks, as we can do that ourselves
kmCall(0x800F51B0, DecideAttackLv1);

kmWrite32(0x800F51C4, 0x807F0024);
s32 updateCycleId(s32 cycleid)
{
	if (++cycleid > 1)
		cycleid = 0;
	return cycleid;
}
kmCall(0x800F51C8, updateCycleId);
kmWrite32(0x800F51CC, 0x60000000);
kmWrite32(0x800F51D0, 0x60000000);
kmWrite32(0x800F51D4, 0x60000000);
kmWrite32(0x800F51D8, 0x7C601B78);

namespace ExtPoltaBattleLv1
{
	void NrvHideBombTeresa::execute(Spine* pSpine) const {
		PoltaBattleLv1_Ext* pExe = (PoltaBattleLv1_Ext*)(pSpine->mExecutor);
		MR::updateActorStateAndNextNerve(pExe, pExe->mStateHideBombTeresa, &NrvPoltaBattleLv1::PoltaBattleLv1NrvWait::sInstance);
	}
	NrvHideBombTeresa(NrvHideBombTeresa::sInstance);


	void NrvHideGroundRock::execute(Spine* pSpine) const {
		PoltaBattleLv1_Ext* pExe = (PoltaBattleLv1_Ext*)(pSpine->mExecutor);
		MR::updateActorStateAndNextNerve(pExe, pExe->mStateHideGroundRock, &NrvPoltaBattleLv1::PoltaBattleLv1NrvWait::sInstance);
	}
	NrvHideGroundRock(NrvHideGroundRock::sInstance);


	void NrvHideChase::execute(Spine* pSpine) const {
		PoltaBattleLv1_Ext* pExe = (PoltaBattleLv1_Ext*)(pSpine->mExecutor);
		if (pSpine->mStep <= 0)
			pExe->mStateHideChase->setup(300, 10, 1, 8.f, false);
		MR::updateActorStateAndNextNerve(pExe, pExe->mStateHideChase, &NrvPoltaBattleLv1::PoltaBattleLv1NrvWait::sInstance);
	}
	NrvHideChase(NrvHideChase::sInstance);
}


// LEVEL 2
// ==================================================

// Add new attack(s)
class PoltaStateStagger;
void initNewStates_PoltaBattleLv2(PoltaStateStagger* pStateStagger)
{
	register PoltaBattleLv2_Ext* pPoltaBattleLv2;
	register Polta* pPolta;
	__asm {
		mr pPoltaBattleLv2, r30
		mr pPolta, r31
	}

	pPoltaBattleLv2->mStateStagger = pStateStagger;

	pPoltaBattleLv2->mStateHideBombTeresa = new PoltaStateHideBombTeresa(pPolta);
	pPoltaBattleLv2->mStateHideGroundRock = new PoltaStateHideGroundRock(pPolta);
	pPoltaBattleLv2->mStateHideChase = new PoltaStateHideChase(pPolta);
}
kmCall(0x800F56C8, initNewStates_PoltaBattleLv2);

// Start the second phase with a specific attack
void setNewNerveLv2(NerveExecutor* pExe)
{
	pExe->setNerve(&ExtPoltaBattleLv2::NrvHideGroundRock::sInstance);
}
kmCall(0x800F57B4, setNewNerveLv2);


void NrvPoltaBattleLv2_PoltaBattleLv2NrvWait_execute(void*, Spine* pSpine)
{
	PoltaBattleLv2_Ext* pExe = (PoltaBattleLv2_Ext*)(pSpine->mExecutor);
	pExe->updateWait();
	s32 currentCycle = pExe->_2C;

	if (currentCycle == 1)
	{
		pExe->setNerve(&NrvPoltaBattleLv2::PoltaBattleLv2NrvGenerateRock::sInstance);
	}
	else
	{
		s32 v = MR::getRandom((s32)0, 101);

		bool HasAttacked = false;
		if (v < 40)
		{
			HasAttacked = pExe->tryAttackGround();
		}

		if (!HasAttacked)
		{
			if (v < 25)
			{
				pExe->setNerve(&ExtPoltaBattleLv2::NrvHideChase::sInstance);
			}
			else if (v < 60 && PoltaFunction::getCountDeadGroundRock(pExe->mPoltaPtr) >= 8)
			{
				pExe->setNerve(&NrvPoltaBattleLv2::PoltaBattleLv2NrvGenerateGroundRock::sInstance);
			}
			else
			{
				if (MR::getRandom((s32)0, 101) < 30)
					pExe->setNerve(&ExtPoltaBattleLv2::NrvHideBombTeresa::sInstance);
				else
					pExe->setNerve(&ExtPoltaBattleLv2::NrvHideGroundRock::sInstance);
			}
		}
	}

	currentCycle++;
	if (currentCycle >= 2)
		currentCycle = 0;
	pExe->_2C = currentCycle;
}

kmBranch(0x800F6530, NrvPoltaBattleLv2_PoltaBattleLv2NrvWait_execute);


namespace ExtPoltaBattleLv2
{
	void NrvHideBombTeresa::execute(Spine* pSpine) const {
		PoltaBattleLv2_Ext* pExe = (PoltaBattleLv2_Ext*)(pSpine->mExecutor);
		MR::updateActorStateAndNextNerve(pExe, pExe->mStateHideBombTeresa, &NrvPoltaBattleLv2::PoltaBattleLv2NrvWait::sInstance);
	}
	NrvHideBombTeresa(NrvHideBombTeresa::sInstance);


	void NrvHideGroundRock::execute(Spine* pSpine) const {
		PoltaBattleLv2_Ext* pExe = (PoltaBattleLv2_Ext*)(pSpine->mExecutor);
		MR::updateActorStateAndNextNerve(pExe, pExe->mStateHideGroundRock, &NrvPoltaBattleLv2::PoltaBattleLv2NrvWait::sInstance);
	}
	NrvHideGroundRock(NrvHideGroundRock::sInstance);


	void NrvHideChase::execute(Spine* pSpine) const {
		PoltaBattleLv2_Ext* pExe = (PoltaBattleLv2_Ext*)(pSpine->mExecutor);
		if (pSpine->mStep <= 0)
			pExe->mStateHideChase->setup(150, 10, 2, 9.f, false);
		MR::updateActorStateAndNextNerve(pExe, pExe->mStateHideChase, &NrvPoltaBattleLv2::PoltaBattleLv2NrvWait::sInstance);
	}
	NrvHideChase(NrvHideChase::sInstance);
}


// Allow the spawning of Power Stars because yes
void tryInitPowerStar(LiveActor* pActor, const JMapInfoIter& rIter)
{
	if (!MR::useStageSwitchWriteDead(pActor, rIter))
		MR::declarePowerStar(pActor);
}
kmCall(0x800F2888, tryInitPowerStar);

bool tryAppearPowerStar(LiveActor* pActor)
{
	bool isvalid;
	if (isvalid = !MR::isValidSwitchDead(pActor))
		MR::requestAppearPowerStar(pActor, pActor->mTranslation);
	return !isvalid;
}
kmCall(0x800F2F58, tryAppearPowerStar);