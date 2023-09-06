// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	bool bHeadShot;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<ABlaster*, uint32> Headshots;
	
	UPROPERTY()
	TMap<ABlaster*, uint32> Bodyshots;
};

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;

};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()
	
	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;

	UPROPERTY()
	ABlaster* Character;
};

class ABlaster;
class ABlasterController;
class AWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MPTESTING_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	friend ABlaster;
	ULagCompensationComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void ShowFramePackage(const FFramePackage& Package, const FColor& Color = FColor::Green);

	/* For hitscan weapons */
	FServerSideRewindResult ServerSideRewind(ABlaster* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);

	/* For projectile weapons */
	FServerSideRewindResult ProjectileServerSideRewind(ABlaster* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);

	/* For shotgun weapons (a subclass of hitscan) */
	FShotgunServerSideRewindResult ShotgunServerSideRewind(const TArray<ABlaster*>& HitCharacters, const FVector_NetQuantize TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime);

	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(ABlaster* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime, AWeapon* DamageCauser);
	
	UFUNCTION(Server, Reliable)
	void ProjectileServerScoreRequest(ABlaster* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);

	UFUNCTION(Server, Reliable)
	void ShotgunServerScoreRequest(const TArray<ABlaster*>& HitCharacters, const FVector_NetQuantize TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime);

protected:
	virtual void BeginPlay() override;
	void SaveFramePackage(FFramePackage& Package);
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);
	void CacheBoxPositions(ABlaster* HitCharacter, FFramePackage& OutFramePackage);
	void MoveBoxes(ABlaster* HitCharacter, const FFramePackage& Package);
	void ResetHitBoxes(ABlaster* HitCharacter, const FFramePackage& Package);
	void EnableCharacterMeshCollision(ABlaster* HitCharacter, ECollisionEnabled::Type CollisionEnabled);
	void SaveFramePackage();
	FFramePackage GetFrameToCheck(ABlaster* HitCharacter, float HitTime);

	FServerSideRewindResult ConfirmHit(const FFramePackage& Package, ABlaster* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation);
	FServerSideRewindResult ProjectileConfirmHit(const FFramePackage& Package, ABlaster* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity);
	FShotgunServerSideRewindResult ShotgunConfirmHit(const TArray<FFramePackage>& FramePackages, const FVector_NetQuantize TraceStart, const TArray<FVector_NetQuantize>& HitLocations);
private:
	UPROPERTY()
	ABlaster* Character;

	UPROPERTY()
	ABlasterController* Controller;

	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere, Category = "Lag Comp Properties")
	float MaxRecordTime = 4.f;
};
