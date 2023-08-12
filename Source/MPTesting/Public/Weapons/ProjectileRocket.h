// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Projectile.h"
#include "ProjectileRocket.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class URocketMovementComponent;

/**
 * 
 */
UCLASS()
class MPTESTING_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()
	
public:
	AProjectileRocket();
	virtual void Destroyed() override;

protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Projectile Properties | Sound")
	USoundCue* ProjectileLoop;

	UPROPERTY()
	UAudioComponent* ProjectiveLoopComponent;

	UPROPERTY(EditAnywhere, Category = "Projectile Properties | Sound")
	USoundAttenuation* LoopingSoundAttenuation;

	UPROPERTY(EditAnywhere)
	URocketMovementComponent* RocketMovementComponent;

};
