// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/Pickup.h"
#include "JumpPickup.generated.h"

/**
 * 
 */
UCLASS()
class MPTESTING_API AJumpPickup : public APickup
{
	GENERATED_BODY()

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	UPROPERTY(EditAnywhere, Category = "Item Properties | Pickup")
	float JumpZVelocityPercent = 1.5f;
	
	UPROPERTY(EditAnywhere, Category = "Item Properties | Pickup")
	float JumpTime = 15.f;

};
