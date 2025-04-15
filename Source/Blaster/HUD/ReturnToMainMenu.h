// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReturnToMainMenu.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UReturnToMainMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	void MenuSetup(); // to set up the widget
	void MenuTearDown(); // To remove the widget

protected:
	virtual bool Initialize() override; // To init internal elements of the widget like button callbacks ( delegates ), etc ...

	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);

	UFUNCTION()
	void OnPlayerLeftGame();
private:
	UPROPERTY(meta = (BindWidget))
	class UButton* ReturnButton;

	UFUNCTION()
	void ReturnButtonClicked();

	UPROPERTY()
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	UPROPERTY()
	class APlayerController* PlayerController;
};