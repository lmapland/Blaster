// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterComponents/BuffComponent.h"
#include "Characters/Blaster.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bHealing) HealRampUp(DeltaTime);

	if (bReplenishingShield) ShieldRampUp(DeltaTime);
}

void UBuffComponent::HealOver(float Amount, float Time)
{
	if (Character && Character->IsElimmed()) return;

	if (Time == 0.f)
	{
		// No healing over time. Heal right now.
		Character->UpdateHealth(Amount);
	}
	else
	{
		HealingRate = Amount / Time;
		HealAmount += Amount;
		bHealing = true;
	}
}

void UBuffComponent::ReplenishShieldOver(float Amount, float Time)
{
	if (Character && Character->IsElimmed()) return;

	if (Time == 0.f)
	{
		// No replenishment over time. Replenish shield right now.
		Character->ReplenishShield(Amount);
	}
	else
	{
		ShieldReplenishRate = Amount / Time;
		ShieldReplenishAmount += Amount;
		bReplenishingShield = true;
	}
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if (!bHealing || !Character || Character->IsElimmed()) return;

	// Get amount to heal this tick, being mindful of if the value will heal more than the total amount we should heal
	float ToHealThisTick = HealingRate * DeltaTime;
	if (HealAmount - ToHealThisTick < 0.f)
	{
		ToHealThisTick = HealAmount;
	}
	
	Character->UpdateHealth(ToHealThisTick);

	HealAmount -= ToHealThisTick;

	if (HealAmount <= 0.f)
	{
		// Healing is complete: Reset HoT properties
		HealAmount = 0.f;
		HealingRate = 0.f;
		bHealing = false;
	}
}

void UBuffComponent::ShieldRampUp(float DeltaTime)
{
	if (!bReplenishingShield || !Character || Character->IsElimmed()) return;

	float ToReplenishThisTick = ShieldReplenishRate * DeltaTime;
	if (ShieldReplenishAmount - ToReplenishThisTick < 0.f)
	{
		ToReplenishThisTick = ShieldReplenishAmount;
	}

	Character->ReplenishShield(ToReplenishThisTick);

	ShieldReplenishAmount -= ToReplenishThisTick;

	if (ShieldReplenishAmount <= 0.f)
	{
		// Shield replenishment is complete
		ShieldReplenishAmount = 0.f;
		ShieldReplenishRate = 0.f;
		bReplenishingShield = false;
	}
}

void UBuffComponent::IncreaseSpeedFor(float Percent, float Time)
{
	if ((Character && Character->IsElimmed()) || Time == 0.f) return;

	Character->UpdateMovementSpeedByPercent(Percent);
	MulticastSpeedBuff(Percent);
	Character->GetWorldTimerManager().SetTimer(SpeedTimerHandle, this, &UBuffComponent::ResetMovementSpeeds, Time);
}

void UBuffComponent::ResetMovementSpeeds()
{
	if ((Character && Character->IsElimmed())) return;

	Character->UpdateMovementSpeedByPercent(1.f);
	MulticastSpeedBuff(1.f);
}

void UBuffComponent::MulticastSpeedBuff_Implementation(float Percent)
{
	if (!Character) return;
	Character->UpdateMovementSpeedByPercent(Percent);
}

void UBuffComponent::UpdateJumpVelocityFor(float Percent, float Time)
{
	if ((Character && Character->IsElimmed()) || Time == 0.f) return;

	Character->UpdateJumpVelocityByPercent(Percent);
	MulticastJumpBuff(Percent);
	Character->GetWorldTimerManager().SetTimer(JumpTimerHandle, this, &UBuffComponent::ResetJumpVelocity, Time);
}

void UBuffComponent::ResetJumpVelocity()
{
	if ((Character && Character->IsElimmed())) return;

	Character->UpdateJumpVelocityByPercent(1.f);
	MulticastJumpBuff(1.f);
}

void UBuffComponent::MulticastJumpBuff_Implementation(float Percent)
{
	if (!Character) return;

	Character->UpdateJumpVelocityByPercent(Percent);
}

