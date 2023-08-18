// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/Pickup.h"
#include "HealthPickup.generated.h"

/**
 * 
 */
UCLASS()
class MPTESTING_API AHealthPickup : public APickup
{
	GENERATED_BODY()
public:
	AHealthPickup();

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	UPROPERTY(EditAnywhere, Category = "Item Properties | Pickup")
	float HealAmount = 10.f;

	UPROPERTY(EditAnywhere, Category = "Item Properties | Pickup")
	float HealingTime = 5.f;
};
