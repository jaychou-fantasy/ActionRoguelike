// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SProjectileBase.h"
#include "SDashProjectile.generated.h"

UCLASS()
class ACTIONROGUELIKE_API ASDashProjectile : public ASProjectileBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASDashProjectile();

protected:
	
	UPROPERTY(EditDefaultsOnly,Category = "Teleport")
	float TeleportDelay;

	UPROPERTY(EditDefaultsOnly,Category = "Teleport")
	float DetonateDelay;//dedonate = explode



	// Handle to cancel timer if we already hit something
	FTimerHandle TimerHandle_DelayedDetonate;// Timer handle ¡ª used to control or query a timer, such as canceling it, checking if it's active, etc.

	// Base class using BlueprintNativeEvent, we must override the _Implementation not the Explode()
	virtual void Explode_Implementation() override;
	
	void TeleportInstigator();

	virtual void BeginPlay() override;

};
