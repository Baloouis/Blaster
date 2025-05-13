// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"

#include "ElimAnnouncement.h"
#include "Announcement.h"
#include "CharacterOverlay.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "GameFramework/PlayerController.h"

void ABlasterHUD::BeginPlay()
{
    Super::BeginPlay();
    
}

void ABlasterHUD::AddAnnouncement()
{
    APlayerController* PlayerController = GetOwningPlayerController();
    if (PlayerController && AnnouncementClass && Announcement  == nullptr)
    {
        Announcement = CreateWidget<UAnnouncement>(PlayerController, AnnouncementClass);
        Announcement->AddToViewport();
    }
}

void ABlasterHUD::AddElimAnnouncement(FString Attacker, FString Victim)
{
    OwningPlayer = OwningPlayer == nullptr ? GetOwningPlayerController() : OwningPlayer;
    if (OwningPlayer && ElimAnnouncementClass)
    {
        UElimAnnouncement* ElimAnnouncementWidget  = CreateWidget<UElimAnnouncement>(OwningPlayer, ElimAnnouncementClass);
        if (ElimAnnouncementWidget )
        {
            ElimAnnouncementWidget->SetElimAnnouncementText(Attacker, Victim);
            ElimAnnouncementWidget->AddToViewport();

            //Move all the previous UElimAnnouncement up before adding new one
            for (UElimAnnouncement* Msg : ElimMessages)
            {
                if (Msg && Msg->AnnouncementBox)
                {
                    //We need to move up the previous announcement by their height which we can get by getting CanvasSlot
                    //The slot is also useful to actually move the widget
                    UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Msg->AnnouncementBox);
                    if (CanvasSlot)
                    {
                        FVector2D Position = CanvasSlot->GetPosition();
                        FVector2D NewPosition(
                            CanvasSlot->GetPosition().X,
                            Position.Y - CanvasSlot->GetSize().Y //Moving widget up by its height
                        );
                        CanvasSlot->SetPosition(NewPosition);
                    }
                }
            }
            
            //Then add new UElimAnnouncement
            ElimMessages.Add(ElimAnnouncementWidget);

            //Set the Timer and callback to ElimAnnouncementTimerFinished method when the timer is done
            FTimerHandle ElimMsgTimer;
            FTimerDelegate ElimMsgDelegate;
            ElimMsgDelegate.BindUObject(this, &ThisClass::ElimAnnouncementTimerFinished, ElimAnnouncementWidget);
            GetWorldTimerManager().SetTimer(
                ElimMsgTimer,
                ElimMsgDelegate,
                ElimAnnouncementTime,
                false
            );
        }
    }
}

void ABlasterHUD::ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove)
{
    if (MsgToRemove)
    {
        MsgToRemove->RemoveFromParent();
        ElimMessages.Remove(MsgToRemove);
    }
}

void ABlasterHUD::AddCharacterOverlay()
{
    APlayerController* PlayerController = GetOwningPlayerController();
    if (PlayerController && CharacterOverlayClass)
    {
        CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
        CharacterOverlay->AddToViewport();
    }
}

void ABlasterHUD::DrawHUD()
{
    Super::DrawHUD();

    FVector2D ViewportSize;
    if (GEngine)
    {
        GEngine->GameViewport->GetViewportSize(ViewportSize);
        const FVector2D ViewportCenter(ViewportSize.X /2.f, ViewportSize.Y /2.f);

        float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;
        if (HUDPackage.CrosshairsCenter)
        {
            DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, FVector2D::Zero(), HUDPackage.CrosshairsColor);
        }
        if (HUDPackage.CrosshairsRight)
        {
            FVector2D Spread(SpreadScaled, 0.f);
            DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
        }
        if (HUDPackage.CrosshairsLeft)
        {
            FVector2D Spread(-SpreadScaled, 0.f);

            DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
        }
        if (HUDPackage.CrosshairsTop)
        {
            // Y value is set to negative because in UV space upward is the negative Y direction
            FVector2D Spread(0.f, -SpreadScaled); 
            DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
        }
        if (HUDPackage.CrosshairsBottom) 
        {
            // Y value is set to positive because in UV space downward is the positive Y direction
            FVector2D Spread(0.f, SpreadScaled);
            DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
        } 
    }
}


void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor)
{
    const float TextureWidth = Texture->GetSizeX();
    const float TextureHeight = Texture->GetSizeY();
    // If we dont offset the DrawPoint from the ViewportCenter, then the top left corner would be placed at this position
    // That is why we offset by half its width and height
    const FVector2D TextureDrawPoint(
        ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
        ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y
        );
    DrawTexture(
        Texture,
        TextureDrawPoint.X,
        TextureDrawPoint.Y,
        TextureWidth,
        TextureHeight,
        0.f,
        0.f,
        1.f,
        1.f,
        CrosshairColor
        );
}

