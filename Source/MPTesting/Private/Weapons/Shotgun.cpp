// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Characters/Blaster.h"
#include "Controller/BlasterController.h"
#include "BlasterComponents/LagCompensationComponent.h"

void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::Fire(FVector());

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	FTransform SocketTransform;

	FVector BeamEnd;
	TMap<ABlaster*, float> HitMap;

	for (FVector_NetQuantize HitTarget : HitTargets)
	{
		FHitResult HitResult;
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

	TArray<ABlaster*> HitCharacters;
	if (InstigatorController)
	{
		for (auto HitPair : HitMap)
		{
			if (HitPair.Key)
			{
				HitCharacters.AddUnique(HitPair.Key);

				bool bCauseAuthDamage = !bUseServerRewind || OwnerPawn->IsLocallyControlled();
				if (HasAuthority() && bCauseAuthDamage)
				{
					UGameplayStatics::ApplyDamage(HitPair.Key, Damage * HitPair.Value, InstigatorController, this, UDamageType::StaticClass());
				}
			}
		}

		if (!HasAuthority() && bUseServerRewind)
		{
			Blaster = Blaster ? Blaster : Cast<ABlaster>(OwnerPawn);
			BlasterController = BlasterController ? BlasterController : Cast<ABlasterController>(InstigatorController);
			if (Blaster && Blaster->IsLocallyControlled() && BlasterController && Blaster->GetLagCompensation())
			{
				Blaster->GetLagCompensation()->ShotgunServerScoreRequest(HitCharacters, SocketTransform.GetLocation(), HitTargets, BlasterController->GetServerTime() - BlasterController->SingleTripTime);
			}
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

void AShotgun::CalculateDamage(FHitResult& HitResult, TMap<ABlaster*, float>& HitMap)
{
	if (ABlaster* BlasterChar = Cast<ABlaster>(HitResult.GetActor()))
	{
		if (HitMap.Contains(BlasterChar))
		{
			HitMap[BlasterChar]++;
		}
		else
		{
			HitMap.Emplace(BlasterChar, 1);
		}
	}
}
