// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Blaster.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Weapons/Weapon.h"
#include "BlasterComponents/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"


ABlaster::ABlaster()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 850.f, 0.f);
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	AutoPossessPlayer = EAutoReceiveInput::Player0;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 2.f;

	DisplayNameWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("DisplayNameWidget"));
	DisplayNameWidget->SetupAttachment(GetRootComponent());

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);

}

void ABlaster::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlaster, OverlappingWeapon, COND_OwnerOnly);
}

void ABlaster::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	check(CombatComponent);
	CombatComponent->Character = this;

}

bool ABlaster::IsWeaponEquipped() const
{
	return CombatComponent->EquippedWeapon != nullptr;
}

bool ABlaster::IsAiming()
{
	return (CombatComponent && CombatComponent->bAiming);
}

AWeapon* ABlaster::GetEquippedWeapon()
{
	return CombatComponent ? CombatComponent->EquippedWeapon : nullptr;
}

void ABlaster::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (IsLocallyControlled() && OverlappingWeapon)
	{
		OverlappingWeapon->SetPickupWidgetVisibility(false);
	}

	UE_LOG(LogTemp, Warning, TEXT("Setting OverlappingWeapon"));
	OverlappingWeapon = Weapon;
	
	if (IsLocallyControlled() && OverlappingWeapon)
	{
		OverlappingWeapon->SetPickupWidgetVisibility(true);
	}
}

void ABlaster::BeginPlay()
{
	Super::BeginPlay();
	
	PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if (Subsystem)
		{
			Subsystem->AddMappingContext(CharMappingContext, 0);
		}
	}
}

void ABlaster::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	AddMovementInput(ForwardDirection, MovementVector.X);
	AddMovementInput(RightDirection, MovementVector.Y);
}

void ABlaster::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisValue = Value.Get<FVector2D>();
	AddControllerYawInput(LookAxisValue.X);
	AddControllerPitchInput(LookAxisValue.Y);
}

void ABlaster::Sprint(const FInputActionValue& Value)
{
}

void ABlaster::EndSprint(const FInputActionValue& Value)
{
}

void ABlaster::CrouchButtonPressed()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ABlaster::AimButtonPressed()
{
	check(CombatComponent);

	CombatComponent->SetAiming(true);
}

void ABlaster::AimButtonReleased()
{
	check(CombatComponent);

	CombatComponent->SetAiming(false);
}

/*void ABlaster::Crouch(bool bClientSimulation)
{
}*/

void ABlaster::Equip()
{
	check(CombatComponent);

	if (HasAuthority())
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);
	}
	else
	{
		ServerEquipButtonPressed();
	}
}

void ABlaster::AimOffset(float DeltaTime)
{
	if (CombatComponent && CombatComponent->EquippedWeapon == nullptr) return;

	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) // standing still & not jumping
	{
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir)
	{
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	AO_Pitch = GetBaseAimRotation().Pitch;

	// When sent across the network, Rotation is compressed to a value between 0 and 360
	// This puts the value back to between -90.f and 90.f
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// Map pitch from [270, 360] to [-90, 0]
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void ABlaster::OnRep_OverlappingWeapon(AWeapon* PreviousWeapon)
{
	if (PreviousWeapon)
	{
		PreviousWeapon->SetPickupWidgetVisibility(false);
	}
	if (OverlappingWeapon)
	{
		OverlappingWeapon->SetPickupWidgetVisibility(true);
	}
}

void ABlaster::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}

	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void ABlaster::ServerEquipButtonPressed_Implementation()
{
	check(CombatComponent);

	CombatComponent->EquipWeapon(OverlappingWeapon);
}

void ABlaster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimOffset(DeltaTime);
}

void ABlaster::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABlaster::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABlaster::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ABlaster::Jump);
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Started, this, &ABlaster::Equip);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ABlaster::CrouchButtonPressed);

		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ABlaster::Sprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ABlaster::EndSprint);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ABlaster::AimButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ABlaster::AimButtonReleased);
	}

}

void ABlaster::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

