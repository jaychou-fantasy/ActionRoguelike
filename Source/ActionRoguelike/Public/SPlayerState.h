// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SPlayerState.generated.h"

// DELEGATE must be declared with UPROPERTY — and BlueprintAssignable — otherwise it cannot be used in Blueprint
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCreditsChanged, ASPlayerState*, PlayerState, int32, NewCredits, int32, Delta);
// Create a delegate to be broadcast later. Then, on the function that needs to receive this information, use AddDynamic to bind it.

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API ASPlayerState : public APlayerState
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly,Category = "Credits")
	int32 Credits;

public:
	UFUNCTION(BlueprintCallable,Category = "Credits")
	int32 GetCredits() const;

	UFUNCTION(BlueprintCallable,Category = "Credits") // <Category|SubCategory
	void AddCredits(int32 Delta);

	// Since this is used to check conditions — for example, being able to subtract 50 credits before purchasing — it needs to return a bool for validation
	UFUNCTION(BlueprintCallable, Category = "Credits")
	bool RemoveCredits(int32 Delta);

	UPROPERTY(BlueprintAssignable,Category = "Evenet")
	FOnCreditsChanged OnCreditsChanged;


};
