// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Enums/WeaponTypes.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Secondary UMETA(DisplayName = "Secondary"),
	EWS_Dropped UMETA(DisplayName = "Dropped")
};

UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "Hit Scan Weapon"),
	EFT_Projectile UMETA(DisplayName = "Projectile"),
	EFT_Shotgun UMETA(DisplayName = "Shotgun"),
};

class USphereComponent;
class UWidgetComponent;
class UAnimationAsset;
class ACasing;
class ABlaster;
class ABlasterController;
class USoundCue;

UCLASS()
class MPTESTING_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetOwner(AActor* NewOwner) override;
	virtual void OnRep_Owner() override;
	void SetPickupWidgetVisibility(bool bIsVisible);
	void SetWeaponState(EWeaponState State);
	virtual void Fire(const FVector& HitTarget);
	void Dropped();
	void SetHUDAmmo();
	void AddAmmo(int32 AmountToAdd);
	void PlayEquipSound(FVector Location);
	void SetCustomDepthEnabled(bool bEnabled);
	FVector TraceEndWithScatter(const FVector& HitTarget);
	
	/*
	* Textures for the weapon crosshairs
	*/
	UPROPERTY(EditAnywhere, Category = "Weapon Properties | Crosshairs")
	UTexture2D* CrosshairsCenter;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Properties | Crosshairs")
	UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties | Crosshairs")
	UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties | Crosshairs")
	UTexture2D* CrosshairsTop;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Properties | Crosshairs")
	UTexture2D* CrosshairsBottom;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties | Equip")
	USoundCue* EquipSound;

	bool bDestroyWeapon = false;

protected:
	virtual void BeginPlay() override;
	virtual void OnWeaponStateSet();
	virtual void OnEquipped();
	virtual void OnDropped();
	virtual void OnSecondary();

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UFUNCTION()
	void OnPingTooHigh(bool bPingTooHigh);

	UPROPERTY()
	ABlaster* Blaster;

	UPROPERTY()
	ABlasterController* BlasterController;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties | Scatter");
	float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties | Scatter");
	float SphereRadius = 75.f;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Properties | Damage")
	float Damage = 20.f;

	UPROPERTY(Replicated, EditAnywhere, Category = "Weapon Properties | Default");
	bool bUseServerRewind = false;

private:
	UFUNCTION()
	void OnRep_WeaponState();
	void SpendRound();

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmountToAdd);

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USphereComponent* AreaSphere;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	UWidgetComponent* PickupWidget;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties | Default")
	EWeaponState WeaponState = EWeaponState::EWS_Initial;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties | Default")
	EFireType FireType = EFireType::EFT_Projectile;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Properties | Default")
	EWeaponType WeaponType = EWeaponType::EWT_AssaultRifle;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties | Firing")
	UAnimationAsset* FireAnimation;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Properties | Default")
	TSubclassOf<ACasing> CasingClass;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties | Default")
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties | Default")
	float ZoomInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties | Firing")
	float FireDelay = .2f;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties | Firing")
	bool bAutomatic = true;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties | Default")
	int32 Ammo = 30;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties | Default")
	int32 MagCapacity = 30;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties | Scatter");
	bool bUseScatter = false;

	/* Number of unprocessed server requests for Ammo
	* Incremented in SpendRound, decremented in ClientUpdateAmmo */
	int32 Sequence = 0;

public:
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	FORCEINLINE float GetFireDelay() const { return FireDelay; }
	FORCEINLINE bool IsAutomatic() const { return bAutomatic; }
	FORCEINLINE bool IsEmpty() const { return Ammo == 0; }
	FORCEINLINE bool IsFull() const { return Ammo == MagCapacity; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
	FORCEINLINE EFireType GetFireType() const { return FireType; }
	FORCEINLINE bool GetUseScatter() const { return bUseScatter; }
	FORCEINLINE float GetDamage() const { return Damage; }
};
