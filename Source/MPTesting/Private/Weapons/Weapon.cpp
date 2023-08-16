// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Weapon.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundCue.h"
#include "Characters/Blaster.h"
#include "Controller/BlasterController.h"
#include "Weapons/Casing.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	SetCustomDepthEnabled(true);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	/*
	* We want the AreaSphere to detect collisions only on the server.
	* To get that we here set collisions disabled, and enable them in BeginPlay, but on the server only.
	*/
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
	SetPickupWidgetVisibility(false);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	}
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ABlaster* Char = Cast<ABlaster>(OtherActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("OnSphereOverlap(): Setting OverlappingWeapon"));
		Char->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ABlaster* Char = Cast<ABlaster>(OtherActor))
	{
		Char->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME(AWeapon, Ammo);
}

void AWeapon::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);

	if (NewOwner == nullptr)
	{
		Blaster = nullptr;
		BlasterController = nullptr;
	}
	else
	{
		Blaster = Cast<ABlaster>(NewOwner);
		if (Blaster)
		{
			//UE_LOG(LogTemp, Warning, TEXT("AWeapon::SetOwner(): Cast to Blaster was successful"));
			BlasterController = Blaster->GetBlasterController();
			if (BlasterController)
			{
				//UE_LOG(LogTemp, Warning, TEXT("AWeapon::SetOwner(): Cast to BlasterController was successful"));
			}
		}
	}
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();

	SetHUDAmmo();
}

void AWeapon::SetPickupWidgetVisibility(bool bIsVisible)
{

	if (PickupWidget) PickupWidget->SetVisibility(bIsVisible);
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		//UE_LOG(LogTemp, Warning, TEXT("Setting Pickup State to EWS_Equipped"));
		SetPickupWidgetVisibility(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (WeaponType == EWeaponType::EWT_SubmachineGun)
		{
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			WeaponMesh->SetEnableGravity(true);
			WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		}
		SetCustomDepthEnabled(false);
		break;
	case EWeaponState::EWS_Dropped:
		if (HasAuthority())
		{
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
		WeaponMesh->MarkRenderStateDirty();
		SetCustomDepthEnabled(true);
		break;
	}
}

void AWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		SetPickupWidgetVisibility(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (WeaponType == EWeaponType::EWT_SubmachineGun)
		{
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			WeaponMesh->SetEnableGravity(true);
			WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		}
		SetCustomDepthEnabled(false);
		break;
	case EWeaponState::EWS_Dropped:
		if (HasAuthority())
		{
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
		WeaponMesh->MarkRenderStateDirty();
		SetCustomDepthEnabled(true);
		break;
	}
}

void AWeapon::SetHUDAmmo()
{
	//UE_LOG(LogTemp, Warning, TEXT("AWeapon::SetHUDAmmo()"));
	Blaster = Blaster ? Cast<ABlaster>(GetOwner()) : Blaster;
	if (Blaster && Blaster->IsLocallyControlled())
	{
		if (BlasterController)
		{
			//UE_LOG(LogTemp, Warning, TEXT("AWeapon::SetHUDAmmo(): Blaster & BlasterController are valid"));
			BlasterController->SetHUDWeaponAmmo(Ammo);
		}
		else
		{
			//UE_LOG(LogTemp, Warning, TEXT("AWeapon::SetHUDAmmo(): Blaster Controller is null"));
			BlasterController = BlasterController ? Blaster->GetBlasterController() : BlasterController;
			if (BlasterController)
			{
				BlasterController->SetHUDWeaponAmmo(Ammo);
			}
		}
	}
}

void AWeapon::AddAmmo(int32 AmountToAdd)
{
	Ammo = FMath::Clamp(Ammo - AmountToAdd, 0, MagCapacity);
	SetHUDAmmo();
}

void AWeapon::PlayEquipSound(FVector Location)
{
	if (EquipSound) UGameplayStatics::PlaySoundAtLocation(this, EquipSound, Location);
}

void AWeapon::SetCustomDepthEnabled(bool bEnabled)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnabled);
	}
}

void AWeapon::SpendRound()
{
	if (Ammo == 0) return;

	--Ammo;
	SetHUDAmmo();
}

void AWeapon::OnRep_Ammo()
{
	if (Blaster && Blaster->IsLocallyControlled() && IsFull())
	{
		Blaster->JumpToShotgunReloadEnd();
	}
	SetHUDAmmo();
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}

	const USkeletalMeshSocket* CasingSocket = GetWeaponMesh()->GetSocketByName(FName("AmmoEject"));
	if (CasingSocket && CasingClass)
	{
		FTransform SocketTransform = CasingSocket->GetSocketTransform(GetWeaponMesh());
		ACasing* Spawned = GetWorld()->SpawnActor<ACasing>(CasingClass, SocketTransform.GetLocation(), SocketTransform.GetRotation().Rotator());
		
	}
	SpendRound();
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld,true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
}

