// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/Pickup.h"
#include "SpeedPickup.generated.h"

/**
 * 
 */
UCLASS()
class MPTESTING_API ASpeedPickup : public APickup
{
	GENERATED_BODY()

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	UPROPERTY(EditAnywhere, Category = "Item Properties | Pickup")
	float SpeedPercent = 2.f; // .1 = 10%; aka a decrease. 1 = 100%; aka no change. 2 = 200%; aka double speed

	UPROPERTY(EditAnywhere, Category = "Item Properties | Pickup")
	float SpeedTime = 10.f;

};
