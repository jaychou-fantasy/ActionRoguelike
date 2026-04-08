// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SGamePlayInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USGamePlayInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */

//ISGamePlayInterface::Execute_Interact(InFocus, MyPawn);
//only this way use I
//otherway use U
class ACTIONROGUELIKE_API ISGamePlayInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	/* Called after the Actor state（not player_state,actor will be loaded as initgame） was restored from a SaveGame file. */
	UFUNCTION(BlueprintNativeEvent)
	void OnActorLoaded();
	
	
	UFUNCTION(BlueprintCallable,BlueprintNativeEvent)//means you can implement it in blueprint as well as c++//but BlueprintImplementableEvent means you can only implement it in blueprint
	//callable means you can call it in blueprint,otherwise you can only get it but cant call(use)it in blueprint;
	void Interact(APawn* InstigatorPawn);

	// Since we're overriding, the original definition is not needed





};
