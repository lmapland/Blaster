// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/OverheadWidget.h"
#include "Components/TextBlock.h"

void UOverheadWidget::SetDisplayText(FString TextToDisplay)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	ENetRole LocalRole = InPawn->GetLocalRole();
	FString Role;
	switch (LocalRole)
	{
	case ENetRole::ROLE_Authority:
		Role = FString("Authority");
		break;
	case ENetRole::ROLE_AutonomousProxy:
		Role = FString("AutonomousProxy");
		break;
	case ENetRole::ROLE_SimulatedProxy:
		Role = FString("SimulatedProxy");
		break;
	case ENetRole::ROLE_None:
		Role = FString("None");
		break;
	}

	UE_LOG(LogTemp, Warning, TEXT("Local Role: %s"), *Role);
	SetDisplayText(FString::Printf(TEXT("Local Role: %s"), *Role));
}

void UOverheadWidget::ShowPlayerName(FString PlayerName)
{
	UE_LOG(LogTemp, Warning, TEXT("Player Name: %s"), *PlayerName);
	SetDisplayText(PlayerName);
}
