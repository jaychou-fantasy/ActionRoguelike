// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "SPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPawnChanged, APawn*, NewPawn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStateChanged,APlayerState*,NewPlayerState);
/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API ASPlayerController : public APlayerController
{
	GENERATED_BODY()


protected:
	UPROPERTY(EditDefaultsOnly,Category = "UI")
	TSubclassOf<UUserWidget> PauseMenuWidget;
	
	UPROPERTY()
	UUserWidget* PauseMenuWidgetInstance;
	
	UFUNCTION(BlueprintCallable)
	void TogglePauseMenu();
	
	UFUNCTION()
	void SetupInputComponent() override;
	
	UPROPERTY(BlueprintAssignable)
	FOnPawnChanged OnPawnChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnPlayerStateChanged OnPlayerStateChanged;
	
	//actually, virtual is not necessary,override has already include that meaning
	virtual void SetPawn(APawn* InPawn) override;
	
	
	/* Called when player controller is ready to begin playing, good moment to initialize things like UI which might be too early in BeginPlay 
	(esp. in multiplayer clients where not all data such as PlayerState may have been received yet) */
	UFUNCTION()
	virtual void BeginPlayingState() override;
	//c++ version to controll function life span
	//blueprint version to assign UI accomplishment
	UFUNCTION(BLueprintImplementableEvent)
	void BlueprintBeginPlayingState();
	
	//called when playerstate was replicated to client
	//to let any comp that needs playerstate to know that::multiplayer was ready to play
	void OnRep_PlayerState() override;
};
