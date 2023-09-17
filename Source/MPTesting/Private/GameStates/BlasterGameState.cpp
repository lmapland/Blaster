// Fill out your copyright notice in the Description page of Project Settings.


#include "GameStates/BlasterGameState.h"
#include "Net/UnrealNetwork.h"
#include "PlayerStates/BlasterPlayerState.h"
#include "Controller/BlasterController.h"

void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterGameState, TopScoringPlayers);
	DOREPLIFETIME(ABlasterGameState, RedTeamScore);
	DOREPLIFETIME(ABlasterGameState, BlueTeamScore);
}

void ABlasterGameState::UpdateTopScore(ABlasterPlayerState* ScoringPlayer)
{
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

bool ABlasterGameState::IsTopScoringPlayer(ABlasterPlayerState* PlayerState)
{
	return TopScoringPlayers.Contains(PlayerState);
}

void ABlasterGameState::RemoveTopScoringPlayer(ABlasterPlayerState* PlayerToRemove)
{
	if (TopScoringPlayers.Contains(PlayerToRemove)) TopScoringPlayers.Remove(PlayerToRemove);
}

void ABlasterGameState::RemovePlayerFromTeam(ABlasterPlayerState* PlayerToRemove)
{
	if (RedTeam.Contains(PlayerToRemove)) RedTeam.Remove(PlayerToRemove);
	else if (BlueTeam.Contains(PlayerToRemove)) BlueTeam.Remove(PlayerToRemove);
}

void ABlasterGameState::RedTeamScores()
{
	RedTeamScore++;

	ABlasterController* BlasterPlayer = Cast<ABlasterController>(GetWorld()->GetFirstPlayerController());
	if (BlasterPlayer)
	{
		BlasterPlayer->SetHUDRedTeamScore(RedTeamScore);
	}
}

void ABlasterGameState::BlueTeamScores()
{
	BlueTeamScore++;

	ABlasterController* BlasterPlayer = Cast<ABlasterController>(GetWorld()->GetFirstPlayerController());
	if (BlasterPlayer)
	{
		BlasterPlayer->SetHUDBlueTeamScore(BlueTeamScore);
	}
}

void ABlasterGameState::OnRep_RedTeamScore()
{
	ABlasterController* BlasterPlayer = Cast<ABlasterController>(GetWorld()->GetFirstPlayerController());
	if (BlasterPlayer)
	{
		BlasterPlayer->SetHUDRedTeamScore(RedTeamScore);
	}
}

void ABlasterGameState::OnRep_BlueTeamScore()
{
	ABlasterController* BlasterPlayer = Cast<ABlasterController>(GetWorld()->GetFirstPlayerController());
	if (BlasterPlayer)
	{
		BlasterPlayer->SetHUDBlueTeamScore(BlueTeamScore);
	}
}
