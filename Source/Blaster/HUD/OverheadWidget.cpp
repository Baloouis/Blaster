// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"

#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UOverheadWidget::SetDisplayText(FString TextToDisplay)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	ENetRole NetRole = InPawn->GetRemoteRole();
	FString Role;
	switch (NetRole)
	{
		case ENetRole::ROLE_Authority:
			Role = "Authority";
			break;
		case ENetRole::ROLE_AutonomousProxy:
			Role = "Autonomous Proxy";
			break;
		case ENetRole::ROLE_SimulatedProxy:
			Role = "Simulated Proxy";
			break;
		case ENetRole::ROLE_None:
			Role = "None";
			break;
	}
	FString PlayerName;
	if (APlayerState* PlayerState = InPawn->GetPlayerState())
	{
		PlayerName = PlayerState->GetPlayerName();
	}
	FString RoleString = FString::Printf(TEXT("%s Remote Role: %s"), *PlayerName, *Role);
	SetDisplayText(RoleString);
}

void UOverheadWidget::NativeDestruct()
{
	RemoveFromParent();
	Super::NativeDestruct();
}
