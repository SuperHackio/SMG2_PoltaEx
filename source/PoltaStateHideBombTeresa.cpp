#include "PoltaStateHideBombTeresa.h"

#include "Game/Boss/Polta.h"
#include "Game/Boss/PoltaFunction.h"
#include "Game/Util/ActorAnimUtil.h"
#include "Game/Util/ActionSoundUtil.h"
#include "Game/Util/MathUtil.h"
#include "Game/Util/NerveUtil.h"
#include "Game/Util/PlayerUtil.h"
#include "ExtPoltaFunction.h"

PoltaStateHideBombTeresa::PoltaStateHideBombTeresa(Polta* pOwner) : ActorStateBase<Polta>("PoltaStateHideBombTeresa")
{
	mIsDead = true;
	mOwner = pOwner;
	mAppearType = -1;
	initNerve(&NrvPoltaStateHideBombTeresa::PoltaStateHideBombTeresaNrvStart::sInstance, 0);
}

void PoltaStateHideBombTeresa::appear()
{
	mIsDead = false;
	PoltaFunction::offArmRepair(mOwner);
	mAppearType++; // Just alternate between them
	if (mAppearType >= 3)
		mAppearType = 0;
	setNerve(&NrvPoltaStateHideBombTeresa::PoltaStateHideBombTeresaNrvStart::sInstance);
}

void PoltaStateHideBombTeresa::exeStart()
{
	if (MR::isFirstStep(this))
	{
		PoltaFunction::startAction(mOwner, "Sink", true);
		MR::startActionSound(mOwner, "PoltaChaseIntoGroundStr", -1, -1, -1);
	}

	if (MR::isActionEnd(mOwner))
	{
		PoltaFunction::disperseBombTeresa(mOwner);
		setNerve(&NrvPoltaStateHideBombTeresa::PoltaStateHideBombTeresaNrvWait::sInstance);
	}
}

void PoltaStateHideBombTeresa::exeWait()
{
	if (MR::isFirstStep(this))
	{
		//PoltaFunction::startAction(mOwner, "PopSign", true);
	}

	if (MR::isIntervalStep(this, 45) && !PoltaFunction::isMaxGenerateBombTeresa(mOwner))
	{
		// Don't wanna use any of the built-in functions
		// because I wanna spawn bomb-boos in a circle around Bouldergiest
		if (mAppearType == 0)
		{
			TVec3f Result = TVec3f();
			JMAVECScaleAdd((const Vec*)&mOwner->mGravity, (const Vec*)&mOwner->mTranslation, (Vec*)&Result, -120.0f);
			PoltaFunction::appearBombTeresaShadowRange(mOwner, 150 + 300, BATTLE_ARENA_RADIUS, Result); // The 150 is Bouldergiests sensor range. The +300 is just padding from me
		}			
		else if (mAppearType == 1)
			PoltaFunction::appearBombTeresaNormalRange(mOwner, 800, 800, *MR::getPlayerPos());
		else if (mAppearType == 2)
		{
			TVec3f Result = TVec3f();
			TVec3f Pos = TVec3f(*MR::getPlayerPos());
			Pos.y = mOwner->mTranslation.y; // Doesn't support tilted arenas I don't think....
			JMAVECScaleAdd((const Vec*)MR::getPlayerGravity(), (const Vec*)&Pos, (Vec*)&Result, -120.0f);
			PoltaFunction::appearBombTeresaShadowRange(mOwner, 0, 0, Result);
		}
	}

	if (MR::isGreaterEqualStep(this, 300))
	{
		setNerve(&NrvPoltaStateHideBombTeresa::PoltaStateHideBombTeresaNrvEnd::sInstance);
	}
}

void PoltaStateHideBombTeresa::exeEnd()
{
	if (MR::isFirstStep(this))
	{
		PoltaFunction::startAction(mOwner, "Pop", true);
		MR::startActionSound(mOwner, "PoltaChaseOutGroundStr", -1, -1, -1);
	}

	mOwner->rotateToPlayer();

	if (MR::isActionEnd(mOwner))
	{
		this->kill();
		PoltaFunction::disperseBombTeresa(mOwner);
		PoltaFunction::onArmRepair(mOwner);
	}
}

namespace NrvPoltaStateHideBombTeresa
{
	void PoltaStateHideBombTeresaNrvStart::execute(Spine* pSpine) const {
		((PoltaStateHideBombTeresa*)pSpine->mExecutor)->exeStart();
	}
	PoltaStateHideBombTeresaNrvStart(PoltaStateHideBombTeresaNrvStart::sInstance);


	void PoltaStateHideBombTeresaNrvWait::execute(Spine* pSpine) const {
		((PoltaStateHideBombTeresa*)pSpine->mExecutor)->exeWait();
	}
	PoltaStateHideBombTeresaNrvWait(PoltaStateHideBombTeresaNrvWait::sInstance);


	void PoltaStateHideBombTeresaNrvEnd::execute(Spine* pSpine) const {
		((PoltaStateHideBombTeresa*)pSpine->mExecutor)->exeEnd();
	}
	PoltaStateHideBombTeresaNrvEnd(PoltaStateHideBombTeresaNrvEnd::sInstance);
}