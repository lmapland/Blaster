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

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UWidgetComponent;
class USoundCue;
class AWeapon;
class UCombatComponent;
class UBuffComponent;
class ABlasterController;
class ABlasterPlayerState;
class UBoxComponent;
class ULagCompensationComponent;
class UNiagaraSystem;
class UNiagaraComponent;

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
	void PlayThrowGrenadeMontage();
	FVector GetHitTarget() const;
	void PlayHitReactMontage();
	void PlaySwapMontage();
	virtual void OnRep_ReplicatedMovement() override;
	void Elim(bool bPlayerLeftGame);
	virtual void Destroyed() override;
	void SetHUDHealth();
	void SetHUDShield();
	ECombatState GetCombatState() const;
	void JumpToShotgunReloadEnd();

	/* For adding health to the player. Cannot be used for removing health from the player.
	 * By default it increases the shield value as well after Health is maxed out. */
	void UpdateHealth(float Amount);

	/* Increases only the Shield value and ignores the Health value. */
	void ReplenishShield(float Amount);

	/* Updates the character's base movement. DOES NOT STACK. If this function is called a second time, the new percent will overwrite the old percent. */
	void UpdateMovementSpeedByPercent(float Percent);

	/* Updates the character's base jump velocity. DOES NOT STACK. */
	void UpdateJumpVelocityByPercent(float Percent);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim(bool bPlayerLeftGame);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScope(bool bShowScope);

	void SetAttachedGrenadeVisibility(bool bIsVisible);
	FVector GetGrenadeLocation();
	bool IsLocallyReloading();
	
	UFUNCTION(Server, Reliable)
	void ServerLeaveGame();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastGainedTheLead();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastLostTheLead();

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	/* Hitboxes used for server-side rewind */
	UPROPERTY(EditAnywhere)
	UBoxComponent* Head;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* Pelvis;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* Spine02;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* Spine03;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* UpperArmL;

	UPROPERTY(EditAnywhere)
	UBoxComponent* LowerArmL;

	UPROPERTY(EditAnywhere)
	UBoxComponent* HandL;

	UPROPERTY(EditAnywhere)
	UBoxComponent* UpperArmR;

	UPROPERTY(EditAnywhere)
	UBoxComponent* LowerArmR;

	UPROPERTY(EditAnywhere)
	UBoxComponent* HandR;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* Backpack;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* Blanket;

	UPROPERTY(EditAnywhere)
	UBoxComponent* ThighL;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* CalfL;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* FootL;

	UPROPERTY(EditAnywhere)
	UBoxComponent* ThighR;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* CalfR;

	UPROPERTY(EditAnywhere)
	UBoxComponent* FootR;
	
	UPROPERTY()
	TMap<FName, UBoxComponent*> HitCollisionBoxes;

	bool bFinishedSwapping = false;

	FOnLeftGame OnLeftGame;

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
	void GrenadeButtonPressed();
	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	void SimProxiesTurn();

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);

	/* For removing health from the player; cannot be used for increasing the player's health */
	void HandleDamage(float Damage);

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
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ThrowGrenadeAction;

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

	void SpawnDefaultWeapon();

	// For handling situations where the reload montage is interrupted or does not play it's FinishReload anim notify
	UFUNCTION()
	void ReloadMontageEndedHandler(UAnimMontage* Montage, bool bInterrupted);

	void ReloadTimerFinished();
	void SwapAttachedTimerFinished();
	void SwapTimerFinished();

	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* FollowCamera;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Widgets, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* DisplayNameWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCombatComponent* CombatComponent2;
	
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	UBuffComponent* BuffComponent2;

	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	ULagCompensationComponent* LagCompensation;

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
	
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ThrowGrenadeMontage;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* SwapWeaponMontage;

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
	void OnRep_Health(float PreviousHealth);
	
	UFUNCTION()
	void OnRep_Shield(float PreviousShield);

	UFUNCTION()
	void OnRep_Stamina();

	UPROPERTY(EditAnywhere, Category = "Player Stats");
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, EditAnywhere, Category = "Player Stats");
	float Health = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Shield, EditAnywhere, Category = "Player Stats");
	float Shield = 0.f;

	UPROPERTY(EditAnywhere, Category = "Player Stats");
	float MaxShield = 50.f;

	UPROPERTY(EditAnywhere, Category = "Player Stats");
	float MaxStamina = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Stamina, EditAnywhere, Category = "Player Stats");
	float Stamina = 100.f;

	bool bElimmed = false;

	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly, Category = Elim)
	float ElimDelay = 3.f;

	bool bLeftGame = false;

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

	UPROPERTY(EditAnywhere, Category = Crown)
	UNiagaraSystem* CrownSystem;

	UPROPERTY()
	UNiagaraComponent* CrownComponent;

	FTimerHandle AfterBeginPlayTimer;

	UPROPERTY(EditAnywhere, Category = Initialization)
	float AfterBeginPlayTime = .1f;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachedGrenade;

	/* These Movement Component variables can be changed throughout the gameplay,
	 * but we also want to be able to set the Movement Component variables back to their original values,
	 * so we will save off the base values as the Character is created.
	 */
	float BaseWalkSpeed = 400.f;
	float BaseCrouchSpeed = 200.f;
	float BaseJumpVelocity = 100.f;

	UPROPERTY(EditAnywhere, Category = "Character | Defaults")
	TSubclassOf<AWeapon> DefaultWeaponClass;

	FTimerHandle ReloadTimer;
	FTimerHandle SwapFinishedTimer;
	FTimerHandle SwapAttachedWepsTimer;

public:
	FORCEINLINE float GetAOYaw() const { return AO_Yaw; }
	FORCEINLINE float GetAOPitch() const { return AO_Pitch; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsElimmed() const { return bElimmed; }
	FORCEINLINE ABlasterController* GetBlasterController() const { return BlasterController; }
	FORCEINLINE UCombatComponent* GetCombatComponent() const { return CombatComponent2; }
	FORCEINLINE UBuffComponent* GetBuffComponent() const { return BuffComponent2; }
	FORCEINLINE bool GameplayIsDisabled() const { return bDisableGameplay; }
	FORCEINLINE ULagCompensationComponent* GetLagCompensation() const { return LagCompensation; }
};
