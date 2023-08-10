// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Enums/TurningInPlace.h"
#include "Enums/CombatState.h"
#include "Interfaces/InteractWithCrosshairs.h"
#include "Components/TimelineComponent.h"
#include "Blaster.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UWidgetComponent;
class USoundCue;
class AWeapon;
class UCombatComponent;
class ABlasterController;
class ABlasterPlayerState;

UCLASS()
class MPTESTING_API ABlaster : public ACharacter, public IInteractWithCrosshairs
{
	GENERATED_BODY()

public:
	ABlaster();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Jump() override;
	void SetOverlappingWeapon(AWeapon* Weapon);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	bool IsWeaponEquipped() const;
	bool IsAiming();
	AWeapon* GetEquippedWeapon();
	void PlayFireMontage(bool bAiming);
	void PlayElimMontage();
	void PlayReloadMontage();
	FVector GetHitTarget() const;
	void PlayHitReactMontage();
	virtual void OnRep_ReplicatedMovement() override;
	void Elim();
	virtual void Destroyed() override;
	void SetHUDHealth();
	ECombatState GetCombatState() const;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

protected:
	virtual void BeginPlay() override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Sprint(const FInputActionValue& Value);
	void EndSprint(const FInputActionValue& Value);
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void FireButtonPressed();
	void FireButtonReleased();
	void Equip();
	void ReloadButtonPressed();
	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	void SimProxiesTurn();

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* CharMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SprintAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* CrouchAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* EquipAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* AimAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* FireAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ReloadAction;


private:
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* PreviousWeapon);

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	void TurnInPlace(float DeltaTime);
	void HideCameraIfCharacterClose();
	float CalculateSpeed();
	void ElimTimerFinished();
	void StartDissolve();
	void AfterBeginPlay();
	void RotateInPlace(float DeltaTime);

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);

	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* FollowCamera;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Widgets, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* DisplayNameWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCombatComponent* CombatComponent2;

	UPROPERTY()
	ABlasterController* BlasterController;

	UPROPERTY()
	ABlasterPlayerState* BlasterPlayerState;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;

	float AO_Yaw = 0.f;
	float AO_Pitch = 0.f;
	float InterpAO_Yaw = 0.f;
	FRotator StartingAimRotation = FRotator(0.f);

	ETurningInPlace TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* FireWeaponMontage;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ElimMontage;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, Category = Camera)
	float CameraThreshold = 200.f;

	bool bRotateRootBone = true;
	float TurnThreshold = 0.5f;
	FRotator PreviousProxyRotation;
	FRotator CurrentProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication = 0.f;

	/**
	* Player Stats
	*/

	UFUNCTION()
	void OnRep_Health();

	UFUNCTION()
	void OnRep_Stamina();

	UPROPERTY(EditAnywhere, Category = "Player Stats");
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, EditAnywhere, Category = "Player Stats");
	float Health = 100.f;

	UPROPERTY(EditAnywhere, Category = "Player Stats");
	float MaxStamina = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Stamina, EditAnywhere, Category = "Player Stats");
	float Stamina = 100.f;

	bool bElimmed = false;

	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly, Category = Elim)
	float ElimDelay = 3.f;

	UPROPERTY(VisibleAnywhere, Category = Elim)
	UTimelineComponent* DissolveTimeline;

	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere, Category = Elim)
	UCurveFloat* DissolveCurve;

	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance;

	UPROPERTY(EditAnywhere, Category = Elim)
	UParticleSystem* ElimBotEffect;

	UPROPERTY(VisibleAnywhere, Category = Elim)
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditAnywhere, Category = Elim)
	USoundCue* ElimBotSound;

	FTimerHandle AfterBeginPlayTimer;

	UPROPERTY(EditAnywhere, Category = Initialization)
	float AfterBeginPlayTime = .1f;


public:
	FORCEINLINE float GetAOYaw() const { return AO_Yaw; }
	FORCEINLINE float GetAOPitch() const { return AO_Pitch; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsElimmed() const { return bElimmed; }
	FORCEINLINE ABlasterController* GetBlasterController() const { return BlasterController; }
	FORCEINLINE UCombatComponent* GetCombatComponent() const { return CombatComponent2; }
	FORCEINLINE bool GameplayIsDisabled() const { return bDisableGameplay; }
};
