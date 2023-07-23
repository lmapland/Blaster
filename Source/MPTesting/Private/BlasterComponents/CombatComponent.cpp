// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterComponents/CombatComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Characters/Blaster.h"
#include "Weapons/Weapon.h"
#include "Net/UnrealNetwork.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UCombatComponent, EquippedWeapon, COND_OwnerOnly);
}

void UCombatComponent::EquipWeapon(AWeapon* ToEquip)
{
	check(Character);
	if (!ToEquip) return;

	// attach wep to socket
	EquippedWeapon = ToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
	EquippedWeapon->SetOwner(Character); // Note that SetOwner)() is provided by Actor and is replicated
}

