#include "ExtPoltaRock.h"

#include "ExtPoltaFunction.h"

#include "kamek/hooks.h"
#include "Game/Boss/PoltaRock.h"
#include "Game/Boss/PoltaFunction.h"
#include "Game/Util/ActorAnimUtil.h"
#include "Game/Util/ActorInitUtil.h"
#include "Game/Util/ActorMovementUtil.h"
#include "Game/Util/ActorSensorUtil.h"
#include "Game/Util/ActionSoundUtil.h"
#include "Game/Util/EffectUtil.h"
#include "Game/Util/ObjUtil.h"
#include "Game/Util/LiveActorUtil.h"
#include "Game/NameObj/NameObjExecuteHolder.h"

class NameObj;

void PoltaRock_setColorRed(PoltaRock* pRock)
{
	pRock->mType = 3;
	MR::startBva(pRock, "Color");
	MR::setBvaFrameAndStop(pRock, 3.0f);
}


void PoltaRock_initInvalidateExplosionSensor(PoltaRock* pRock, const JMapInfoIter& rIter, const char* pname, bool b)
{
	MR::processInitFunction(pRock, rIter, pname, b);

	HitSensor* pExplodeSensor = pRock->getSensor("Bomb");
	if (pExplodeSensor != NULL)
		pExplodeSensor->invalidate();
}
kmCall(0x800F84D0, PoltaRock_initInvalidateExplosionSensor);


void PoltaRock_attackSensor(PoltaRock* pRock, HitSensor* pSender, HitSensor* pReciever)
{
	if (!PoltaFunction::isSensorAnyPolta(pRock->mOwner, pReciever) &&
		!PoltaFunction::isSensorPoltaRock(pRock->mOwner, pReciever) &&
		!PoltaFunction::isSensorPoltaBombTeresa(pRock->mOwner, pReciever) &&
		pRock->isNerve(&NrvExtPoltaRock::ExtPoltaRockNrvExplode::sInstance))
	{
		HitSensor* pExplodeSensor = pRock->getSensor("Bomb");

		if (pExplodeSensor != NULL && MR::isSensorValid(pExplodeSensor) && MR::isSensorEnemyAttack(pSender))
		{
			MR::sendMsgEnemyAttackExplosion(pReciever, pSender);
		}
	}
	else
	{
		if (!MR::isSensorPlayer(pReciever))
			return;

		if (!pRock->isEnableAttack())
		{
			MR::sendMsgPush(pSender, pReciever);
			return;
		}

		if (MR::sendMsgEnemyAttack(pReciever, pSender))
			pRock->generateKill();
	}
}
kmBranch(0x800F87A0, PoltaRock_attackSensor); // Replacing this too


void PoltaRock_generateKill(PoltaRock* pRock)
{
	switch (pRock->mType)
	{
	case 1:
		PoltaFunction::appearBombTeresaNormal(pRock->mOwner, pRock->mTranslation, TVec3f(0.0f));
		break;
	case 2:
		MR::appearCoinPop((const NameObj*)pRock->mOwner, pRock->mTranslation, 1);
		break;
	case 3:
		if (!pRock->isNerve(&NrvExtPoltaRock::ExtPoltaRockNrvExplode::sInstance))
		{
			pRock->setNerve(&NrvExtPoltaRock::ExtPoltaRockNrvExplode::sInstance);
			return; // DO NOT KILL
		}
	}
	pRock->kill();
}
kmBranch(0x800F8D50, PoltaRock_generateKill); // Just gonna replace the wholething...

namespace NrvExtPoltaRock {
	void ExtPoltaRockNrvExplode::execute(Spine* pSpine) const {
		PoltaRock* pRock = ((PoltaRock*)pSpine->mExecutor);

		if (MR::isFirstStep(pRock))
		{
			MR::disconnectToDrawTemporarily(pRock);
			MR::zeroVelocity(pRock);
			MR::startActionSound(pRock, "OjPoltaRockExplode", -1, -1, -1);
			MR::tryEmitEffect(pRock, "Explosion");
			MR::shakeCameraNormalStrong();
			HitSensor* pExplodeSensor = pRock->getSensor("Bomb");
			if (pExplodeSensor != NULL)
				pExplodeSensor->validate();
		}

		if (MR::isGreaterStep(pRock, 30))
		{
			HitSensor* pExplodeSensor = pRock->getSensor("Bomb");
			if (pExplodeSensor != NULL)
				pExplodeSensor->invalidate();

			pRock->makeActorDead(); // PLEASE temm le this works...
		}
	}
	ExtPoltaRockNrvExplode(ExtPoltaRockNrvExplode::sInstance);
}