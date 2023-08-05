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
	EWS_Dropped UMETA(DisplayName = "Dropped")
	//EWS_Initial UMETA(DisplayName = "Initial State"),
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
	
	/*
	* Textures for the weapon crosshairs
	*/
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsCenter;
	
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsTop;
	
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsBottom;

	UPROPERTY(EditAnywhere, Category = Equip)
	USoundCue* EquipSound;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	UFUNCTION()
	void OnRep_WeaponState();

	UFUNCTION()
	void OnRep_Ammo();

	void SpendRound();

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USphereComponent* AreaSphere;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	UWidgetComponent* PickupWidget;

	UPROPERTY()
	ABlaster* Blaster;

	UPROPERTY()
	ABlasterController* BlasterController;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState = EWeaponState::EWS_Initial;
	
	UPROPERTY(EditAnywhere, Category = "WeaponProperties")
	EWeaponType WeaponType = EWeaponType::EWT_AssaultRifle;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	UAnimationAsset* FireAnimation;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<ACasing> CasingClass;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float ZoomInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float FireDelay = .2f;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	bool bAutomatic = true;

	UPROPERTY(ReplicatedUsing = OnRep_Ammo)
	int32 Ammo = 30;

	int32 MagCapacity = 30;

public:
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	FORCEINLINE float GetFireDelay() const { return FireDelay; }
	FORCEINLINE bool IsAutomatic() const { return bAutomatic; }
	FORCEINLINE bool IsEmpty() const { return Ammo == 0; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
};
