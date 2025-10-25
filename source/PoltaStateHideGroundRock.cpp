#include "PoltaStateHideGroundRock.h"

#include "Game/Boss/Polta.h"
#include "Game/Boss/PoltaFunction.h"
#include "Game/Util/ActorAnimUtil.h"
#include "Game/Util/ActionSoundUtil.h"
#include "Game/Util/MathUtil.h"
#include "Game/Util/NerveUtil.h"
#include "Game/Util/PlayerUtil.h"
#include "ExtPoltaFunction.h"

#define BATTLE_ARENA_RADIUS 2500.0f

PoltaStateHideGroundRock::PoltaStateHideGroundRock(Polta* pOwner) : ActorStateBase<Polta>("PoltaStateHideGroundRock")
{
	mIsDead = true;
	mOwner = pOwner;
	mAppearType = -1;
	initNerve(&NrvPoltaStateHideGroundRock::PoltaStateHideGroundRockNrvStart::sInstance, 0);
}

void PoltaStateHideGroundRock::appear()
{
	mIsDead = false;
	PoltaFunction::offArmRepair(mOwner);
	mAppearType++; // Just alternate between them
	if (mAppearType >= 2)
		mAppearType = 0;
	setNerve(&NrvPoltaStateHideGroundRock::PoltaStateHideGroundRockNrvStart::sInstance);
}

void PoltaStateHideGroundRock::exeStart()
{
	if (MR::isFirstStep(this))
	{
		PoltaFunction::startAction(mOwner, "Sink", true);
		MR::startActionSound(mOwner, "PoltaChaseIntoGroundStr", -1, -1, -1);
	}

	if (MR::isActionEnd(mOwner))
	{
		PoltaFunction::breakGroundRock(mOwner);
		setNerve(&NrvPoltaStateHideGroundRock::PoltaStateHideGroundRockNrvWait::sInstance);
	}
}

void PoltaStateHideGroundRock::exeWait()
{
	if (MR::isFirstStep(this))
	{
		//PoltaFunction::startAction(mOwner, "PopSign", true);
	}

	const s32 Length = 600;
	const s32 Interval = 32;

	if (MR::isIntervalStep(this, Interval) && MR::isLessStep(this, Length - Interval) && PoltaFunction::getCountDeadGroundRock(mOwner) != 0)
	{
		// Don't wanna use any of the built-in functions
		// because I wanna spawn bomb-boos in a circle around Bouldergiest
		if (mAppearType == 0)
		{
			//TVec3f Result = TVec3f();
			//JMAVECScaleAdd((const Vec*)&mOwner->mGravity, (const Vec*)&mOwner->mTranslation, (Vec*)&Result, -400.0f);
			PoltaFunction::appearGroundRockRange(mOwner, 150 + 450, BATTLE_ARENA_RADIUS - 150.0f, mOwner->mTranslation); // The 150 is Bouldergiests sensor range. The +450 is just padding from me
		}			
		else if (mAppearType == 1)
		{
			//TVec3f Result = TVec3f();
			TVec3f Pos = TVec3f(*MR::getPlayerPos());
			Pos.y = mOwner->mTranslation.y; // Doesn't support tilted arenas I don't think....
			//JMAVECScaleAdd((const Vec*)MR::getPlayerGravity(), (const Vec*)&Pos, (Vec*)&Result, -120.0f);
			PoltaFunction::appearGroundRockRange(mOwner, 0, 0, Pos);
		}
	}

	if (MR::isGreaterEqualStep(this, Length))
	{
		setNerve(&NrvPoltaStateHideGroundRock::PoltaStateHideGroundRockNrvEnd::sInstance);
	}
}

void PoltaStateHideGroundRock::exeEnd()
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
		PoltaFunction::onArmRepair(mOwner);
	}
}

namespace NrvPoltaStateHideGroundRock
{
	void PoltaStateHideGroundRockNrvStart::execute(Spine* pSpine) const {
		((PoltaStateHideGroundRock*)pSpine->mExecutor)->exeStart();
	}
	PoltaStateHideGroundRockNrvStart(PoltaStateHideGroundRockNrvStart::sInstance);


	void PoltaStateHideGroundRockNrvWait::execute(Spine* pSpine) const {
		((PoltaStateHideGroundRock*)pSpine->mExecutor)->exeWait();
	}
	PoltaStateHideGroundRockNrvWait(PoltaStateHideGroundRockNrvWait::sInstance);


	void PoltaStateHideGroundRockNrvEnd::execute(Spine* pSpine) const {
		((PoltaStateHideGroundRock*)pSpine->mExecutor)->exeEnd();
	}
	PoltaStateHideGroundRockNrvEnd(PoltaStateHideGroundRockNrvEnd::sInstance);
}