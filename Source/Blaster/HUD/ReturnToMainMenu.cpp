// Fill out your copyright notice in the Description page of Project Settings.


#include "ReturnToMainMenu.h"
#include "GameFramework/PlayerController.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "GameFramework/GameModeBase.h"

void UReturnToMainMenu::MenuSetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);

	UWorld* World = GetWorld();
	if (World)
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController)
		{
			FInputModeGameAndUI InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			PlayerController->SetInputMode(InputModeData);

			//PlayerController->SetInputMode(FInputModeUIOnly());
			
			PlayerController->SetShowMouseCursor(true);
		}
	}
	if (ReturnButton && !ReturnButton->OnClicked.IsAlreadyBound(this, &UReturnToMainMenu::ReturnButtonClicked))
	{
		ReturnButton->OnClicked.AddDynamic(this, &UReturnToMainMenu::ReturnButtonClicked);
	}
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		
		if (MultiplayerSessionsSubsystem && !MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.IsAlreadyBound(this, &UReturnToMainMenu::OnDestroySession))
		{
			MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UReturnToMainMenu::OnDestroySession);
		}
	}
}


void UReturnToMainMenu::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
	if (ReturnButton && ReturnButton->OnClicked.IsAlreadyBound(this, &UReturnToMainMenu::ReturnButtonClicked))
	{
		ReturnButton->OnClicked.RemoveDynamic(this, &UReturnToMainMenu::ReturnButtonClicked);
	}
	if (MultiplayerSessionsSubsystem && MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.IsAlreadyBound(this,&UReturnToMainMenu::OnDestroySession))
	{
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &UReturnToMainMenu::OnDestroySession);
	}
}

bool UReturnToMainMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	return true;
}

void UReturnToMainMenu::OnDestroySession(bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		ReturnButton->SetIsEnabled(true);
		return;
	}

	UWorld* World = GetWorld();
	if (World)
	{
		AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>();
		if (GameMode) // if GameMode is valid then we are on the Server
		{
			GameMode->ReturnToMainMenuHost();
		}
		else // we're a Client
		{
			PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
			if (PlayerController)
			{
				PlayerController->ClientReturnToMainMenuWithTextReason(FText()); // using WithTextReason version of this method to avoid the UE warning
			}
		}
	}
}


void UReturnToMainMenu::ReturnButtonClicked()
{
	ReturnButton->SetIsEnabled(false);

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* FirstPlayerController = World->GetFirstPlayerController();
		if (FirstPlayerController)
		{
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FirstPlayerController->GetPawn());
			if (BlasterCharacter)
			{
				BlasterCharacter->ServerLeaveGame();
				BlasterCharacter->OnLeftGame.AddDynamic(this, &UReturnToMainMenu::OnPlayerLeftGame);
			}
			else //The player character is in the middle of being respawned, so we nothing but let the player try again after respawning by re-enabling button
			{
				ReturnButton->SetIsEnabled(true);
			}
		}
	}
}

void UReturnToMainMenu::OnPlayerLeftGame()
{
	UE_LOG(LogTemp, Warning, TEXT("OnPlayerLeftGame()"))
	if (MultiplayerSessionsSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("MultiplayerSessionsSubsystem valid"))
		MultiplayerSessionsSubsystem->DestroySession();
	}
}