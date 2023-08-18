// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"

class ABlaster;
class ABlasterController;
class ABlasterHUD;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MPTESTING_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	friend ABlaster;

	UBuffComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/* Heal the specified Amount Over the specified Time */
	void HealOver(float Amount, float Time);

	void ReplenishShieldOver(float Amount, float Time);

	/* Change the Character's Movement Speed by the specified Percent For the specified length of Time */
	void IncreaseSpeedFor(float Percent, float Time);

	/* Change the Character's Jump Z Velocity by the specified Percent For the specified length of Time */
	void UpdateJumpVelocityFor(float Percent, float Time);

protected:
	void HealRampUp(float DeltaTime);
	void ShieldRampUp(float DeltaTime);

private:
	void ResetMovementSpeeds();
	void ResetJumpVelocity();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float Percent);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float Percent);

	UPROPERTY()
	ABlaster* Character;

	bool bHealing = false;
	float HealingRate = 0.f; // Heal per second
	float HealAmount = 0.f;

	bool bReplenishingShield = false;
	float ShieldReplenishRate = 0.f;
	float ShieldReplenishAmount = 0.f;

	FTimerHandle SpeedTimerHandle;
	FTimerHandle JumpTimerHandle;

};
