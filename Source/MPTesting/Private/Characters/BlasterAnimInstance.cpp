// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/BlasterAnimInstance.h"
#include "Characters/Blaster.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapons/Weapon.h"
#include "Enums/CombatState.h"

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
	bAiming = Blaster->IsAiming();
	bRotateRootBone = Blaster->ShouldRotateRootBone();
	bElimmed = Blaster->IsElimmed();
	bUseFABRIK = Blaster->GetCombatState() == ECombatState::ECS_Unoccupied;
	if (Blaster->IsLocallyControlled() && Blaster->GetCombatState() != ECombatState::ECS_ThrowingGrenade && Blaster->GetCombatState() != ECombatState::ECS_Swapping)
	{
		bUseFABRIK = !Blaster->IsLocallyReloading() && Blaster->bFinishedSwapping;
	}
	bUseAimOffsets = Blaster->GetCombatState() == ECombatState::ECS_Unoccupied && !Blaster->GameplayIsDisabled();
	bTransformRightHand = Blaster->GetCombatState() == ECombatState::ECS_Unoccupied && !Blaster->GameplayIsDisabled();

	EquippedWeapon = Blaster->GetEquippedWeapon();
	TurningInPlace = Blaster->GetTurningInPlace();

	FRotator AimRotation = Blaster->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(Blaster->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;

	PreviousRotation = CurrentRotation;
	CurrentRotation = Blaster->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CurrentRotation, PreviousRotation);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);
	//UE_LOG(LogTemp, Warning, TEXT("AimRotation Yaw: %f"), AimRotation.Yaw);

	AO_Yaw = Blaster->GetAOYaw();
	AO_Pitch = Blaster->GetAOPitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && Blaster->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		Blaster->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if (Blaster->IsLocallyControlled())
		{
			bLocallyControlled = true;
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Hand_R"), ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - Blaster->GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);
		}
	}
}
