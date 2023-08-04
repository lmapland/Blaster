// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"

class ABlaster;
class ABlasterController;

/**
 * 
 */
UCLASS()
class MPTESTING_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Score() override;

	UFUNCTION()
	virtual void OnRep_Defeats();
	void AddToScore(float AmountToAdd);
	void AddToDefeats(int32 AmountToAdd);

private:
	UPROPERTY()
	ABlaster* Character;

	UPROPERTY()
	ABlasterController* Controller;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats = 0;
};
