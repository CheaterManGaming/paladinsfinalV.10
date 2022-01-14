

//BLUEFIRE1337 PALADINS SDK - UPDATED 2021-08-19
#pragma once

#ifndef PALADINS_H

#include <iostream>
#include <Windows.h>
#include "pku.h"
#include <cstdint>
#include "vema.h"

#include "Mhyprot/baseadress.h"

inline namespace Paladins
{
	class Variables : public Option<Variables>
	{
	public:
		const char* dwProcessName = "Paladins.exe";
		DWORD dwProcessId;
		uint64_t dwProcess_Base;

		int ScreenHeight;
		int ScreenWidth;
		float ScreenCenterX = ScreenWidth / 2;
		float ScreenCenterY = ScreenHeight / 2;

	};
#define PakuAlam Paladins::Variables::Get()


	class Offsets : public Option<Offsets>
	{
	public:
		DWORD ENGINE = 0x3768130; // Paladins.exe + ENGINE -> UEngine 

		DWORD LOCALPLAYER = 0x6D8; // UEngine -> ULocalPlayer
		DWORD CONTROLLER = 0x68; // ULocalPlayer -> APlayerController

		DWORD ROTATION = 0x008C; // APlayerController -> FRotator
		DWORD WORLD_INFO = 0x0120; // APlayerController -> AWorldInfo

		DWORD ACKNOWLEDGEDPAWN = 0x0498; // APlayerController -> acknowledgedPawn | APawn

		DWORD PLAYER_FOV_MULTIPLIER = 0x04CC; // APlayerController -> FOVMultiplier, aka LODDistanceFactor | float
		DWORD CAMERA = 0x0478; // APlayerController -> ACamera

		DWORD REAL_LOCATION = 0x0494; // ACamera -> FVector
		DWORD DEAFULT_FOV = 0x0290; // ACamera -> float

		DWORD PAWN_LIST = 0x05B4; // AWorldInfo -> pawnList
		DWORD TIMESECONDS = 0x04EC; // AWorldInfo -> float

		DWORD TEAM = 0x4DC; // APlayerReplicationInfo -> Team 
		DWORD TEAMINDEX = 0x298; // Team -> int

		DWORD AMMO_COUNT = 0x4A4; // ATgDevice -> int
		DWORD AMMO_MAX_COUNT = 0x5F0; // ATgDevice -> int

		DWORD GLOW = 0x674; // acknowledgedPawn -> glow
		DWORD THIRDPERSON = 0x0624; // acknowledgedPawn -> write float
		DWORD WEAPON = 0x04E4; // APawn -> ATgDevice
		DWORD BASE_EYE_HEIGHT = 0x03A4; // APawn -> float
		DWORD HEALTH = 0xA90; // APawn -> float
		DWORD GET_HEALTH = 0x3C4; // APawn -> float
		DWORD MAX_HEALTH = 0x111C; // APawn -> float
		DWORD LOCATION = 0x0080; // APawn -> FVector
		DWORD VELOCITY = 0x0190; // APawn -> FVector
		DWORD PLAYER_REPLICATION_INFO = 0x0440; // APawn -> APlayerReplicationInfo
		DWORD NEXT_PAWN = 0x02AC; // APawn -> next APawn
		DWORD MESH = 0x048C; // APawn -> USkeletalMeshComponent

		DWORD BOUNDS = 0x00A0; // USkeletalMeshComponent -> FBoxSphereBounds
		DWORD LAST_RENDER_TIME = 0x0244; // USkeletalMeshComponent -> float

		DWORD PLAYER_NAME = 0x0290; // APlayerReplicationInfo -> FString
		DWORD RECOIL_SETTINGS = 0x0960; // ATgDevice -> FRecoilSettings
		DWORD ACCURACY_SETTINGS = 0x08C0; // ATgDevice -> FAccuracySettings

		DWORD CURRENT_PROJECTILES = 0x0610; // ATgDevice -> TArray<ATgProjectile>
		DWORD PROJECTILE_SPEED = 0x0280; // ATgProjectile -> float
	};
#define ClaimOffset Paladins::Offsets::Get()
}
#endif  !PALADINS_H


template<class T>
class TArray
{
public:
	int Length() const
	{
		return m_nCount;
	}

	bool IsValid() const
	{
		if (m_nCount > m_nMax)
			return false;
		if (!m_Data)
			return false;
		return true;
	}

	uint64_t GetAddress() const
	{
		return m_Data;
	}

	T GetById(int i)
	{
		return read<T>(m_Data + i * 8);
	}

protected:
	uint64_t m_Data;
	uint32_t m_nCount;
	uint32_t m_nMax;
};

struct FString : private TArray<wchar_t>
{
	std::wstring ToWString() const
	{
		wchar_t* buffer = new wchar_t[m_nCount];
		read_array(m_Data, buffer, m_nCount);
		std::wstring ws(buffer);
		delete[] buffer;

		return ws;
	}

	std::string ToString() const
	{
		std::wstring ws = ToWString();
		std::string str(ws.begin(), ws.end());

		return str;
	}
};

class ATgProjectile {
public:
	inline float GetSpeed() {
		return read<float>(data + ClaimOffset.PROJECTILE_SPEED);
	}
	uint64_t data;
};

class USkeletalMeshComponent {
public:
	inline FBoxSphereBounds GetBounds() {
		return read<FBoxSphereBounds>(data + ClaimOffset.BOUNDS);
	}

	inline float GetLastRenderTime() {
		return read<float>(data + ClaimOffset.LAST_RENDER_TIME);
	}

	inline bool IsVisible(float timeSeconds) {
		float lastRenderTime = GetLastRenderTime();
		return (lastRenderTime > timeSeconds - 0.05f);
	}

	uint64_t data;
};

class ATeamInfo {
public:
	inline int GetTeamIndex() {
		return read<int>(data + ClaimOffset.TEAMINDEX);
	}

	uint64_t data;
};

class APlayerReplicationInfo {
public:
	ATeamInfo GetTeamInfo() {
		return read<ATeamInfo>(data + ClaimOffset.TEAM);
	}

	FString GetName() {
		return read<FString>(data + ClaimOffset.PLAYER_NAME);
	}

	uint64_t data;
};

struct FRecoilSettings {
	int                                                bUsesRecoil;                                              // 0x0000(0x0004)
	float                                              fRecoilReductionPerSec;                                   // 0x0004(0x0004)
	float                                              fRecoilCenterDelay;                                       // 0x0008(0x0004)
	float                                              fRecoilSmoothRate;                                        // 0x000C(0x0004) 
};

struct FAccuracySettings
{
	int                                                bUsesAdvancedAccuracy;                                    // 0x0000(0x0004)
	float                                              fMaxAccuracy;                                             // 0x0004(0x0004)
	float                                              fMinAccuracy;                                             // 0x0008(0x0004)
	float                                              fAccuracyLossPerShot;                                     // 0x000C(0x0004)
	float                                              fAccuracyGainPerSec;                                      // 0x0010(0x0004)
	float                                              fAccuracyGainDelay;                                       // 0x0014(0x0004)
	int                                                nNumFreeShots;                                            // 0x0018(0x0004)
};

static FRecoilSettings oldRecoilSettings;
static bool hasRecoil = true;
static FAccuracySettings oldAccuracySettings;
static bool hasSpread = true;

class ATgDevice {
public:
	inline FRecoilSettings GetRecoil() {
		return read<FRecoilSettings>(data + ClaimOffset.RECOIL_SETTINGS);
	}

	inline bool SetRecoil(FRecoilSettings settings) {
		return write(data + ClaimOffset.RECOIL_SETTINGS, settings);
	}

	inline FAccuracySettings GetAccuracy() {
		return read<FAccuracySettings>(data + ClaimOffset.ACCURACY_SETTINGS);
	}

	inline bool SetAccuracy(FAccuracySettings settings) {
		return write(data + ClaimOffset.ACCURACY_SETTINGS, settings);
	}

	inline void NoRecoil(bool toggle = true) {
		if (toggle) {
			auto recoil = GetRecoil();
			if (recoil.bUsesRecoil) {
				oldRecoilSettings = recoil;
			}
			recoil.bUsesRecoil = false;
			recoil.fRecoilCenterDelay = 0;
			recoil.fRecoilReductionPerSec = 0;
			recoil.fRecoilSmoothRate = 0;
			hasRecoil = false;
			SetRecoil(recoil);
		}
		else {
			if (!hasRecoil) {
				hasRecoil = true;
				SetRecoil(oldRecoilSettings);
			}
		}
	}


	inline void NoSpread(bool toggle) {
		if (toggle) {
			auto accuracy = GetAccuracy();
			accuracy.fAccuracyGainPerSec = 0;
			accuracy.fMaxAccuracy = 1;
			accuracy.fMinAccuracy = 1;
			hasSpread = false;
			SetAccuracy(accuracy);
		}
		else {
			if (!hasSpread) {
				hasSpread = true;
				SetAccuracy(oldAccuracySettings);
			}
		}
	}


	inline int GetAmmoCount() {
		return read<int>(data + ClaimOffset.AMMO_COUNT);
	}

	inline TArray<ATgProjectile> GetProjectiles() {
		return read<TArray<ATgProjectile>>(data + ClaimOffset.CURRENT_PROJECTILES);
	}

	inline int GetMaxAmmoCount() {
		return read<int>(data + ClaimOffset.AMMO_MAX_COUNT);
	}

	inline void SetPerspective(bool thirdPerson) {
		write<float>(data + ClaimOffset.THIRDPERSON, thirdPerson ? 1.f : 0.f); // THIS SETS THE DURATION OF THE PERSPECIVE IN SEC
	}

	uint64_t data;
};

class APawn {
public:
	inline ATgDevice GetWeapon() {
		return read<ATgDevice>(data + ClaimOffset.WEAPON);
	}

	inline int GetHealth() {
		return read<int>(data + ClaimOffset.GET_HEALTH);
	}

	inline int GetMaxHealth() {
		return (int)read<float>(data + ClaimOffset.MAX_HEALTH);
	}

	inline bool SetHealth(float hp) {
		return write<float>(data + ClaimOffset.HEALTH, hp);
	}

	inline int SetMaxHealth(float hp) {
		return write<float>(data + ClaimOffset.MAX_HEALTH, hp);
	}

	inline FVector GetLocation() {
		return read<FVector>(data + ClaimOffset.LOCATION);
	}

	inline FRotator GetRotation() {
		return read<FRotator>(data + ClaimOffset.ROTATION);
	}

	inline FVector GetVelocity() {
		return read<FVector>(data + ClaimOffset.VELOCITY);
	}

	inline APlayerReplicationInfo GetPlayerReplicationInfo() {
		return read<APlayerReplicationInfo>(data + ClaimOffset.PLAYER_REPLICATION_INFO);
	}

	inline APawn GetNextPawn() {
		return read<APawn>(data + ClaimOffset.NEXT_PAWN);
	}

	inline float GetEyeHeight() {
		return read<float>(data + ClaimOffset.BASE_EYE_HEIGHT);
	}

	inline USkeletalMeshComponent GetMesh() {
		return read<USkeletalMeshComponent>(data + ClaimOffset.MESH);
	}

	inline void SetGlowhack(bool isGlowing) {
		auto offset = data + ClaimOffset.GLOW;
		auto current = read<unsigned long>(offset);
		if (isGlowing) {
			current |= (1u << 17);
		}
		else {
			current &= ~(1u << 17); // air = 2
		}

		write(offset, current);
	}

	uint64_t data;
};

class ACamera {
public:
	inline float GetDeafultFov() {
		return read<float>(data + ClaimOffset.DEAFULT_FOV);
	}

	inline FVector GetRealLocation() {
		return read<FVector>(data + ClaimOffset.REAL_LOCATION);
	}

	uint64_t data;
};

class AWorldInfo {
public:
	inline float GetTimeSeconds() {
		return read<float>(data + ClaimOffset.TIMESECONDS);
	}

	inline APawn GetPawnList() {
		return read<APawn>(data + ClaimOffset.PAWN_LIST);
	}

	uint64_t data;
};

class APlayerController {
public:
	inline FRotator GetRotation() {
		return read<FRotator>(data + ClaimOffset.ROTATION);
	}

	inline void SetRotation(FRotator rotation) {
		write(data + ClaimOffset.ROTATION, rotation);
	}

	inline float GetFovMultiplier() {
		return read<float>(data + ClaimOffset.PLAYER_FOV_MULTIPLIER);
	}

	inline APawn GetAcknowledgedPawn() {
		return read<APawn>(data + ClaimOffset.ACKNOWLEDGEDPAWN);
	}

	inline ACamera GetCamera() {
		return read<ACamera>(data + ClaimOffset.CAMERA);
	}

	inline AWorldInfo GetWorldInfo() {
		return read<AWorldInfo>(data + ClaimOffset.WORLD_INFO);
	}

	uint64_t data;
};

class ULocalPlayer {
public:
	inline APlayerController GetController() {
		return read<APlayerController>(data + ClaimOffset.CONTROLLER);
	}

	uint64_t data;
};

class UEngine {
public:
	inline ULocalPlayer GetLocalPlayer() {
		return read<ULocalPlayer>(read<uint64_t>(data + ClaimOffset.LOCALPLAYER));
	}

	uint64_t data;
};


UEngine GetUEngine(uint64_t base) {
	return read<UEngine>(base + ClaimOffset.ENGINE);
}

namespace vema
{
	// Constants that we need for maths stuff
#define Const_URotation180        32768
#define Const_PI                  3.14159265358979323
#define Const_RadToUnrRot         10430.3783504704527
#define Const_UnrRotToRad         0.00009587379924285
#define Const_URotationToRadians  Const_PI / Const_URotation180 

	int ClampYaw(int angle) {
		static const auto max = Const_URotation180 * 2;

		while (angle > max)
		{
			angle -= max;
		}

		while (angle < 0) {
			angle += max;
		}
		return angle;
	}
	int ClampPitch(int angle) {
		if (angle > 16000) {
			angle = 16000;
		}
		if (angle < -16000) {
			angle = -16000;
		}
		return angle;
	}
	float GetFOV(float Fov)
	{
		return Fov * 10.0f;
	}



	FRotator VectorToRotation(FVector vVector)
	{
		FRotator rRotation;

		rRotation.Yaw = atan2(vVector.Y, vVector.X) * Const_RadToUnrRot;
		rRotation.Pitch = atan2(vVector.Z, sqrt((vVector.X * vVector.X) + (vVector.Y * vVector.Y))) * Const_RadToUnrRot;
		rRotation.Roll = 0;

		return rRotation;
	}

	FVector RotationToVector(FRotator R)
	{
		float fYaw = R.Yaw * Const_URotationToRadians;
		float fPitch = R.Pitch * Const_URotationToRadians;
		float CosPitch = cos(fPitch);
		FVector Vec = FVector(cos(fYaw) * CosPitch,
			sin(fYaw) * CosPitch,
			sin(fPitch));
		return Vec;
	}

	float VectorMagnitude(FVector Vec) {
		return sqrt((Vec.X * Vec.X) + (Vec.Y * Vec.Y) + (Vec.Z * Vec.Z));
	}

	void Normalize(FVector& v)
	{
		float size = VectorMagnitude(v);

		if (!size)
		{
			v.X = v.Y = v.Z = 1;
		}
		else
		{
			v.X /= size;
			v.Y /= size;
			v.Z /= size;
		}
	}


	float VectorMagnitude(Vec2 Vec) {
		return sqrt((Vec.x * Vec.x) + (Vec.y * Vec.y));
	}
	void Normalize(Vec2& v)
	{
		float size = VectorMagnitude(v);

		if (!size)
		{
			v.x = v.y = 1;
		}
		else
		{
			v.x /= size;
			v.y /= size;
		}
	}


	void GetAxes(FRotator R, FVector& X, FVector& Y, FVector& Z)
	{
		X = RotationToVector(R);
		Normalize(X);
		R.Yaw += 16384;
		FRotator R2 = R;
		R2.Pitch = 0.f;
		Y = RotationToVector(R2);
		Normalize(Y);
		Y.Z = 0.f;
		R.Yaw -= 16384;
		R.Pitch += 16384;
		Z = RotationToVector(R);
		Normalize(Z);
	}

	FVector VectorSubtract(FVector s1, FVector s2)
	{
		FVector temp{};
		temp.X = s1.X - s2.X;
		temp.Y = s1.Y - s2.Y;
		temp.Z = s1.Z - s2.Z;

		return temp;
	}

	FVector VectorAdd(FVector s1, FVector s2)
	{
		FVector temp;
		temp.X = s1.X + s2.X;
		temp.Y = s1.Y + s2.Y;
		temp.Z = s1.Z + s2.Z;

		return temp;
	}

	FVector VectorScale(FVector s1, float scale)
	{
		FVector temp;
		temp.X = s1.X * scale;
		temp.Y = s1.Y * scale;
		temp.Z = s1.Z * scale;

		return temp;
	}

	float VectorDotProduct(const FVector& A, const FVector& B)
	{
		float tempx = A.X * B.X;
		float tempy = A.Y * B.Y;
		float tempz = A.Z * B.Z;

		return (tempx + tempy + tempz);
	}

	void AimAtVector(FVector TargetVec, FVector PlayerLocation, FRotator& AimRot)
	{
		FVector AimVec;
		AimVec.X = TargetVec.X - PlayerLocation.X;
		AimVec.Y = TargetVec.Y - PlayerLocation.Y;
		AimVec.Z = TargetVec.Z - PlayerLocation.Z;

		FRotator AimAtRot = VectorToRotation(AimVec);
		AimRot = AimAtRot;
	}

	FVector GetAngleTo(FVector TargetVec, FVector OriginVec)
	{
		FVector Diff;
		Diff.X = TargetVec.X - OriginVec.X;
		Diff.Y = TargetVec.Y - OriginVec.Y;
		Diff.Z = TargetVec.Z - OriginVec.Z;

		return Diff;
	}

	float GetDistance(FVector to, FVector from) {
		float deltaX = to.X - from.X;
		float deltaY = to.Y - from.Y;
		float deltaZ = to.Z - from.Z;

		return (float)sqrt(deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ);
	}

	float GetCrosshairDistance(float Xx, float Yy, float xX, float yY)
	{
		return sqrt((yY - Yy) * (yY - Yy) + (xX - Xx) * (xX - Xx));
	}
	bool W2S(FVector target, Vec2& dst, FRotator myRot, FVector myLoc, float fov)
	{
		FVector AxisX, AxisY, AxisZ, Delta, Transformed;
		GetAxes(myRot, AxisX, AxisY, AxisZ);

		Delta = VectorSubtract(target, myLoc);
		Transformed.X = VectorDotProduct(Delta, AxisY);
		Transformed.Y = VectorDotProduct(Delta, AxisZ);
		Transformed.Z = VectorDotProduct(Delta, AxisX);

		if (Transformed.Z < 1.00f)
			Transformed.Z = 1.00f;

		float GFOV = fov;
		float CentX = (PakuAlam.ScreenWidth / 2.0f);
		float CentY = (PakuAlam.ScreenHeight / 2.0f);

		dst.x = CentX + Transformed.X * (CentX / tan(GFOV * Const_PI / 360.0f)) / Transformed.Z;
		dst.y = CentY + -Transformed.Y * (CentX / tan(GFOV * Const_PI / 360.0f)) / Transformed.Z;

		if (dst.x >= 0.0f && dst.x <= PakuAlam.ScreenWidth)
		{
			if (dst.y >= 0.0f && dst.y <= PakuAlam.ScreenHeight)
			{
				return true;
			}
		}
		return false;
	}


}


