// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AnnouncementWidget.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS()
class MPTESTING_API UAnnouncementWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetWarmupTime(int32 Minutes, int32 Seconds);
	void SetInfoText(FString TextToDisplay);
	void SetAnnouncementText(FString TextToDisplay);

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AnnouncementText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WarmupTime;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* InfoText;
	
};
