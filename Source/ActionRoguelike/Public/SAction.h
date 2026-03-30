// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "SAction.generated.h"

class UWorld;
/**
 * 
 */

USTRUCT()
struct FActionRepData
{
	GENERATED_BODY()

public:

	UPROPERTY()
	bool bIsRunning;

	// This way, when the server plays the projectile animation, the client will display not only the projectile itself, but also the instigator's action.
	// Because PlayAnimMontage is executed on the instigator.
	// Previously, only bIsRunning was replicated, but the instigator wasn't passed along, resulting in the projectile being visible without the corresponding action animation.
	UPROPERTY()
	AActor* Instigator;
};


// Blueprintable indicates that this class can have child classes created in Blueprint ¡ª for example, creating a subclass of BTTaskNode.
// In this case, BTTaskNode itself is marked as Blueprintable.UCLASS(Blueprintable)
class ACTIONROGUELIKE_API USAction : public UObject
{
	GENERATED_BODY()
	

protected:
	// Components that absolutely need to participate in replication or Blueprint processes must be marked as UPROPERTY
	UPROPERTY(Replicated)
	USActionComponent* ActionComp;

	/* Tags added to owning actor when activated, removed when action stops */
	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTagContainer GrantsTags;

	/* Action can only start if OwningActor has none of these Tags applied */
	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTagContainer BlockedTags;


	UPROPERTY(ReplicatedUsing = "OnRep_RepData")
	FActionRepData RepData;

	// Currently, actions from the client can reach the server, but changes to actions on the server cannot propagate to the client
	UFUNCTION()
	void OnRep_RepData();

public:
	void Initialize(USActionComponent* NewActionComp);

	UPROPERTY(EditDefaultsOnly,Category = "Action")
	bool bAutoStart;

	UFUNCTION(BlueprintCallable,Category = "Action")
	bool IsRunning() const;

	UFUNCTION(BlueprintNativeEvent,Category = "Action")
	bool CanStart(AActor* Instigator);

	UFUNCTION(BlueprintCallable,Category = "Action")
	USActionComponent* GetOwningComponent() const;

	// Any function marked as a BlueprintNativeEvent must have a default implementation in C++ ¡ª that is, the _Implementation function
	UFUNCTION(BlueprintNativeEvent,Category = "Action")
	void StartAction(AActor* Instigator);

	UFUNCTION(BlueprintNativeEvent,BlueprintCallable, Category = "Action")
	void StopAction(AActor* Instigator);


	UPROPERTY(EditDefaultsOnly,Category = "Action")
	FName ActionName;

	UWorld* GetWorld() const override;

	// Virtual doesn't need to be written ¡ª when we start overriding, we can just use the override keyword
	//
	// Only UObject requires this; components derived from UActorComponent don't need it (but they do use SetIsReplicatedByDefault)
	// 
	// This function returns false by default (networking disabled)
	// Changing it to true enables networking
		bool IsSupportedForNetworking() const override
	{
		return true;
	}
};
