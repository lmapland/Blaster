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
	virtual void Fire(const FVector& HitTarget) override;

protected:
	void CalculateDamage(FHitResult& HitResult, TMap<ABlaster*, uint32>& HitMap);
	
private:
	UPROPERTY(EditAnywhere, Category = "Weapon Properties | Scatter")
	uint32 NumberOfPellets = 3;

};
