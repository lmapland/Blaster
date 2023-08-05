// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerStates/BlasterPlayerState.h"
#include "Characters/Blaster.h"
#include "Controller/BlasterController.h"
#include "Net/UnrealNetwork.h"

void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterPlayerState, Defeats, COND_OwnerOnly);
}

void ABlasterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<ABlaster>(GetPawn()) : Character;
	if (Character && Character->GetBlasterController())
	{
		Character->GetBlasterController()->SetHUDScore(GetScore());
	}
}

void ABlasterPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<ABlaster>(GetPawn()) : Character;
	if (Character && Character->GetBlasterController())
	{
		Character->GetBlasterController()->SetHUDDefeats(Defeats);
	}
}

void ABlasterPlayerState::AddToScore(float AmountToAdd)
{
	SetScore(GetScore() + AmountToAdd);
	Character = Character == nullptr ? Cast<ABlaster>(GetPawn()) : Character;
	if (Character && Character->GetBlasterController())
	{
		Character->GetBlasterController()->SetHUDScore(GetScore());
	}
}

void ABlasterPlayerState::AddToDefeats(int32 AmountToAdd)
{
	Defeats += AmountToAdd;
	Character = Character == nullptr ? Cast<ABlaster>(GetPawn()) : Character;
	if (Character && Character->GetBlasterController())
	{
		Character->GetBlasterController()->SetHUDDefeats(Defeats);
	}
}
