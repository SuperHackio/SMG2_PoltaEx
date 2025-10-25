#include "ExtPoltaFunction.h"

#include "ExtPoltaRock.h"

#include "kamek/hooks.h"
#include "JSystem/JGeometry/TVec.h"
#include "Game/Util/MathUtil.h"
#include "Game/Util/ActorMovementUtil.h"
#include "Game/Util/MathUtil.h"
#include "Game/Boss/PoltaRockHolder.h"
#include "Game/Boss/PoltaArm.h"
#include "Game/Boss/BombTeresaHolder.h"
#include "Game/Boss/PoltaGroundRock.h"
#include "Game/Boss/PoltaGroundRockHolder.h"
#include "Game/Enemy/BombTeresa.h"

void PoltaFunction_appearRockCircle_Ext(PoltaRock* pRock)
{
	register s32 type;
	__asm {
		mr type, r29
	}

	if (type == 3)
		PoltaRock_setColorRed(pRock);
}
kmCall(0x800F7610, PoltaFunction_appearRockCircle_Ext);








namespace PoltaFunction
{
	bool appearRedRockCircle(Polta* pPolta, const TVec3f& vec, f32 f1, s32 r3, s32 r4)
	{
		if (MR::getRandom((s32)0, 101) > 70)
			return appearRockCircle(pPolta, vec, f1, r3, r4, 3);
		else
			return appearRockCircle(pPolta, vec, f1, r3, r4, 0);
	}
	kmCall(0x800F9D18, appearRedRockCircle);


	bool isSensorAnyPolta(const Polta* pPolta, const HitSensor* pSensor)
	{
		return PoltaFunction::isCoreSensor(pPolta, pSensor) ||
			PoltaFunction::isBodySensor(pPolta, pSensor) ||
			PoltaFunction::isArmSensor(pPolta, pSensor) ||
			PoltaFunction::isLeftArmSensor(pPolta, pSensor) ||
			PoltaFunction::isRightArmSensor(pPolta, pSensor) ||
			PoltaFunction::isHeadSensor(pPolta, pSensor);
	}

	bool isSensorPoltaRock(const Polta* pPolta, const HitSensor* pSensor)
	{
		PoltaRockHolder* pHolder = pPolta->mRockHolder;

		for (s32 i = 0; i < pHolder->mNumObjs; i++)
		{
			LiveActor* pRock = pHolder->getActor(i);
			if (pRock == NULL)
				continue;
			if (pSensor->mActor == pRock)
				return true;
		}
		return false;
	}

	bool isSensorPoltaBombTeresa(const Polta* pPolta, const HitSensor* pSensor)
	{
		BombTeresaHolder* pHolder = pPolta->mBombTeresaHolder;

		for (s32 i = 0; i < pHolder->mNumObjs; i++)
		{
			LiveActor* pRock = pHolder->getActor(i);
			if (pRock == NULL)
				continue;
			if (pSensor->mActor == pRock)
				return true;
		}
		return false;
	}


	void getRandomPosInRange(TVec3f* result, f32 radiusInner, f32 radiusOuter)
	{
		f32 D = MR::getRandomDegree(); // Extremely convinient
		f32 R = radiusInner + ((radiusOuter - radiusInner) * MR::getRandom());

		// TODO: Replace this with a Rotation-Enabled version?
		result->set(R * MR::cosDegree(D), 0.0f, R * MR::sinDegree(D));
	}

	bool appearBombTeresaShadowRange(const Polta* pPolta, f32 radiusInner, f32 radiusOuter, const TVec3f& basePos)
	{
		TVec3f(P);
		getRandomPosInRange(&P, radiusInner, radiusOuter);
		P += basePos;
		//OSReport("BombTeresa - %ff degrees, %ff range   [%ff, %ff, %ff] \n", D, R, P.x, P.y, P.z);

		BombTeresa* pTeresa = pPolta->mBombTeresaHolder->getDeadMember();
		if (pTeresa != NULL)
			return pTeresa->appearShadow(P, TVec3f(0.0f));
		return false;
	}
	bool appearBombTeresaNormalRange(const Polta* pPolta, f32 radiusInner, f32 radiusOuter, const TVec3f& basePos)
	{
		TVec3f(P);
		getRandomPosInRange(&P, radiusInner, radiusOuter);
		P += basePos;
		//OSReport("BombTeresa - %ff degrees, %ff range   [%ff, %ff, %ff] \n", D, R, P.x, P.y, P.z);

		BombTeresa* pTeresa = pPolta->mBombTeresaHolder->getDeadMember();
		if (pTeresa != NULL)
			return pTeresa->appearNormal(P, TVec3f(0.0f));
		return false;
	}
	bool appearGroundRockRange(Polta* pPolta, f32 radiusInner, f32 radiusOuter, const TVec3f& basePos)
	{
		TVec3f(P);
		getRandomPosInRange(&P, radiusInner, radiusOuter);
		P += basePos;
		OSReport("GroundRock - [%ff, %ff, %ff]\n", P.x, P.y, P.z);

		PoltaGroundRock* pRock = pPolta->mGroundRockHolder->getDeadMember();
		if (pRock != NULL)
		{
			pRock->start(pPolta, P);
			return true;
		}
		return false;
	}




	void rotateToHome(Polta* pPolta)
	{
		rotateToPoint(pPolta, pPolta->_E4);
	}

	void rotateToPoint(Polta* pPolta, const TVec3f& target)
	{
		f32 spd = calcToPointRotateSpeed(pPolta, target);
		pPolta->_F0 = spd;
		MR::rotateDirectionGravityDegree(pPolta, &pPolta->_C8, (f32)spd); // is that cast how I get the frsp??
	}

	f32 calcToPointRotateSpeed(Polta* pPolta, const TVec3f& target)
	{
		f32 what = pPolta->_F0;
		if (MR::isFaceToTargetHorizontalDegree(pPolta, target, pPolta->_C8, 10.0f))
		{
			what *= 0.99f;
		}
		else
		{
			TVec3f sideVec;
			MR::calcSideVec(&sideVec, pPolta);
			TVec3f sub;
			PSVECSubtract((const Vec*)&target, (const Vec*)&pPolta->mTranslation, (Vec*)&sub);
			
			if (sideVec.dot(sub) < 0.0f)
			{
				what += 0.1f;
			}
			else
			{
				what -= 0.1f;
			}
			what *= 0.998f;
		}

		if (what < -1.1f)
			return -1.1f;

		if (what > 1.1f)
			return 1.1f;

		return what;
	}




	// I LOVE ASSEMBLY RAAAAAAAAAAAAHHH
	void clampToArena(register TVec3f* pValue, register const TVec3f& rCenter, register f32 radius) {
		// just so future me doesn't forget what the hell this does...
		// load X into the upper half of f2
		// load Y into the upper half of f3
		// merge the upper half of f3 into the lower half of f2
		// This is actually just magic I swear...
		// Do it for the Output and Center values and we good
		register f32 FloatingZero = 0.0f;
		register f32 FloatingHalf = 0.5f;
		register f32 FloatingThree = 3.f;
		__asm {
			psq_l  f2, 0(pValue), 1, 0
			psq_l  f3, 8(pValue), 1, 0
			ps_merge00 f2, f2, f3

			psq_l  f3, 0(rCenter), 1, 0
			psq_l  f4, 8(rCenter), 1, 0
			ps_merge00 f5, f3, f4

			ps_sub f2, f2, f5 // Should be ps0(f2) - ps0(f3) and ps0(f2) - ps0(f3)

			ps_mul f1, f2, f2 // Should be ps0 * ps0 and ps1 * ps2

			ps_sum0 f1, f1, f1, f1 // should be ps0 + ps1 and = ps1

			// SQUARE ROOT TIME YIPEEE
			// Don't ask why I am manually inlining this...
			// from: JMASqrt(float)
			fcmpo     cr0, f1, FloatingZero
			ble NoSqrt
			frsqrte   f3, f1
			fmuls     f4, f3, f1
			fmuls     f1, f4, f3
			fsubs     f1, f1, FloatingThree
			fneg      f3, f1
			fmuls     f3, f3, f4
			fmuls     f3, f3, FloatingHalf
			NoSqrt:

			// Okay the sqrt is done and in f3.
			// Time for the genius division
			ps_div f1, f2, f3
			ps_madds0 f1, f1, radius, f5
			psq_st f1, 0(pValue), 1, 0
			ps_merge10 f1, f1, f1 // Can't believe I need to swap before I store Z again...
			psq_st f1, 8(pValue), 1, 0
		}
		// The above ASM is supposed to do the following (poorly written 'cause yes)
		//f32 vX = pX - cX;
		//f32 vY = pY - cY;
		//f32 magV = sqrt(vX * vX + vY * vY);
		//f32 aX = cX + (vX / magV) * R;
		//f32 aY = cY + (vY / magV) * R;
	}


	inline void setArmRepair(Polta* pPolta, bool v)
	{
		pPolta->mLeftArm->_D8 = pPolta->mRightArm->_D8 = !v ? 1 : 0;
	}

	void offArmRepair(Polta* pPolta)
	{
		setArmRepair(pPolta, false);
	}
	void onArmRepair(Polta* pPolta)
	{
		setArmRepair(pPolta, true);
	}
}
