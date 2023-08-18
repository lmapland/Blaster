// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/HealthPickup.h"
#include "Characters/Blaster.h"
#include "BlasterComponents/BuffComponent.h"

AHealthPickup::AHealthPickup()
{
	bReplicates = true;
}

void AHealthPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlaster* Blaster = Cast<ABlaster>(OtherActor);
	if (Blaster)
	{
		UBuffComponent* BuffComponent = Blaster->GetBuffComponent();
		if (BuffComponent)
		{
			BuffComponent->HealOver(HealAmount, HealingTime);
		}
	}

	Destroy();
}
