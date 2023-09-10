// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BlasterGameState.generated.h"

class ABlasterPlayerState;

/**
 * 
 */
UCLASS()
class MPTESTING_API ABlasterGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScore(ABlasterPlayerState* ScoringPlayer);
	bool IsTopScoringPlayer(ABlasterPlayerState* PlayerState);
	void RemoveTopScoringPlayer(ABlasterPlayerState* PlayerToRemove);

private:
	float TopScore = 0.f;
	
	UPROPERTY(Replicated)
	TArray<ABlasterPlayerState*> TopScoringPlayers;

public:
	FORCEINLINE TArray<ABlasterPlayerState*> GetTopScoringPlayers() const { return TopScoringPlayers; }
};
