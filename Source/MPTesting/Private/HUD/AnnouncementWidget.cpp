// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/AnnouncementWidget.h"
#include "Components/TextBlock.h"

void UAnnouncementWidget::SetWarmupTime(int32 Minutes, int32 Seconds)
{
	if (WarmupTime)
	{
		const FString CountdownString = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		WarmupTime->SetText(FText::FromString(CountdownString));
	}
}

void UAnnouncementWidget::SetInfoText(FString TextToDisplay)
{
	if (InfoText)
	{
		InfoText->SetText(FText::FromString(TextToDisplay));
	}
}

void UAnnouncementWidget::SetAnnouncementText(FString TextToDisplay)
{
	if (AnnouncementText)
	{
		AnnouncementText->SetText(FText::FromString(TextToDisplay));
	}
}
