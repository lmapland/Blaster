// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/BlasterAnimInstance.h"
#include "Characters/Blaster.h"
#include "GameFramework/CharacterMovementComponent.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Blaster = Cast<ABlaster>(TryGetPawnOwner());
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (Blaster == nullptr)
	{
		Blaster = Cast<ABlaster>(TryGetPawnOwner());
	}

	if (Blaster == nullptr) return;

	FVector Velocity = Blaster->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = Blaster->GetCharacterMovement()->IsFalling();
	bIsAccelerating = Blaster->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f;
	bWeaponEquipped = Blaster->IsWeaponEquipped();
	bIsCrouched = Blaster->bIsCrouched;
}
