// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Characters/Blaster.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AShotgun::Fire(const FVector& HitTarget)
{
	AWeapon::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	FHitResult HitResult;
	FTransform SocketTransform;

	FVector BeamEnd;
	TMap<ABlaster*, uint32> HitMap;
	for (uint32 i = 0; i < NumberOfPellets; i++)
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

	for (auto HitPair : HitMap)
	{
		UE_LOG(LogTemp, Warning, TEXT("Applying damage %f to %s"), HitPair.Value, *HitPair.Key->GetName());
		ApplyDamageOnHit(HitPair.Key, OwnerPawn, HitPair.Value);
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
