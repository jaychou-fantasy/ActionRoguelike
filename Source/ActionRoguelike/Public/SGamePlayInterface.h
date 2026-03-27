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
class ACTIONROGUELIKE_API ISGamePlayInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	
	
	
	
	UFUNCTION(BlueprintCallable,BlueprintNativeEvent)//means you can implement it in blueprint as well as c++//but BlueprintImplementableEvent means you can only implement it in blueprint
	//callable means you can call it in blueprint,otherwise you can only get it but cant call(use)it in blueprint;
		void Interact(APawn* InstigatorPawn);

	//反正都是override，所以不需要原定义





};
