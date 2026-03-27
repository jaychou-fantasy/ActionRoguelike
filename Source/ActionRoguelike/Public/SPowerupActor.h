// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "SGamePlayInterface.h"

#include "SPowerupActor.generated.h"

class USphereComponent;
class UStaticMeshComponent;

//就是只能用来作为父类，自己不能产生作用
UCLASS(Abstract)
class ACTIONROGUELIKE_API ASPowerupActor : public AActor, public ISGamePlayInterface
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(VisibleAnywhere,Category = "Components")
	USphereComponent* SphereComp;
	
	UPROPERTY(VisibleAnywhere,Category = "Components")
	UStaticMeshComponent* MeshComp;
	
	UPROPERTY(EditAnywhere,Category = "Powerup")
	float RespawnTime;

	

	FTimerHandle TimerHandle_RespawnTimer;

	UFUNCTION()
	void ShowPowerup();


	void HideAndCooldownPowerup();
	void SetPowerupState(bool bNewIsActive);



public:
	
	void Interact_Implementation(APawn* InstigatorPawn) override;



public:	
	// Sets default values for this actor's properties
	ASPowerupActor();


};
