// Fill out your copyright notice in the Description page of Project Settings.


#include "SPlayerController.h"
#include "Blueprint/UserWidget.h"

void ASPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	InputComponent->BindAction("PauseMenu",IE_Pressed,this,&ASPlayerController::TogglePauseMenu);
}

void ASPlayerController::TogglePauseMenu()
{
	//if have already has Pause_UI
	if (PauseMenuWidgetInstance && PauseMenuWidgetInstance->IsInViewport())
	{
		PauseMenuWidgetInstance->RemoveFromParent();
		//prevent use old UI,much safer
		PauseMenuWidgetInstance = nullptr;
		
		bShowMouseCursor = false;
		SetInputMode(FInputModeGameOnly());
		return;
	}
	
	//if haven't initialize yet
	PauseMenuWidgetInstance = CreateWidget<UUserWidget>(this,PauseMenuWidget);
	if (PauseMenuWidgetInstance)
	{
		PauseMenuWidgetInstance->AddToViewport();
		
		bShowMouseCursor = true;
		SetInputMode(FInputModeUIOnly());
	}
}


void ASPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	OnPawnChanged.Broadcast(InPawn);
}

void ASPlayerController::BeginPlayingState()
{
	BlueprintBeginPlayingState();
}

void ASPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	OnPlayerStateChanged.Broadcast(PlayerState);
}
