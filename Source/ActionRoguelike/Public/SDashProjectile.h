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
	FTimerHandle TimerHandle_DelayedDetonate;
	//计时器的句柄（TimerHandle），用来控制或查询定时器，比如取消它、检查是否激活等

	// Base class using BlueprintNativeEvent, we must override the _Implementation not the Explode()
	virtual void Explode_Implementation() override;
	
	void TeleportInstigator();

	virtual void BeginPlay() override;

};
