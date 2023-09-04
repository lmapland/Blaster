// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Weapon.h"
#include "HitScanWeapon.generated.h"

class UParticleSystem;
class USoundCue;
/**
 * 
 */
UCLASS()
class MPTESTING_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()
	
public:
	virtual void Fire(const FVector& HitTarget);

protected:
	void PerformTraceHit(const FVector& HitTarget, FTransform& SocketTransform, FHitResult& HitResult);
	void ApplyDamageOnHit(ABlaster* TargetHit, APawn* OwnerPawn, float DamageToApply, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& TraceEnd);
	void PlayFireBeginEffects(FTransform& SocketTransform);
	void FireBeam(FTransform& SocketTransform, const FVector& BeamEnd);
	void PlayFireImpactEffects(FHitResult& HitResult);

	UPROPERTY(EditAnywhere, Category = "Weapon Properties | Impact");
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties | Impact");
	USoundCue* ImpactSound;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties | Traveling");
	UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties | Firing");
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties | Firing");
	USoundCue* FireSound;

};
