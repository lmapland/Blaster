// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:
	UTexture2D* CrosshairsCenter;
	UTexture2D* CrosshairsLeft;
	UTexture2D* CrosshairsRight;
	UTexture2D* CrosshairsTop;
	UTexture2D* CrosshairsBottom;

	float CrosshairSpread;
	FLinearColor CrosshairsColor;
};

class UBlasterOverlay;
class UAnnouncementWidget;
class UElimAnnouncement;
class APlayerController;

/**
 * 
 */
UCLASS()
class MPTESTING_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	virtual void DrawHUD() override;
	void AddCharacterOverlay();
	void AddAnnouncement();
	void SetAnnouncementVisibility(bool bVisible);
	void AddElimAnnouncement(FString Attacker, FString Victim);

	UPROPERTY(EditAnywhere, Category = "HUD|Player Stats")
	TSubclassOf<UUserWidget> BlasterOverlayClass;

	UPROPERTY()
	UBlasterOverlay* Overlay;

	UPROPERTY(EditAnywhere, Category = "HUD|Announcements")
	TSubclassOf<UUserWidget> AnnouncementClass;
	
	UPROPERTY()
	UAnnouncementWidget* Announcement;

protected:
	virtual void BeginPlay() override;

private:
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor Color);

	UFUNCTION()
	void ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove);

	FHUDPackage HUDPackage;

	UPROPERTY(EditAnywhere, Category = "HUD|Crosshair")
	float CrosshairSpreadMax = 16.f;
	
	UPROPERTY(EditAnywhere, Category = "HUD|Announcements")
	TSubclassOf<UElimAnnouncement> ElimAnnouncementClass;

	APlayerController* PlayerController;

	UPROPERTY(EditAnywhere, Category = "HUD|Announcements")
	float ElimAnnouncementTime = 5.f;

	UPROPERTY()
	TArray<UElimAnnouncement*> ElimMessages;

public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};
