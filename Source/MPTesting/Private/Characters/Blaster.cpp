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
#include "Kismet/GameplayStatics.h"
#include "Characters/BlasterAnimInstance.h"
#include "MPTesting/MPTesting.h"
#include "Controller/BlasterController.h"
#include "GameMode/BlasterGameMode.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "PlayerStates/BlasterPlayerState.h"
#include "Enums/WeaponTypes.h"


ABlaster::ABlaster()
{
	PrimaryActorTick.bCanEverTick = true;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);

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

	CombatComponent2 = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent2"));
	CombatComponent2->SetIsReplicated(true);

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));
}

void ABlaster::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlaster, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlaster, Health);
	DOREPLIFETIME(ABlaster, Stamina);
	DOREPLIFETIME(ABlaster, bDisableGameplay);
}

void ABlaster::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (CombatComponent2)
	{
		CombatComponent2->Character = this;
	}
}

bool ABlaster::IsWeaponEquipped() const
{
	return (CombatComponent2 && CombatComponent2->EquippedWeapon);
}

bool ABlaster::IsAiming()
{
	return (CombatComponent2 && CombatComponent2->bAiming);
}

AWeapon* ABlaster::GetEquippedWeapon()
{
	return CombatComponent2 ? CombatComponent2->EquippedWeapon : nullptr;
}

void ABlaster::PlayFireMontage(bool bAiming)
{
	if (!CombatComponent2 || !CombatComponent2->EquippedWeapon) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName = bAiming ? FName("FireIronsights") : FName("FireHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlaster::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void ABlaster::PlayReloadMontage()
{
	if (!CombatComponent2 || !CombatComponent2->EquippedWeapon) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		switch (CombatComponent2->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

FVector ABlaster::GetHitTarget() const
{
	if (!CombatComponent2) return FVector();
	return CombatComponent2->HitTarget;
}

void ABlaster::PlayHitReactMontage()
{
	// NOTE! IF THE PLAYER HIT IS NOT CARRYING A WEAPON THIS WILL NOT BE PLAYED
	if (!CombatComponent2 || !CombatComponent2->EquippedWeapon) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName = FName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlaster::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void ABlaster::Destroyed()
{
	Super::Destroyed();

	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}

	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	bool bMatchNotInProgress = GameMode && GameMode->GetMatchState() != MatchState::InProgress;
	if (CombatComponent2 && CombatComponent2->EquippedWeapon && bMatchNotInProgress)
	{
		CombatComponent2->EquippedWeapon->Destroy();
	}
}

void ABlaster::SetHUDHealth()
{
	if (IsLocallyControlled() && BlasterController)	BlasterController->SetHUDHealth(Health, MaxHealth);
}

ECombatState ABlaster::GetCombatState() const
{
	if (!CombatComponent2) return ECombatState::ECS_Unoccupied;
	return CombatComponent2->CombatState;
}

void ABlaster::Elim()
{
	if (CombatComponent2 && CombatComponent2->EquippedWeapon)
	{
		CombatComponent2->EquippedWeapon->Dropped();
	}

	MulticastElim();
	GetWorldTimerManager().SetTimer(ElimTimer, this, &ABlaster::ElimTimerFinished, ElimDelay);
}

void ABlaster::MulticastElim_Implementation()
{
	bElimmed = true;
	PlayElimMontage();

	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	bDisableGameplay = true;
	if (CombatComponent2)
	{
		CombatComponent2->FireButtonPressed(false);
	}

	if (BlasterController)
	{
		BlasterController->SetHUDWeaponAmmo(0);
		//DisableInput(BlasterController);
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 100.f);
	}
	StartDissolve();

	if (ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ElimBotEffect, ElimBotSpawnPoint, GetActorRotation());
	}

	if (ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, ElimBotSound, GetActorLocation());
	}
}

void ABlaster::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (IsLocallyControlled() && OverlappingWeapon)
	{
		OverlappingWeapon->SetPickupWidgetVisibility(false);
	}

	//UE_LOG(LogTemp, Warning, TEXT("Setting OverlappingWeapon"));
	OverlappingWeapon = Weapon;
	
	if (IsLocallyControlled() && OverlappingWeapon)
	{
		OverlappingWeapon->SetPickupWidgetVisibility(true);
	}
}

void ABlaster::BeginPlay()
{
	Super::BeginPlay();
	
	BlasterController = Cast<ABlasterController>(GetController());

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABlaster::ReceiveDamage);
	}

	GetWorldTimerManager().SetTimer(AfterBeginPlayTimer, this, &ABlaster::AfterBeginPlay, AfterBeginPlayTime);
}

void ABlaster::Move(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;

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
	if (bDisableGameplay) return;
}

void ABlaster::EndSprint(const FInputActionValue& Value)
{
	if (bDisableGameplay) return;
}

void ABlaster::CrouchButtonPressed()
{
	if (bDisableGameplay) return;

	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ABlaster::Jump()
{
	if (bDisableGameplay) return;

	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}
void ABlaster::AimButtonPressed()
{
	if (bDisableGameplay) return;

	check(CombatComponent2);

	CombatComponent2->SetAiming(true);
}

void ABlaster::AimButtonReleased()
{
	if (bDisableGameplay) return;

	check(CombatComponent2);

	CombatComponent2->SetAiming(false);
}

void ABlaster::FireButtonPressed()
{
	if (bDisableGameplay) return;

	check(CombatComponent2);

	CombatComponent2->FireButtonPressed(true);
}

void ABlaster::FireButtonReleased()
{
	if (bDisableGameplay) return;

	check(CombatComponent2);

	CombatComponent2->FireButtonPressed(false);
}

/*void ABlaster::Crouch(bool bClientSimulation)
{
}*/

void ABlaster::Equip()
{
	if (bDisableGameplay) return;

	check(CombatComponent2);

	if (HasAuthority())
	{
		CombatComponent2->EquipWeapon(OverlappingWeapon);
	}
	else
	{
		ServerEquipButtonPressed();
	}
}

void ABlaster::ReloadButtonPressed()
{
	if (bDisableGameplay) return;

	if (CombatComponent2) CombatComponent2->Reload();
}

void ABlaster::AimOffset(float DeltaTime)
{
	if (CombatComponent2 && CombatComponent2->EquippedWeapon == nullptr) return;

	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) // standing still & not jumping
	{
		bRotateRootBone = true;
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
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}

void ABlaster::CalculateAO_Pitch()
{
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

void ABlaster::SimProxiesTurn()
{
	if (!CombatComponent2 || !CombatComponent2->EquippedWeapon) return;

	bRotateRootBone = false;
	if (float Speed = CalculateSpeed() > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	PreviousProxyRotation = CurrentProxyRotation;
	CurrentProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(CurrentProxyRotation, PreviousProxyRotation).Yaw;

	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void ABlaster::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	if (IsLocallyControlled() && BlasterController)	BlasterController->SetHUDHealth(Health, MaxHealth);

	if (Health == 0.f)
	{
		ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
		if (BlasterGameMode)
		{
			ABlasterController* AttackerController = Cast<ABlasterController>(InstigatorController);
			if (!BlasterController)
			{
				//UE_LOG(LogTemp, Warning, TEXT("In Blaster::ReceiveDamage(): BlasterController is null - casting"));
				BlasterController = Cast<ABlasterController>(GetController());
			}
			BlasterGameMode->PlayerEliminated(this, BlasterController, AttackerController);
		}
	}
	else
	{
		PlayHitReactMontage();
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

void ABlaster::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled()) return;
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (CombatComponent2 && CombatComponent2->EquippedWeapon && CombatComponent2->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent2->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (CombatComponent2 && CombatComponent2->EquippedWeapon && CombatComponent2->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent2->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

float ABlaster::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void ABlaster::ElimTimerFinished()
{
	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	if (BlasterGameMode)
	{
		BlasterGameMode->RequestRespawn(this, Controller);
	}

	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}
}

void ABlaster::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ABlaster::UpdateDissolveMaterial);

	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void ABlaster::AfterBeginPlay()
{
	if (!BlasterController)
	{
		BlasterController = Cast<ABlasterController>(GetController());
	}

	if (BlasterController)
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(BlasterController->GetLocalPlayer());
		if (Subsystem)
		{
			Subsystem->AddMappingContext(CharMappingContext, 0);
		}

		BlasterController->SetHUDHealth(Health, MaxHealth);
	}

	BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
	if (BlasterPlayerState)
	{
		BlasterPlayerState->AddToScore(0.f);

		//UE_LOG(LogTemp, Warning, TEXT("In Blaster calling AddToDefeats(0)"));
		BlasterPlayerState->AddToDefeats(0);
	}
}

void ABlaster::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}

}

void ABlaster::OnRep_Health()
{
	if (IsLocallyControlled() && BlasterController)
	{
		BlasterController->SetHUDHealth(Health, MaxHealth);
	}
	PlayHitReactMontage();
}

void ABlaster::OnRep_Stamina()
{
}

void ABlaster::ServerEquipButtonPressed_Implementation()
{
	check(CombatComponent2);

	CombatComponent2->EquipWeapon(OverlappingWeapon);
}

void ABlaster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime);
	
	HideCameraIfCharacterClose();
}

void ABlaster::RotateInPlace(float DeltaTime)
{
	if (bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
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
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &ABlaster::ReloadButtonPressed);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ABlaster::CrouchButtonPressed);

		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ABlaster::Sprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ABlaster::EndSprint);

		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ABlaster::AimButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ABlaster::AimButtonReleased);

		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ABlaster::FireButtonPressed);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ABlaster::FireButtonReleased);
	}

}


