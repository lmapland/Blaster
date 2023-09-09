// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterComponents/CombatComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Characters/Blaster.h"
#include "Controller/BlasterController.h"
#include "HUD/BlasterHUD.h"
#include "Weapons/Weapon.h"
#include "Weapons/Projectile.h"
#include "Weapons/Shotgun.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UCombatComponent::PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount)
{
	if (!Character) return;

	if (CarriedAmmoMap.Contains(WeaponType))
	{
		CarriedAmmoMap[WeaponType] = FMath::Clamp(CarriedAmmoMap[WeaponType] + AmmoAmount, 0, MaxCarriedAmmo);
		UpdateCarriedAmmo();
	}
	else
	{
		CarriedAmmoMap.Emplace(WeaponType, AmmoAmount);
	}

	if (EquippedWeapon && EquippedWeapon->IsEmpty() && EquippedWeapon->GetWeaponType() == WeaponType)
	{
		Reload();
	}

	Controller = Controller == nullptr ? Character->GetBlasterController() : Controller;
	if (Controller) Controller->SetHUDCarriedAmmo(CarriedAmmo);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

		if (Character->GetFollowCamera())
		{
			CurrentFOV = DefaultFOV = Character->GetFollowCamera()->FieldOfView;
		}

		if (Character->HasAuthority())
		{
			InitializeCarriedAmmo();
		}
	}
}

void UCombatComponent::SetAiming(bool bInAiming)
{
	if (!Character || !EquippedWeapon) return;

	bAiming = bInAiming;
	ServerSetAiming(bInAiming);
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bAiming ? AimWalkSpeed : BaseWalkSpeed;

		if (Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
		{
			Character->ShowSniperScope(bInAiming);
		}
	}
	if (Character->IsLocallyControlled()) bAimButtonPressed = bAiming;
}

void UCombatComponent::ServerSetAiming_Implementation(bool bInAiming)
{
	bAiming = bInAiming;
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed) Fire();
}

void UCombatComponent::ThrowGrenadeFinished()
{
	AttachActorToRightHand(EquippedWeapon);
	CombatState = ECombatState::ECS_Unoccupied;
}

void UCombatComponent::LaunchGrenade()
{
	if (Character && Character->IsLocallyControlled())
	{
		ServerLaunchGrenade(HitTarget);
	}
}

void UCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize& Target)
{
	if (Character && GrenadeClass)
	{
		Character->SetAttachedGrenadeVisibility(false);
		const FVector StartingLocation = Character->GetGrenadeLocation();
		FVector ToTarget = Target - StartingLocation;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		if (GetWorld())
		{
			GetWorld()->SpawnActor<AProjectile>(GrenadeClass, StartingLocation, ToTarget.Rotation(), SpawnParams);
		}
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& HitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2, ViewportSize.Y / 2);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);
	
	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;
		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		}
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;
		GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility);
		if (!HitResult.bBlockingHit) HitResult.ImpactPoint = End;
		if (HitResult.GetActor() && HitResult.GetActor()->Implements<UInteractWithCrosshairs>())
		{
			HUDPackage.CrosshairsColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairsColor = FLinearColor::White;
		}
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (!Character || !Character->Controller) return;

	Controller = Controller == nullptr ? Cast<ABlasterController>(Character->Controller) : Controller;
	if (!Controller) return;

	HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;
	if (!HUD) return;

	if (EquippedWeapon)
	{
		HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
		HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
		HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
		HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
		HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
	}
	else
	{
		HUDPackage.CrosshairsCenter = nullptr;
		HUDPackage.CrosshairsLeft = nullptr;
		HUDPackage.CrosshairsRight = nullptr;
		HUDPackage.CrosshairsTop = nullptr;
		HUDPackage.CrosshairsBottom = nullptr;
	}

	FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
	FVector2D VelocityMultiplier(0.f, 1.f);
	FVector Velocity = Character->GetVelocity();
	Velocity.Z = 0.f;
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplier, Velocity.Size());
	if (Character->GetCharacterMovement()->IsFalling())
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
	}
	else
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
	}

	if (bAiming)
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.f);
	}
	else
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
	}

	CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 20.f);
	
	HUDPackage.CrosshairSpread = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;

	HUD->SetHUDPackage(HUDPackage);
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (!EquippedWeapon) return;

	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}

	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::FireTimerFinished()
{
	bCanFire = true;
	if (!bFireButtonPressed || !EquippedWeapon || !EquippedWeapon->IsAutomatic()) return;
	Fire();

	if (EquippedWeapon->IsEmpty()) Reload();
}

void UCombatComponent::StartFireTimer()
{
	if (!EquippedWeapon || !Character) return;
	Character->GetWorldTimerManager().SetTimer(FireTimer, this, &UCombatComponent::FireTimerFinished, EquippedWeapon->GetFireDelay());
}

void UCombatComponent::Fire()
{
	if (bFireButtonPressed && CanFire() && EquippedWeapon)
	{
		CrosshairShootingFactor = 0.75f;
		bCanFire = false;

		switch (EquippedWeapon->GetFireType())
		{
		case EFireType::EFT_Projectile:
			FireProjectileWeapon();
			break;
		case EFireType::EFT_HitScan:
			FireHitScanWeapon();
			break;
		case EFireType::EFT_Shotgun:
			FireShotgunWeapon();
			break;
		}

		StartFireTimer();
	}
}

void UCombatComponent::FireProjectileWeapon()
{
	if (!EquippedWeapon || !Character) return;

	HitTarget = EquippedWeapon->GetUseScatter() ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
	if (!Character->HasAuthority()) LocalFire(HitTarget);
	ServerFire(HitTarget, EquippedWeapon->GetFireDelay());
}

void UCombatComponent::FireHitScanWeapon()
{
	if (!EquippedWeapon || !Character) return;

	HitTarget = EquippedWeapon->GetUseScatter() ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
	if (!Character->HasAuthority()) LocalFire(HitTarget);
	ServerFire(HitTarget, EquippedWeapon->GetFireDelay());
}

void UCombatComponent::FireShotgunWeapon()
{
	if (!EquippedWeapon || !Character) return;

	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if (Shotgun)
	{
		TArray<FVector_NetQuantize> HitTargets;
		Shotgun->ShotgunTraceEndWithScatter(HitTarget, HitTargets);
		if (!Character->HasAuthority()) LocalShotgunFire(HitTargets);
		ServerShotgunFire(HitTargets, EquippedWeapon->GetFireDelay());
	}
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHit)
{
	if (!EquippedWeapon || !Character || CombatState != ECombatState::ECS_Unoccupied) return;

	Character->PlayFireMontage(bAiming);
	EquippedWeapon->Fire(TraceHit);
	CombatState = ECombatState::ECS_Unoccupied;
}

void UCombatComponent::LocalShotgunFire(const TArray<FVector_NetQuantize>& TraceHits)
{
	if (!EquippedWeapon || !Character) return;

	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);

	if (CombatState == ECombatState::ECS_Unoccupied || CombatState == ECombatState::ECS_Reloading || !Shotgun)
	{
		Character->PlayFireMontage(bAiming);
		Shotgun->FireShotgun(TraceHits);
		CombatState = ECombatState::ECS_Unoccupied;
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	MulticastFire(TraceHitTarget);
}

bool UCombatComponent::ServerFire_Validate(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	if (EquippedWeapon)
	{
		bool bNearlyEqual = FMath::IsNearlyEqual(EquippedWeapon->GetFireDelay(), FireDelay, 0.001f);
		return bNearlyEqual;
	}
	return true;
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;

	LocalFire(TraceHitTarget);
}

void UCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay)
{
	MulticastShotgunFire(TraceHitTargets);
}

bool UCombatComponent::ServerShotgunFire_Validate(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay)
{
	if (EquippedWeapon)
	{
		bool bNearlyEqual = FMath::IsNearlyEqual(EquippedWeapon->GetFireDelay(), FireDelay, 0.001f);
		return bNearlyEqual;
	}
	return true;
}

void UCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	LocalShotgunFire(TraceHitTargets);
}

bool UCombatComponent::CanFire()
{
	return bCanFire && EquippedWeapon && !EquippedWeapon->IsEmpty() &&
		(CombatState == ECombatState::ECS_Unoccupied || (CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun) &&
		!bLocallyReloading);
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	Controller = Controller == nullptr ? Character->GetBlasterController() : Controller;
	if (Controller) Controller->SetHUDCarriedAmmo(CarriedAmmo);
	if (CombatState == ECombatState::ECS_Reloading && EquippedWeapon && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun && CarriedAmmo == 0)
	{
		Character->JumpToShotgunReloadEnd();
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SubmachineGun, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartingSniperRifleAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGrenadeLauncherAmmo);
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, Grenades);
}

bool UCombatComponent::ShouldSwapWeapons()
{
	return EquippedWeapon && SecondaryWeapon && CombatState == ECombatState::ECS_Unoccupied;
}

void UCombatComponent::SwapWeapons()
{
	if (!Character || !EquippedWeapon || !SecondaryWeapon || CombatState != ECombatState::ECS_Unoccupied) return;

	Character->PlaySwapMontage();
	Character->bFinishedSwapping = false;
	CombatState = ECombatState::ECS_Swapping;

	SecondaryWeapon->SetCustomDepthEnabled(false);

	AWeapon* Temp = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = Temp;
}

void UCombatComponent::EquipWeapon(AWeapon* ToEquip)
{
	if (!Character || !ToEquip || CombatState != ECombatState::ECS_Unoccupied) return;

	if (EquippedWeapon && !SecondaryWeapon) EquipSecondaryWeapon(ToEquip);
	else EquipPrimaryWeapon(ToEquip);

	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* ToEquip)
{
	if (!Character || !ToEquip) return;

	if (EquippedWeapon) EquippedWeapon->Dropped();

	EquippedWeapon = ToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	AttachActorToRightHand(EquippedWeapon);

	EquippedWeapon->SetOwner(Character); // Note that SetOwner() is provided by Actor and is replicated
	EquippedWeapon->SetHUDAmmo();
	EquippedWeapon->PlayEquipSound(Character->GetActorLocation());

	UpdateCarriedAmmo();

	if (EquippedWeapon->IsEmpty()) Reload();
}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* ToEquip)
{
	if (!Character || !ToEquip) return;

	SecondaryWeapon = ToEquip;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_Secondary);
	SecondaryWeapon->SetOwner(Character);
	SecondaryWeapon->PlayEquipSound(Character->GetActorLocation());
	AttachActorToBackpack(SecondaryWeapon);
}

void UCombatComponent::AttachActorToRightHand(AWeapon* ToAttach)
{
	if (!Character || !ToAttach || !Character->GetMesh()) return;

	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(ToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToLeftHand(AWeapon* ToAttach)
{
	if (!Character || !Character->GetMesh() || !ToAttach) return;

	bool bUsePistolSocket = ToAttach->GetWeaponType() == EWeaponType::EWT_Pistol || ToAttach->GetWeaponType() == EWeaponType::EWT_SubmachineGun;
	FName SocketName = bUsePistolSocket ? FName("PistolHandSocket") : FName("LeftHandSocket");
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(SocketName);
	if (HandSocket)
	{
		HandSocket->AttachActor(ToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToBackpack(AWeapon* ToAttach)
{
	if (!Character || !Character->GetMesh() || !ToAttach) return;

	const USkeletalMeshSocket* BackpackSocket = Character->GetMesh()->GetSocketByName(FName("BackpackSocket"));
	if (BackpackSocket)
	{
		BackpackSocket->AttachActor(ToAttach, Character->GetMesh());
	}
}

void UCombatComponent::UpdateCarriedAmmo()
{
	if (!EquippedWeapon) return;

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	Controller = Controller == nullptr ? Character->GetBlasterController() : Controller;
	if (Controller) Controller->SetHUDCarriedAmmo(CarriedAmmo);
}

void UCombatComponent::Reload()
{
	if (CarriedAmmo <= 0 || bLocallyReloading || CombatState != ECombatState::ECS_Unoccupied || (EquippedWeapon && EquippedWeapon->IsFull())) return;

	ServerReload();
	bLocallyReloading = true;
	HandleReload();
}

void UCombatComponent::ShotgunShellReload()
{
	if (!EquippedWeapon || EquippedWeapon->GetWeaponType() != EWeaponType::EWT_Shotgun) return;

	if (Character && Character->HasAuthority())
	{
		UpdateShotgunAmmoValues();
	}
}

void UCombatComponent::ServerReload_Implementation()
{
	if (!Character || !EquippedWeapon) return;

	CombatState = ECombatState::ECS_Reloading;
	if (!Character->IsLocallyControlled()) HandleReload();
}

void UCombatComponent::HandleReload()
{
	if (!Character) return;

	Character->PlayReloadMontage();
}

void UCombatComponent::FinishReloading()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	bLocallyReloading = false;
	if (Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		//UE_LOG(LogTemp, Warning, TEXT("FinishReloading(): Character has authority"));
		UpdateAmmoValues();
	}

	if (bFireButtonPressed) Fire();
}

void UCombatComponent::FinishSwapping()
{
	if (Character && Character->HasAuthority()) CombatState = ECombatState::ECS_Unoccupied;

	Character->bFinishedSwapping = true;

	if (SecondaryWeapon) SecondaryWeapon->SetCustomDepthEnabled(true);
}

void UCombatComponent::FinishSwapAttachWeapons()
{
	// Equipped Weapon
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetHUDAmmo();
	EquippedWeapon->PlayEquipSound(Character->GetActorLocation());
	UpdateCarriedAmmo();
	if (EquippedWeapon->IsEmpty()) Reload();

	//Secondary Weapon
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_Secondary);
	AttachActorToBackpack(SecondaryWeapon);
}

void UCombatComponent::UpdateAmmoValues()
{
	int32 ReloadAmount = AmountToReload();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	Controller = Controller == nullptr ? Character->GetBlasterController() : Controller;
	if (Controller) Controller->SetHUDCarriedAmmo(CarriedAmmo);

	EquippedWeapon->AddAmmo(ReloadAmount);
}

void UCombatComponent::UpdateHUDGrenades()
{
	Controller = Controller == nullptr ? Cast<ABlasterController>(Character->Controller) : Controller;
	if (!Controller) return;
	Controller->SetHUDGrenades(Grenades);
}

void UCombatComponent::OnRep_Grenades()
{
	UpdateHUDGrenades();
}

void UCombatComponent::OnRep_Aiming()
{
	if (Character && Character->IsLocallyControlled())
	{
		bAiming = bAimButtonPressed;
	}
}

void UCombatComponent::UpdateShotgunAmmoValues()
{
	if (!Character || !EquippedWeapon) return;

	int32 ReloadAmount = 1;
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	Controller = Controller == nullptr ? Character->GetBlasterController() : Controller;
	if (Controller) Controller->SetHUDCarriedAmmo(CarriedAmmo);

	EquippedWeapon->AddAmmo(1);
	bCanFire = true;
	if (EquippedWeapon->IsFull() || CarriedAmmo == 0)
	{
		Character->JumpToShotgunReloadEnd();
	}
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		if (Character && !Character->IsLocallyControlled()) HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed) Fire();
		break;
	case ECombatState::ECS_ThrowingGrenade:
		if (Character && !Character->IsLocallyControlled())
		{
			Character->SetAttachedGrenadeVisibility(true);
			Character->PlayThrowGrenadeMontage();
			AttachActorToLeftHand(EquippedWeapon);
		}
		break;
	case ECombatState::ECS_Swapping:
		if (Character && !Character->IsLocallyControlled()) Character->PlaySwapMontage();
		break;
	}
}

void UCombatComponent::ThrowGrenade()
{
	if (CombatState != ECombatState::ECS_Unoccupied || Grenades == 0) return;

	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->SetAttachedGrenadeVisibility(true);
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
	}
	if (Character && !Character->HasAuthority())
	{
		ServerThrowGrenade();
	}
	if (Character && Character->HasAuthority())
	{
		Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
		UpdateHUDGrenades();
	}
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
	if (Grenades == 0) return;

	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->SetAttachedGrenadeVisibility(true);
		Character->PlayThrowGrenadeMontage();
	}
	Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
	UpdateHUDGrenades();
}

int32 UCombatComponent::AmountToReload()
{
	if (!EquippedWeapon) return 0;

	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = CarriedAmmo < RoomInMag ? CarriedAmmo : RoomInMag;
		return FMath::Clamp(RoomInMag, 0, Least);
	}

	return 0;
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToRightHand(EquippedWeapon);

		EquippedWeapon->PlayEquipSound(Character->GetActorLocation());
		EquippedWeapon->SetCustomDepthEnabled(false);
		EquippedWeapon->SetHUDAmmo();

		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
	if (SecondaryWeapon && Character)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_Secondary);
		AttachActorToBackpack(SecondaryWeapon);
		SecondaryWeapon->PlayEquipSound(Character->GetActorLocation());
	}
}

