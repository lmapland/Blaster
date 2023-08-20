// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Characters/Blaster.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::Fire(FVector());

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	FHitResult HitResult;
	FTransform SocketTransform;

	FVector BeamEnd;
	TMap<ABlaster*, uint32> HitMap;

	for (FVector_NetQuantize HitTarget : HitTargets)
	{
		PerformTraceHit(HitTarget, SocketTransform, HitResult);
		BeamEnd = HitResult.TraceEnd;

		if (HitResult.bBlockingHit)
		{
			BeamEnd = HitResult.ImpactPoint;
			PlayFireImpactEffects(HitResult);
			CalculateDamage(HitResult, HitMap);
		}
		FireBeam(SocketTransform, BeamEnd);
	}
	PlayFireBeginEffects(SocketTransform);

	if (HasAuthority() && InstigatorController)
	{
		for (auto HitPair : HitMap)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Applying damage %f to %s"), HitPair.Value, *HitPair.Key->GetName());
			ApplyDamageOnHit(HitPair.Key, OwnerPawn, HitPair.Value);
		}
	}
}

void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (!MuzzleFlashSocket) return;

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;

	for (uint32 i = 0; i < NumberOfPellets; i++)
	{
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		const FVector ToEndLoc = (SphereCenter + RandVec) - TraceStart;

		HitTargets.Add(FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()));
	}
}

void AShotgun::CalculateDamage(FHitResult& HitResult, TMap<ABlaster*, uint32>& HitMap)
{
	if (ABlaster* BlasterChar = Cast<ABlaster>(HitResult.GetActor()))
	{
		if (HitMap.Contains(BlasterChar))
		{
			HitMap[BlasterChar] += Damage;
		}
		else
		{
			HitMap.Emplace(BlasterChar, Damage);
		}
	}
}
