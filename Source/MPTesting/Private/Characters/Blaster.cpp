// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Blaster.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"
#include "Characters/BlasterAnimInstance.h"
#include "MPTesting/MPTesting.h"
#include "Controller/BlasterController.h"
#include "GameMode/BlasterGameMode.h"
#include "PlayerStates/BlasterPlayerState.h"
#include "GameStates/BlasterGameState.h"
#include "Enums/WeaponTypes.h"
#include "BlasterComponents/CombatComponent.h"
#include "BlasterComponents/BuffComponent.h"
#include "BlasterComponents/LagCompensationComponent.h"
#include "Weapons/Weapon.h"


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

	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HandGrenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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

	BuffComponent2 = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	BuffComponent2->SetIsReplicated(true);

	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	/* Hitboxes for server-side rewind */
	Head = CreateDefaultSubobject<UBoxComponent>(TEXT("HeadBox"));
	Head->SetupAttachment(GetMesh(), FName("head"));
	HitCollisionBoxes.Add(FName("head"), Head);

	Pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("PelvisBox"));
	Pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitCollisionBoxes.Add(FName("pelvis"), Pelvis);

	Spine02 = CreateDefaultSubobject<UBoxComponent>(TEXT("Spine02Box"));
	Spine02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxes.Add(FName("spine_02"), Spine02);

	Spine03 = CreateDefaultSubobject<UBoxComponent>(TEXT("Spine03Box"));
	Spine03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxes.Add(FName("spine_03"), Spine03);

	UpperArmL = CreateDefaultSubobject<UBoxComponent>(TEXT("UpperArmLBox"));
	UpperArmL->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitCollisionBoxes.Add(FName("upperarm_l"), UpperArmL);

	LowerArmL = CreateDefaultSubobject<UBoxComponent>(TEXT("LowerArmLBox"));
	LowerArmL->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitCollisionBoxes.Add(FName("lowerarm_l"), LowerArmL);

	HandL = CreateDefaultSubobject<UBoxComponent>(TEXT("HandLBox"));
	HandL->SetupAttachment(GetMesh(), FName("hand_l"));
	HitCollisionBoxes.Add(FName("hand_l"), HandL);

	UpperArmR = CreateDefaultSubobject<UBoxComponent>(TEXT("UpperArmRBox"));
	UpperArmR->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitCollisionBoxes.Add(FName("upperarm_r"), UpperArmR);

	LowerArmR = CreateDefaultSubobject<UBoxComponent>(TEXT("LowerArmRBox"));
	LowerArmR->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitCollisionBoxes.Add(FName("lowerarm_r"), LowerArmR);

	HandR = CreateDefaultSubobject<UBoxComponent>(TEXT("HandRBox"));
	HandR->SetupAttachment(GetMesh(), FName("hand_r"));
	HitCollisionBoxes.Add(FName("hand_r"), HandR);

	Backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("BackpackBox"));
	Backpack->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("backpack"), Backpack);

	Blanket = CreateDefaultSubobject<UBoxComponent>(TEXT("BlanketBox"));
	Blanket->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("blanket"), Blanket);

	ThighL = CreateDefaultSubobject<UBoxComponent>(TEXT("ThighLBox"));
	ThighL->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitCollisionBoxes.Add(FName("thigh_l"), ThighL);

	CalfL = CreateDefaultSubobject<UBoxComponent>(TEXT("CalfL"));
	CalfL->SetupAttachment(GetMesh(), FName("calf_l"));
	HitCollisionBoxes.Add(FName("calf_l"), CalfL);

	FootL = CreateDefaultSubobject<UBoxComponent>(TEXT("FootLBox"));
	FootL->SetupAttachment(GetMesh(), FName("foot_l"));
	HitCollisionBoxes.Add(FName("foot_l"), FootL);

	ThighR = CreateDefaultSubobject<UBoxComponent>(TEXT("ThighRBox"));
	ThighR->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitCollisionBoxes.Add(FName("thigh_r"), ThighR);

	CalfR = CreateDefaultSubobject<UBoxComponent>(TEXT("CalfRBox"));
	CalfR->SetupAttachment(GetMesh(), FName("calf_r"));
	HitCollisionBoxes.Add(FName("calf_r"), CalfR);

	FootR = CreateDefaultSubobject<UBoxComponent>(TEXT("FootRBox"));
	FootR->SetupAttachment(GetMesh(), FName("foot_r"));
	HitCollisionBoxes.Add(FName("foot_r"), FootR);

	for (auto Box : HitCollisionBoxes)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ABlaster::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlaster, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlaster, Health);
	DOREPLIFETIME(ABlaster, Shield);
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

	if (BuffComponent2)
	{
		BuffComponent2->Character = this;
	}

	if (LagCompensation)
	{
		LagCompensation->Character = this;
		if (Controller)
		{
			LagCompensation->Controller = BlasterController? BlasterController : Cast<ABlasterController>(Controller);
		}
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
	//if (HasAuthority()) UE_LOG(LogTemp, Warning, TEXT("PlayReloadMontage(): About to play reload montage on authoritative character"));

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
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("RocketLauncher");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName("Pistol"); // Pistol reload animation works for SMG as well
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Shotgun");
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("SniperRifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("GrenadeLauncher");
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
		AnimInstance->OnMontageEnded.AddDynamic(this, &ABlaster::ReloadMontageEndedHandler);
	}
}

void ABlaster::ReloadMontageEndedHandler(UAnimMontage* Montage, bool bInterrupted)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance || !CombatComponent2) return;

	if (bInterrupted)
	{
		//UE_LOG(LogTemp, Warning, TEXT("ReloadMontageEndedHandler(): Montage was interrupted"));
		if (HasAuthority()) UE_LOG(LogTemp, Warning, TEXT("ReloadMontageEndedHandler(): Character has authority"));
		/* If the character got interrupted in the middle of their reload
		 * we want to handle that. We do so by determining how far along they were 
		 * in the animation and wait for the appropriate amount of time before calling
		 * FinishReloading() on the CombatComponent */

		float OutStartTime;
		float OutEndTime;
		ReloadMontage->GetSectionStartAndEndTime(ReloadMontage->GetSectionIndex(AnimInstance->Montage_GetCurrentSection()), OutStartTime, OutEndTime);

		float TimeToWait = FMath::Clamp(OutEndTime - AnimInstance->Montage_GetPosition(ReloadMontage), 0, OutEndTime - OutStartTime);
		//UE_LOG(LogTemp, Warning, TEXT("ReloadMontageEndedHandler(): TimeToWait: %f"), TimeToWait);

		// If we get interrupted at the end of our reload sequence, there is no need to start a timer
		if (TimeToWait > 0.f)
		{
			GetWorldTimerManager().SetTimer(ReloadTimer, this, &ABlaster::ReloadTimerFinished, TimeToWait);
			AnimInstance->OnMontageEnded.RemoveDynamic(this, &ABlaster::ReloadMontageEndedHandler);
			return;
		}
		else if (CombatComponent2->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun)
		{
			JumpToShotgunReloadEnd();
		}
	}

	CombatComponent2->FinishReloading();
	AnimInstance->OnMontageEnded.RemoveDynamic(this, &ABlaster::ReloadMontageEndedHandler);
}

void ABlaster::ReloadTimerFinished()
{
	if (!CombatComponent2) return;

	CombatComponent2->FinishReloading();
}

void ABlaster::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
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
	// The last check will prevent the hit react montage from being played if the character is, for example, reloading
	if (AnimInstance && HitReactMontage && !AnimInstance->IsAnyMontagePlaying())
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName = FName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlaster::PlaySwapMontage()
{
	if (!CombatComponent2 || !CombatComponent2->EquippedWeapon) return;

	// Play the montage and also set two timers, one for the middle where the character should do the weapon swapping and one at the end
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && SwapWeaponMontage)
	{
		AnimInstance->Montage_Play(SwapWeaponMontage);
	}

	float OutStartTime;
	float OutEndTime;
	SwapWeaponMontage->GetSectionStartAndEndTime(SwapWeaponMontage->GetSectionIndex(AnimInstance->Montage_GetCurrentSection()), OutStartTime, OutEndTime);
	float TimeToWait = FMath::Clamp(OutEndTime - AnimInstance->Montage_GetPosition(SwapWeaponMontage), 0, OutEndTime - OutStartTime);

	GetWorldTimerManager().SetTimer(SwapAttachedWepsTimer, this, &ABlaster::SwapAttachedTimerFinished, TimeToWait / 2);
	GetWorldTimerManager().SetTimer(SwapFinishedTimer, this, &ABlaster::SwapTimerFinished, TimeToWait);
	
}

void ABlaster::SwapAttachedTimerFinished()
{
	if (!CombatComponent2) return;

	CombatComponent2->FinishSwapAttachWeapons();
}

void ABlaster::SwapTimerFinished()
{
	if (!CombatComponent2) return;

	CombatComponent2->FinishSwapping();
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

	BlasterGameMode = BlasterGameMode ? BlasterGameMode : GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	//BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	bool bMatchNotInProgress = BlasterGameMode && BlasterGameMode->GetMatchState() != MatchState::InProgress;
	if (CombatComponent2 && CombatComponent2->EquippedWeapon && bMatchNotInProgress)
	{
		CombatComponent2->EquippedWeapon->Destroy();
	}
}

void ABlaster::SetHUDHealth()
{
	if (IsLocallyControlled() && BlasterController)	BlasterController->SetHUDHealth(Health, MaxHealth);
}

void ABlaster::SetHUDShield()
{
	if (IsLocallyControlled() && BlasterController)	BlasterController->SetHUDShield(Shield, MaxShield);
}

ECombatState ABlaster::GetCombatState() const
{
	if (!CombatComponent2) return ECombatState::ECS_Unoccupied;
	return CombatComponent2->CombatState;
}

void ABlaster::JumpToShotgunReloadEnd()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage && AnimInstance->Montage_IsPlaying(ReloadMontage))
	{
		AnimInstance->Montage_JumpToSection("ShotgunEnd");
	}
}

void ABlaster::UpdateMovementSpeedByPercent(float Percent)
{
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed * Percent;
	GetCharacterMovement()->MaxWalkSpeedCrouched = BaseCrouchSpeed * Percent;
}

void ABlaster::UpdateJumpVelocityByPercent(float Percent)
{
	GetCharacterMovement()->JumpZVelocity = BaseJumpVelocity * Percent;
}

void ABlaster::Elim(bool bPlayerLeftGame)
{
	if (CombatComponent2)
	{
		if (CombatComponent2->EquippedWeapon) CombatComponent2->EquippedWeapon->Dropped();
		if (CombatComponent2->SecondaryWeapon) CombatComponent2->SecondaryWeapon->Dropped();
	}

	MulticastElim(bPlayerLeftGame);
}

void ABlaster::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;
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

	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}

	if (IsLocallyControlled() && CombatComponent2 && CombatComponent2->bAiming && CombatComponent2->EquippedWeapon && CombatComponent2->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		ShowSniperScope(false);
	}

	GetWorldTimerManager().SetTimer(ElimTimer, this, &ABlaster::ElimTimerFinished, ElimDelay);
}

void ABlaster::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (IsLocallyControlled() && OverlappingWeapon)
	{
		OverlappingWeapon->SetPickupWidgetVisibility(false);
	}

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

	SetAttachedGrenadeVisibility(false);

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

	ServerEquipButtonPressed();
	if (CombatComponent2->ShouldSwapWeapons() && !HasAuthority() && !OverlappingWeapon)
	{
		PlaySwapMontage();
		CombatComponent2->CombatState = ECombatState::ECS_Swapping;
		bFinishedSwapping = false;
	}
}

void ABlaster::ServerEquipButtonPressed_Implementation()
{
	check(CombatComponent2);

	if (OverlappingWeapon) CombatComponent2->EquipWeapon(OverlappingWeapon);
	else CombatComponent2->SwapWeapons();
}

void ABlaster::ReloadButtonPressed()
{
	if (bDisableGameplay) return;

	if (CombatComponent2) CombatComponent2->Reload();
}

void ABlaster::GrenadeButtonPressed()
{
	if (CombatComponent2)
	{
		CombatComponent2->ThrowGrenade();
	}
}

void ABlaster::SetAttachedGrenadeVisibility(bool bIsVisible)
{
	if (AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(bIsVisible);
	}
}

FVector ABlaster::GetGrenadeLocation()
{
	if (AttachedGrenade)
	{
		return AttachedGrenade->GetComponentLocation();
	}
	return FVector();
}

bool ABlaster::IsLocallyReloading()
{
	if (!CombatComponent2) return false;
	else return CombatComponent2->IsLocallyReloading();
}

void ABlaster::SetTeamColor(ETeam InTeam)
{
	if (!GetMesh() || !DefaultMaterial || !BlueMaterial || !RedMaterial || !BlueDissolveMatInstance || !RedDissolveMatInstance) return;

	switch (InTeam)
	{
	case ETeam::ET_NoTeam:
		GetMesh()->SetMaterial(0, DefaultMaterial);
		DissolveMaterialInstance = BlueDissolveMatInstance;
		break;
	case ETeam::ET_BlueTeam:
		GetMesh()->SetMaterial(0, BlueMaterial);
		DissolveMaterialInstance = BlueDissolveMatInstance;
		break;
	case ETeam::ET_RedTeam:
		GetMesh()->SetMaterial(0, RedMaterial);
		DissolveMaterialInstance = RedDissolveMatInstance;
		break;
	}
}

void ABlaster::MulticastGainedTheLead_Implementation()
{
	if (CrownSystem && !CrownComponent)
	{
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(CrownSystem, GetCapsuleComponent(), NAME_None, GetActorLocation() + FVector(0.f, 0.f, 110.f), GetActorRotation(), EAttachLocation::KeepWorldPosition, false);
	}
	if (CrownComponent)
	{
		CrownComponent->Activate();
	}
}

void ABlaster::MulticastLostTheLead_Implementation()
{
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
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
	BlasterGameMode = BlasterGameMode ? BlasterGameMode : GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	if (bElimmed || !BlasterGameMode || !GetController()) return;
	if (!BlasterGameMode->PlayerShouldDamage(InstigatorController, GetController())) return;

	HandleDamage(Damage);

	if (Health == 0.f) // Kill the player
	{
		ABlasterController* AttackerController = Cast<ABlasterController>(InstigatorController);
		if (!BlasterController)
		{
			BlasterController = Cast<ABlasterController>(GetController());
		}
		BlasterGameMode->PlayerEliminated(this, BlasterController, AttackerController);
	}
	else
	{
		PlayHitReactMontage();
	}
}

void ABlaster::HandleDamage(float Damage)
{
	//UE_LOG(LogTemp, Warning, TEXT("HandleDamage(): (Pre) Damage: %f, Shield: %f, Health: %f"), Damage, Shield, Health);
	if (Damage >= Shield + Health)
	{
		Shield = 0.f;
		Health = 0.f;
	}
	else if (Damage >= Shield)
	{
		Health = FMath::Clamp(Health - (Damage - Shield), 0.f, MaxHealth);
		Shield = 0.f;
	}
	else // Damage < Shield
	{
		Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield);
	}

	SetHUDHealth();
	SetHUDShield();
}

void ABlaster::UpdateHealth(float Amount)
{
	//UE_LOG(LogTemp, Warning, TEXT("HandleHeal(): (Pre) Amount: %f, Shield: %f, Health: %f"), Amount, Shield, Health);
	if (Amount > (MaxHealth - Health) + (MaxShield - Shield))
	{
		Health = MaxHealth;
		Shield = MaxShield;
	}
	else if (Amount >= (MaxHealth - Health))
	{
		Shield = FMath::Clamp(Shield + (Amount - (MaxHealth - Health)), 0.f, MaxShield);
		Health = MaxHealth;
	}
	else // Amount < (MaxHealth - Health)
	{
		Health = FMath::Clamp(Health + Amount, 0.f, MaxHealth);
	}

	SetHUDHealth();
	SetHUDShield();
}

void ABlaster::ReplenishShield(float Amount)
{
	Shield = FMath::Clamp(Shield + Amount, 0.f, MaxShield);

	SetHUDShield();
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
		if (CombatComponent2 && CombatComponent2->SecondaryWeapon && CombatComponent2->SecondaryWeapon->GetWeaponMesh())
		{
			CombatComponent2->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (CombatComponent2 && CombatComponent2->EquippedWeapon && CombatComponent2->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent2->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
		if (CombatComponent2 && CombatComponent2->SecondaryWeapon && CombatComponent2->SecondaryWeapon->GetWeaponMesh())
		{
			CombatComponent2->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = false;
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
	BlasterGameMode = BlasterGameMode ? BlasterGameMode : GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	if (BlasterGameMode && !bLeftGame)
	{
		BlasterGameMode->RequestRespawn(this, Controller);
	}

	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}

	if (bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
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

	if (BlasterController && CombatComponent2)
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(BlasterController->GetLocalPlayer());
		if (Subsystem)
		{
			Subsystem->AddMappingContext(CharMappingContext, 0);
		}

		SetHUDHealth();
		SetHUDShield();
		SpawnDefaultWeapon();
		BlasterController->SetHUDCarriedAmmo(CombatComponent2->CarriedAmmo);
		BlasterController->SetHUDGrenades(CombatComponent2->GetGrenadeCount());

		if (CombatComponent2->EquippedWeapon) BlasterController->SetHUDWeaponAmmo(CombatComponent2->EquippedWeapon->GetAmmo());
	}

	BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
	if (BlasterPlayerState)
	{
		BlasterPlayerState->AddToScore(0.f);
		BlasterPlayerState->AddToDefeats(0);
		SetTeamColor(BlasterPlayerState->GetTeam());
	}

	ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if (BlasterGameState && BlasterGameState->IsTopScoringPlayer(BlasterPlayerState))
	{
		MulticastGainedTheLead();
	}

	if (GetMovementComponent())
	{
		BaseWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
		BaseCrouchSpeed = GetCharacterMovement()->MaxWalkSpeedCrouched;
		BaseJumpVelocity = GetCharacterMovement()->JumpZVelocity;
	}
}

void ABlaster::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void ABlaster::SpawnDefaultWeapon()
{
	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	if (DefaultWeaponClass && GameMode && !bElimmed)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AWeapon* StartingWeapon = GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass, SpawnParams);
		if (StartingWeapon && CombatComponent2)
		{
			CombatComponent2->EquipWeapon(StartingWeapon);
			StartingWeapon->bDestroyWeapon = true;
		}
	}
}

void ABlaster::OnRep_Health(float PreviousHealth)
{
	SetHUDHealth();

	if (Health < PreviousHealth) PlayHitReactMontage();
}

void ABlaster::OnRep_Shield(float PreviousShield)
{
	SetHUDShield();

	if (Shield < PreviousShield) PlayHitReactMontage();
}

void ABlaster::OnRep_Stamina()
{
}

void ABlaster::ServerLeaveGame_Implementation()
{
	BlasterPlayerState = !BlasterPlayerState ? GetPlayerState<ABlasterPlayerState>() : BlasterPlayerState;
	BlasterGameMode = BlasterGameMode ? BlasterGameMode : GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	if (BlasterPlayerState && BlasterGameMode)
	{
		BlasterGameMode->PlayerLeftGame(BlasterPlayerState);
	}
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
		EnhancedInputComponent->BindAction(ThrowGrenadeAction, ETriggerEvent::Started, this, &ABlaster::GrenadeButtonPressed);

		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ABlaster::Sprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ABlaster::EndSprint);

		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ABlaster::AimButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ABlaster::AimButtonReleased);

		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ABlaster::FireButtonPressed);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ABlaster::FireButtonReleased);
	}

}


