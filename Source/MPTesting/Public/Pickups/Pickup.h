// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

class USphereComponent;
class USoundCue;
class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class MPTESTING_API APickup : public AActor
{
	GENERATED_BODY()
	
public:
	APickup();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere, Category = "Item Properties | Pre-Pickup")
	float BaseTurnRate = 45.f;
	
	UPROPERTY(EditAnywhere, Category = "Item Properties | Pickup")
	USoundCue* PickupSound;

	UPROPERTY(EditAnywhere, Category = "Item Properties | Pickup")
	UNiagaraSystem* OnPickupEffect;

	/* Allows the particle system that spawns when the item is picked up to spawn in a different location than the default */
	UPROPERTY(EditAnywhere, Category = "Item Properties | Pickup")
	FVector PickupLocationOffset = FVector(0.f, 0.f, 50.f);

	UPROPERTY(VisibleAnywhere)
	UNiagaraComponent* PickupEffectComponent;

private:
	void BindOverlapTimerFinished();

	UPROPERTY(EditAnywhere)
	USphereComponent* OverlapSphere;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* PickupMesh;

	FTimerHandle BindOverlapTimer;

	float BindOverlapTime = 0.25f;

};
