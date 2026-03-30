// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SAction.h"
#include "SActionEffect.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API USActionEffect : public USAction
{
	GENERATED_BODY()
	
public:
	// Overridden functions don't need UFUNCTION specifiers.
	// For BlueprintNativeEvent functions, you can directly override the _Implementation version
	void StartAction_Implementation(AActor* Instigator) override;

	void StopAction_Implementation(AActor* Instigator) override;

protected:
	
	FTimerHandle PeriodHandle;
	FTimerHandle DurationHandle;

	UPROPERTY(EditDefaultsOnly,Category = "Effect")
	float Period;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	float Duration;


	UFUNCTION(BlueprintNativeEvent,Category = "Effect")
	void ExecutePeriodicEffect(AActor* Instigator);

public:
	USActionEffect();
};
