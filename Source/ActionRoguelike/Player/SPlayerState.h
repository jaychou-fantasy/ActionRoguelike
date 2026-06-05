// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SPlayerState.generated.h"

// DELEGATE must be declared with UPROPERTY — and BlueprintAssignable — otherwise it cannot be used in Blueprint
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCreditsChanged, ASPlayerState*, PlayerState, int32, NewCredits, int32, Delta);
// Create a delegate to be broadcast later. Then, on the function that needs to receive this information, use AddDynamic to bind it.

class USSaveGame;

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API ASPlayerState : public APlayerState
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly,ReplicatedUsing = "OnRep_Credits",Category = "Credits")
	int32 Credits;
	
	// OnRep_ can use a parameter containing the 'old value' of the variable it is bound to. Very useful in this case to figure out the 'delta'.
	
	//OnRep Function can only at most pass one parameter--->its previous value(name depend on itself)
	//so if your multicast has only one parameter and it's its old value,then raplace it with OnRep Function
	UFUNCTION()
	void OnRep_Credits(int32 OldCredits);
	
	// Downside of using multicast here is that we send over more data over the net, since it's an RPC with two parameters. OnRep_ is "free" since Credits is already getting replicated anyway.
	//UFUNCTION(NetMulticast, Unreliable)
	//void MulticastCredits(float NewCredits, float Delta);

public:
	ASPlayerState();
	
	UFUNCTION(BlueprintNativeEvent)
	void SavePlayerState(USSaveGame* SaveObject);
	
	UFUNCTION(BlueprintNativeEvent)
	void LoadPlayerState(USSaveGame* SaveObject);
	
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
