// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/BlasterHUD.h"
#include "GameFramework/PlayerController.h"
#include "HUD/BlasterOverlay.h"
#include "HUD/AnnouncementWidget.h"
#include "HUD/ElimAnnouncement.h"

#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;

	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

		if (HUDPackage.CrosshairsCenter) DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, FVector2D(0.f, 0.f), HUDPackage.CrosshairsColor);
		if (HUDPackage.CrosshairsLeft) DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, FVector2D(-SpreadScaled, 0.f), HUDPackage.CrosshairsColor);
		if (HUDPackage.CrosshairsRight) DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, FVector2D(SpreadScaled, 0.f), HUDPackage.CrosshairsColor);
		if (HUDPackage.CrosshairsTop) DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, FVector2D(0.f, -SpreadScaled), HUDPackage.CrosshairsColor);
		if (HUDPackage.CrosshairsBottom) DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, FVector2D(0.f, SpreadScaled), HUDPackage.CrosshairsColor);
	}
}

void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();
}

void ABlasterHUD::AddCharacterOverlay()
{
	PlayerController = !PlayerController ? GetOwningPlayerController() : PlayerController;
	if (PlayerController && BlasterOverlayClass)
	{
		Overlay = CreateWidget<UBlasterOverlay>(PlayerController, BlasterOverlayClass);
		Overlay->AddToViewport();
	}
}

void ABlasterHUD::AddAnnouncement()
{
	PlayerController = !PlayerController ? GetOwningPlayerController() : PlayerController;
	if (PlayerController && AnnouncementClass && Announcement == nullptr)
	{
		Announcement = CreateWidget<UAnnouncementWidget>(PlayerController, AnnouncementClass);
		Announcement->AddToViewport();
	}
}

void ABlasterHUD::SetAnnouncementVisibility(bool bVisible)
{
	if (Announcement)
	{
		if (bVisible) Announcement->SetVisibility(ESlateVisibility::Visible);
		else Announcement->SetVisibility(ESlateVisibility::Hidden);
	}
}

void ABlasterHUD::AddElimAnnouncement(FString Attacker, FString Victim)
{
	PlayerController = !PlayerController ? GetOwningPlayerController() : PlayerController;

	if (PlayerController && ElimAnnouncementClass)
	{
		UElimAnnouncement* ElimAnnouncementWidget = CreateWidget<UElimAnnouncement>(PlayerController, ElimAnnouncementClass);
		ElimAnnouncementWidget->SetElimAnnouncementText(Attacker, Victim);
		ElimAnnouncementWidget->AddToViewport();

		for (UElimAnnouncement* Msg : ElimMessages)
		{
			if (Msg && Msg->AnnouncementBox)
			{
				UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Msg->AnnouncementBox);
				if (CanvasSlot)
				{
					FVector2D Position = CanvasSlot->GetPosition();
					FVector2D NewPosition(CanvasSlot->GetPosition().X, Position.Y - CanvasSlot->GetSize().Y);
					CanvasSlot->SetPosition(NewPosition);
				}
			}
		}

		ElimMessages.Add(ElimAnnouncementWidget);

		FTimerHandle ElimMsgTimer;
		FTimerDelegate ElimMsgDelegate;
		ElimMsgDelegate.BindUFunction(this, FName("ElimAnnouncementTimerFinished"), ElimAnnouncementWidget);
		GetWorldTimerManager().SetTimer(ElimMsgTimer, ElimMsgDelegate, ElimAnnouncementTime, false);
	}
}

void ABlasterHUD::ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove)
{
	if (MsgToRemove)
	{
		MsgToRemove->RemoveFromParent();
	}
}

void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor Color)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(ViewportCenter.X - (TextureWidth / 2.f) + Spread.X, ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y);

	DrawTexture(Texture, TextureDrawPoint.X, TextureDrawPoint.Y, TextureWidth, TextureHeight, 0.f, 0.f, 1.f, 1.f, Color);
}
