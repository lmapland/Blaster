// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HUD/BlasterHUD.h"
#include "Enums/WeaponTypes.h"
#include "Enums/CombatState.h"
#include "CombatComponent.generated.h"

class ABlaster;
class ABlasterController;
class ABlasterHUD;
class AWeapon;
class AProjectile;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MPTESTING_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	friend ABlaster;

	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void EquipWeapon(AWeapon* ToEquip);
	void Reload();

	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();

	void UpdateShotgunAmmoValues();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	void FireButtonPressed(bool bPressed);

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();

	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);

	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);

protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bInAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bInAiming);
	
	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_CarriedAmmo();

	UFUNCTION()
	void OnRep_CombatState();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshairs(FHitResult& HitResult);
	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();
	void ThrowGrenade();

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	void AttachActorToRightHand(AWeapon* ToAttach);
	void AttachActorToLeftHand(AWeapon* ToAttach);
	void UpdateCarriedAmmo();

	int32 AmountToReload();

	UPROPERTY(EditAnywhere, Category = "Combat Properties | Ammo")
	TSubclassOf<AProjectile> GrenadeClass;

private:
	void InterpFOV(float DeltaTime);
	void FireTimerFinished();
	void StartFireTimer();
	void Fire();
	bool CanFire();
	void InitializeCarriedAmmo();
	void UpdateAmmoValues();
	void UpdateHUDGrenades();

	UFUNCTION()
	void OnRep_Grenades();

	UPROPERTY()
	ABlaster* Character;

	UPROPERTY()
	ABlasterController* Controller;

	UPROPERTY()
	ABlasterHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon = nullptr;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere, Category = "Combat Properties | Movement")
	float BaseWalkSpeed = 600.f;

	UPROPERTY(EditAnywhere, Category = "Combat Properties | Movement")
	float AimWalkSpeed = 450.f;

	bool bFireButtonPressed = false;

	FVector HitTarget;

	// Field of view when not aiming; set to the camera's base FOV in BeginPlay
	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = "Combat Properties | Combat")
	float ZoomedFOV = 30.f;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = "Combat Properties | Combat")
	float ZoomInterpSpeed = 20.f;

	/*
	* HUD and crosshairs
	*/
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;
	FHUDPackage HUDPackage;

	/**
	* Automatic Fire
	*/
	FTimerHandle FireTimer;

	bool bCanFire = true;

	// Carried Ammo for the currently equipped weapon
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UPROPERTY(EditAnywhere, Category = "Combat Properties | Ammo")
	int32 StartingARAmmo = 30;

	UPROPERTY(EditAnywhere, Category = "Combat Properties | Ammo")
	int32 StartingRocketAmmo = 3;

	UPROPERTY(EditAnywhere, Category = "Combat Properties | Ammo")
	int32 StartingPistolAmmo = 25;

	UPROPERTY(EditAnywhere, Category = "Combat Properties | Ammo")
	int32 StartingSMGAmmo = 10;

	UPROPERTY(EditAnywhere, Category = "Combat Properties | Ammo")
	int32 StartingShotgunAmmo = 10;

	UPROPERTY(EditAnywhere, Category = "Combat Properties | Ammo")
	int32 StartingSniperRifleAmmo = 10;

	UPROPERTY(EditAnywhere, Category = "Combat Properties | Ammo")
	int32 StartingGrenadeLauncherAmmo = 8;

	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere, Category = "Combat Properties | Ammo")
	int32 MaxCarriedAmmo = 500;

	UPROPERTY(ReplicatedUsing = OnRep_Grenades, EditAnywhere, Category = "Combat Properties | Ammo")
	int32 Grenades = 3;

	UPROPERTY(EditAnywhere, Category = "Combat Properties | Ammo")
	int32 MaxGrenades = 10;

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	public:
		FORCEINLINE int32 GetGrenadeCount() const { return Grenades; }
};
