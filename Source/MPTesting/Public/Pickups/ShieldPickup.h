// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/Pickup.h"
#include "ShieldPickup.generated.h"

/**
 * 
 */
UCLASS()
class MPTESTING_API AShieldPickup : public APickup
{
	GENERATED_BODY()

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	UPROPERTY(EditAnywhere, Category = "Item Properties | Pickup")
	float ShieldAmount = 20.f;

	UPROPERTY(EditAnywhere, Category = "Item Properties | Pickup")
	float ShieldTime = 3.f;
};
