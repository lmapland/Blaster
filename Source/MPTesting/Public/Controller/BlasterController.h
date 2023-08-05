// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterController.generated.h"

class ABlasterHUD;

/**
 * 
 */
UCLASS()
class MPTESTING_API ABlasterController : public APlayerController
{
	GENERATED_BODY()
public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDScore(float Score);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	virtual void OnPossess(APawn* InPawn) override;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	ABlasterHUD* HUD;
};
