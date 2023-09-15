// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UBoxComponent;
class UProjectileMovementComponent;
class UParticleSystemComponent;
class USoundCue;
class UNiagaraSystem;
class UNiagaraComponent;

UCLASS()
class MPTESTING_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:
	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

	// Used with server-side rewind
	bool bUseServerSideRewind = false;
	FVector_NetQuantize TraceStart;
	FVector_NetQuantize100 InitialVelocity;

	UPROPERTY(EditAnywhere)
	float InitialSpeed = 15000;
	
	/* Damage value used for direct damage and also the BaseDamage value used for Radial Damage
	   Only set this for Grenades and Rockets */
	UPROPERTY(EditAnywhere, Category = "Projectile Properties | Damage")
	float Damage = 10.f;
	
	/* Isn't used for Grenades and Rockets */
	UPROPERTY(EditAnywhere, Category = "Projectile Properties | Damage")
	float HeadShotDamage = 20.f;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void SpawnTrailSystem();
	void StartDestroyTimer();
	void DestroyTimerFinished();
	void ApplyRadialDamage();

	UPROPERTY(EditAnywhere, Category = "Projectile Properties | Radial Damage")
	float DamageMinimum = 1.f;

	UPROPERTY(EditAnywhere, Category = "Projectile Properties | Radial Damage")
	float InnerRadius = 500.f;

	UPROPERTY(EditAnywhere, Category = "Projectile Properties | Radial Damage")
	float OuterRadius = 200.f;

	/* 1.f means linear damage falloff */
	UPROPERTY(EditAnywhere, Category = "Projectile Properties | Radial Damage")
	float DamageFalloffExp = 1.f;
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY()
	AController* OwnerController;

	UPROPERTY(EditAnywhere, Category = "Projectile Properties | Impact");
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere, Category = "Projectile Properties | Impact");
	USoundCue* ImpactSound;

	UPROPERTY(EditAnywhere);
	UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere);
	UProjectileMovementComponent* ProjectileMovementComponent;
	
	UPROPERTY(EditAnywhere, Category = "Projectile Properties | Trail")
	UNiagaraSystem* TrailSystem;

	UPROPERTY()
	UNiagaraComponent* TrailSystemComponent;

private:
	UPROPERTY(EditAnywhere, Category = "Projectile Properties | Tracer");
	UParticleSystem* Tracer;

	UParticleSystemComponent* TracerComponent;
	
	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere, Category = "Projectile Properties | Trail")
	float DestroyTime = 3.f;
};
