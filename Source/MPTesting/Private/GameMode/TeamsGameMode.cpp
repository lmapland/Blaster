// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/TeamsGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameStates/BlasterGameState.h"
#include "PlayerStates/BlasterPlayerState.h"
#include "Controller/BlasterController.h"

ATeamsGameMode::ATeamsGameMode()
{
	bTeamsMatch = true;
}

void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if (BlasterGameState)
	{
		ABlasterPlayerState* State = NewPlayer->GetPlayerState<ABlasterPlayerState>();
		if (State && State->GetTeam() == ETeam::ET_NoTeam)
		{
			if (BlasterGameState->GetBlueTeam().Num() >= BlasterGameState->GetRedTeam().Num())
			{
				BlasterGameState->AddToRedTeam(State);
				State->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				BlasterGameState->AddToBlueTeam(State);
				State->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if (BlasterGameState)
	{
		for (auto PlayerState : BlasterGameState->PlayerArray)
		{
			ABlasterPlayerState* State = Cast<ABlasterPlayerState>(PlayerState.Get());
			if (State && State->GetTeam() == ETeam::ET_NoTeam)
			{
				if (BlasterGameState->GetBlueTeam().Num() >= BlasterGameState->GetRedTeam().Num())
				{
					BlasterGameState->AddToRedTeam(State);
					State->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					BlasterGameState->AddToBlueTeam(State);
					State->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}
}

void ATeamsGameMode::Logout(AController* Exiting)
{
	ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	ABlasterPlayerState* PlayerState = Exiting->GetPlayerState<ABlasterPlayerState>();
	if (!BlasterGameState || !PlayerState) return;

	BlasterGameState->RemovePlayerFromTeam(PlayerState);
}

void ATeamsGameMode::PlayerEliminated(ABlaster* ElimmedCharacter, ABlasterController* VictimController, ABlasterController* AttackerController)
{
	Super::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);

	ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	if (BlasterGameState && AttackerPlayerState)
	{
		if (AttackerPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			BlasterGameState->BlueTeamScores();
		}
		else if (AttackerPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			BlasterGameState->RedTeamScores();
		}
	}
}

bool ATeamsGameMode::PlayerShouldDamage(AController* PlayerOne, AController* PlayerTwo)
{
	ABlasterPlayerState* AttackerPState = PlayerOne->GetPlayerState<ABlasterPlayerState>();
	ABlasterPlayerState* VictimPState = PlayerTwo->GetPlayerState<ABlasterPlayerState>();
	if (!AttackerPState || !VictimPState) return false;
	if (AttackerPState == VictimPState) return false;

	if (!bFriendlyFireEnabled && AttackerPState->GetTeam() != VictimPState->GetTeam())
	{
		return true;
	}
	
	return bFriendlyFireEnabled;
}
