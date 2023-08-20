// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Characters/Blaster.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Enums/WeaponTypes.h"
#include "DrawDebugHelpers.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;

	FHitResult HitResult;
	FTransform SocketTransform;
	PerformTraceHit(HitTarget, SocketTransform, HitResult);

	FVector BeamEnd = HitResult.TraceEnd;

	if (HitResult.bBlockingHit)
	{
		BeamEnd = HitResult.ImpactPoint;
		if (HasAuthority()) ApplyDamageOnHit(Cast<ABlaster>(HitResult.GetActor()), OwnerPawn, Damage);
		PlayFireImpactEffects(HitResult);
	}
	FireBeam(SocketTransform, BeamEnd);
	PlayFireBeginEffects(SocketTransform);
}

void AHitScanWeapon::PerformTraceHit(const FVector& HitTarget, FTransform& SocketTransform, FHitResult& HitResult)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();
		const FVector End = Start + (HitTarget - Start) * 1.25f;

		if (GetWorld())
		{
			GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility);
		}
	}
}

void AHitScanWeapon::ApplyDamageOnHit(ABlaster* TargetHit, APawn* OwnerPawn, float DamageToApply)
{
	AController* InstigatorController = OwnerPawn->GetController();
	if (TargetHit && HasAuthority() && InstigatorController)
	{
		UGameplayStatics::ApplyDamage(TargetHit, DamageToApply, InstigatorController, this, UDamageType::StaticClass());
	}
}

void AHitScanWeapon::PlayFireBeginEffects(FTransform& SocketTransform)
{
	if (MuzzleFlash)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
	}
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}
}

void AHitScanWeapon::PlayFireImpactEffects(FHitResult& HitResult)
{
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, HitResult.ImpactPoint);
	}
}

void AHitScanWeapon::FireBeam(FTransform& SocketTransform, const FVector& BeamEnd)
{
	if (BeamParticles)
	{
		UParticleSystemComponent* BeamComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
		if (BeamComponent)
		{
			BeamComponent->SetVectorParameter(FName("Target"), BeamEnd);
		}
	}
}
