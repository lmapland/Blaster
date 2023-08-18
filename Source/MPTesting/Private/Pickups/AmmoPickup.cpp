// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/AmmoPickup.h"
#include "Characters/Blaster.h"
#include "BlasterComponents/CombatComponent.h"

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlaster* Blaster = Cast<ABlaster>(OtherActor);
	if (Blaster)
	{
		UCombatComponent* CombatComponent = Blaster->GetCombatComponent();
		if (CombatComponent)
		{
			CombatComponent->PickupAmmo(WeaponType, AmmoAmount);
		}
	}

	Destroy();
}
