#include "PoltaStateHideChase.h"

#include "Game/Boss/Polta.h"
#include "Game/Boss/PoltaFunction.h"
#include "Game/Boss/PoltaSensorCtrl.h"
#include "Game/Util/ActorAnimUtil.h"
#include "Game/Util/ActionSoundUtil.h"
#include "Game/Util/ActorSwitchUtil.h"
#include "Game/Util/ActorMovementUtil.h"
#include "Game/Util/ActorSensorUtil.h"
#include "Game/Util/MathUtil.h"
#include "Game/Util/NerveUtil.h"
#include "Game/Util/PlayerUtil.h"
#include "ExtPoltaFunction.h"

PoltaStateHideChase::PoltaStateHideChase(Polta* pOwner) : ActorStateBase<Polta>("PoltaStateHideChase")
{
	mIsDead = true;
	mOwner = pOwner;

	mChaseStepLength = 0;
	mPopDelay = 0;
	mPopNum = 0;
	mMoveSpeed = 0.0f;
	mUseSW_B = false;

	initNerve(&NrvPoltaStateHideChase::PoltaStateHideChaseNrvStart::sInstance, 0);
}

void PoltaStateHideChase::appear()
{
	mIsDead = false;
	PoltaFunction::offArmRepair(mOwner);

	setNerve(&NrvPoltaStateHideChase::PoltaStateHideChaseNrvStart::sInstance);
}

void PoltaStateHideChase::moveToPlayer(f32 Accel, f32 Max)
{
	TVec3f front = TVec3f(0.0f);
	MR::calcFrontVec(&front, mOwner);
	mOwner->mVelocity.zero();
	mAccel += Accel;
	mAccel = MR::clamp(mAccel, 0, Max + 2);
	MR::addVelocityMoveToDirection(mOwner, front, mAccel);
	MR::restrictVelocity(mOwner, Max);

	mOwner->rotateToPlayer();
	mOwner->rotateToPlayer();

	if (PSVECDistance((const Vec*)&mOwner->mTranslation, (const Vec*)&mOwner->_E4) > BATTLE_MOVE_RADIUS)
		PoltaFunction::clampToArena(&mOwner->mTranslation, mOwner->_E4, BATTLE_MOVE_RADIUS);

	MR::startActionSound(mOwner, "PoltaChasePlayer", -1, -1, -1);
}

void PoltaStateHideChase::exeStart()
{
	if (MR::isFirstStep(this))
	{
		PoltaFunction::startAction(mOwner, "Sink", true);
		MR::startActionSound(mOwner, "PoltaChaseIntoGroundStr", -1, -1, -1);
		mAccel = 0;
	}

	if (MR::isStep(this, 120))
	{
		PoltaFunction::disperseBombTeresa(mOwner);

		if (MR::isValidSwitchB(mOwner))
			MR::onSwitchB(mOwner);

	}

	if (MR::isActionEnd(mOwner))
	{
		PoltaFunction::disperseBombTeresa(mOwner);

		MR::invalidateHitSensor(mOwner->mSensorCtrl->mBodySensors[0]);
		MR::invalidateHitSensor(mOwner->mSensorCtrl->mBodySensors[1]);
		MR::invalidateHitSensor(mOwner->mSensorCtrl->mBodySensors[2]);
		setNerve(&NrvPoltaStateHideChase::PoltaStateHideChaseNrvFollow::sInstance);
	}
}

void PoltaStateHideChase::exeFollow()
{
	if (MR::isFirstStep(this))
	{
		PoltaFunction::startAction(mOwner, "PopSign", true);
	}
	bool isGreater = MR::isGreaterStep(this, mChaseStepLength);

	moveToPlayer(0.9f, mMoveSpeed * (isGreater ? 2.f : 1.f));
	if (isGreater)
		mOwner->rotateToPlayer(); // Another one

	if (PSVECDistance((const Vec*)&mOwner->mTranslation, (const Vec*)MR::getPlayerPos()) < 150.0f || MR::isGreaterStep(this, mChaseStepLength*2))
	{
		setNerve(&NrvPoltaStateHideChase::PoltaStateHideChaseNrvPop::sInstance);
	}
}

void PoltaStateHideChase::exePop()
{
	if (MR::isFirstStep(this))
	{
		PoltaFunction::startAction(mOwner, "PopPrepare", true);
		PoltaFunction::disperseBombTeresa(mOwner);
		mPopPhase = false;
	}

	if (MR::isLessStep(this, mPopDelay))
	{
		moveToPlayer(-1.1f, mMoveSpeed);
		return;
	}

	if (MR::isStep(this, mPopDelay))
	{
		mOwner->mVelocity.zero();
		mAccel = 0;
		PoltaFunction::startAction(mOwner, "Pop", true);
		MR::startActionSound(mOwner, "PoltaChaseOutGroundStr", -1, -1, -1);
		return;
	}
	if (MR::isStep(this, mPopDelay + 30))
	{
		MR::validateHitSensor(mOwner->mSensorCtrl->mBodySensors[0]);
		MR::validateHitSensor(mOwner->mSensorCtrl->mBodySensors[1]);
		MR::validateHitSensor(mOwner->mSensorCtrl->mBodySensors[2]);
	}

	if (!mPopPhase && MR::isActionEnd(mOwner))
	{
		mPopPhase = true;
		PoltaFunction::startAction(mOwner, "Sink", true);
		MR::startActionSound(mOwner, "PoltaChaseIntoGroundStr", -1, -1, -1);
	}
	else if (mPopPhase && MR::isActionEnd(mOwner))
	{
		MR::invalidateHitSensor(mOwner->mSensorCtrl->mBodySensors[0]);
		MR::invalidateHitSensor(mOwner->mSensorCtrl->mBodySensors[1]);
		MR::invalidateHitSensor(mOwner->mSensorCtrl->mBodySensors[2]);
		mPopNum--;
		if (mPopNum <= 0)
			setNerve(&NrvPoltaStateHideChase::PoltaStateHideChaseNrvReturn::sInstance);
		else
			setNerve(&NrvPoltaStateHideChase::PoltaStateHideChaseNrvFollow::sInstance);
	}
}

void PoltaStateHideChase::exeReturn()
{
	if (MR::isFirstStep(this))
	{
		PoltaFunction::startAction(mOwner, "PopSign", true);
	}

	f32 dist = PSVECDistance((const Vec*)&mOwner->mTranslation, (const Vec*)&mOwner->_E4);
	f32 distMax = MR::clamp(dist, 0.0f, 100.0f);

	TVec3f front = TVec3f(0.0f);
	MR::calcFrontVec(&front, mOwner);
	mOwner->mVelocity.zero();
	MR::addVelocityMoveToDirection(mOwner, front, distMax * 0.1f);
	MR::attenuateVelocity(mOwner, 0.9f);
	MR::restrictVelocity(mOwner, 10.f);

	PoltaFunction::rotateToHome(mOwner);
	PoltaFunction::rotateToHome(mOwner);
	if (MR::isGreaterStep(this, 200))
	{
		PoltaFunction::rotateToHome(mOwner);
		PoltaFunction::rotateToHome(mOwner);
		MR::restrictVelocity(mOwner, 5.f);
	}
	if (PSVECDistance((const Vec*)&mOwner->mTranslation, (const Vec*)&mOwner->_E4) > BATTLE_MOVE_RADIUS)
		PoltaFunction::clampToArena(&mOwner->mTranslation, mOwner->_E4, BATTLE_MOVE_RADIUS);

	if (dist < 10.0f)
	{
		mOwner->mVelocity.zero();
		mOwner->mTranslation.set(mOwner->_E4);
		setNerve(&NrvPoltaStateHideChase::PoltaStateHideChaseNrvEnd::sInstance);
	}
}

void PoltaStateHideChase::exeEnd()
{
	if (MR::isFirstStep(this))
	{
		PoltaFunction::startAction(mOwner, "Pop", true);
		MR::startActionSound(mOwner, "PoltaChaseOutGroundStr", -1, -1, -1);

		MR::validateHitSensor(mOwner->mSensorCtrl->mBodySensors[0]);
		MR::validateHitSensor(mOwner->mSensorCtrl->mBodySensors[1]);
		MR::validateHitSensor(mOwner->mSensorCtrl->mBodySensors[2]);
	}

	mOwner->rotateToPlayer();
	mOwner->rotateToPlayer();

	if (MR::isActionEnd(mOwner))
	{
		this->kill();
		PoltaFunction::onArmRepair(mOwner);
	}
}

namespace NrvPoltaStateHideChase
{
	void PoltaStateHideChaseNrvStart::execute(Spine* pSpine) const {
		((PoltaStateHideChase*)pSpine->mExecutor)->exeStart();
	}
	PoltaStateHideChaseNrvStart(PoltaStateHideChaseNrvStart::sInstance);


	void PoltaStateHideChaseNrvFollow::execute(Spine* pSpine) const {
		((PoltaStateHideChase*)pSpine->mExecutor)->exeFollow();
	}
	PoltaStateHideChaseNrvFollow(PoltaStateHideChaseNrvFollow::sInstance);


	void PoltaStateHideChaseNrvPop::execute(Spine* pSpine) const {
		((PoltaStateHideChase*)pSpine->mExecutor)->exePop();
	}
	PoltaStateHideChaseNrvPop(PoltaStateHideChaseNrvPop::sInstance);


	void PoltaStateHideChaseNrvReturn::execute(Spine* pSpine) const {
		((PoltaStateHideChase*)pSpine->mExecutor)->exeReturn();
	}
	PoltaStateHideChaseNrvReturn(PoltaStateHideChaseNrvReturn::sInstance);


	void PoltaStateHideChaseNrvEnd::execute(Spine* pSpine) const {
		((PoltaStateHideChase*)pSpine->mExecutor)->exeEnd();
	}
	PoltaStateHideChaseNrvEnd(PoltaStateHideChaseNrvEnd::sInstance);
}