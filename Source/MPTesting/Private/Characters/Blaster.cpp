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

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	DisplayNameWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("DisplayNameWidget"));
	DisplayNameWidget->SetupAttachment(GetRootComponent());

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
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

void ABlaster::ServerEquipButtonPressed_Implementation()
{
	check(CombatComponent);

	CombatComponent->EquipWeapon(OverlappingWeapon);
}

void ABlaster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABlaster::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABlaster::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABlaster::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ABlaster::Jump);
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &ABlaster::Equip);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ABlaster::Sprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ABlaster::EndSprint);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ABlaster::CrouchButtonPressed);
	}

}

void ABlaster::Jump()
{
	Super::Jump();
}

