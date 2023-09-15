// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/HitScanWeapon.h"
#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class MPTESTING_API AShotgun : public AHitScanWeapon
{
	GENERATED_BODY()

public:
	virtual void FireShotgun(const TArray<FVector_NetQuantize>& HitTargets);
	void ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets);

protected:
	void CalculateDamage(FHitResult& HitResult, TMap<ABlaster*, float>& HitMap, TMap<ABlaster*, float>& HeadShotHitMap);
	
private:
	UPROPERTY(EditAnywhere, Category = "Weapon Properties | Scatter")
	uint32 NumberOfPellets = 3;

};
