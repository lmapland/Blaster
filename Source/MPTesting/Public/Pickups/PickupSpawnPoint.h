// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

class APickup;

UCLASS()
class MPTESTING_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:
	APickupSpawnPoint();

protected:
	virtual void BeginPlay() override;
	void SpawnPickup();

	UFUNCTION()
	void StartSpawnPickupTimer(AActor* DestroyedActor);

	void SpawnPickupTimerFinished();

	UPROPERTY(EditAnywhere, Category = "Spawner | Defaults")
	TArray<TSubclassOf<APickup>> PickupClasses;

private:
	FTimerHandle SpawnTimer;

	UPROPERTY(EditAnywhere, Category = "Spawner | Defaults")
	float SpawnPickupTimeMin = 10.f;
	
	UPROPERTY(EditAnywhere, Category = "Spawner | Defaults")
	float SpawnPickupTimeMax = 15.f;

	UPROPERTY()
	APickup* SpawnedPickup;

};
