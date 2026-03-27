// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "SGameModeBase.generated.h"

class UEnvQueryInstanceBlueprintWrapper;
class UEnvQuery;
class UCurveFloat;

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API ASGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	

protected:

	FTimerHandle TimerHandle_SpawnBot;
	
	UPROPERTY(EditDefaultsOnly,Category = "AI")
	float SpawnTimerInterval;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	UEnvQuery* SpawnBotQuery;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	UCurveFloat* DifficultyCurve;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TSubclassOf<AActor> MinionClass;
	

	// Read/write access as we could change this as our difficulty increases via Blueprint
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category = "AI")
	int32 CreditsPerKill;

	/* Distance required between power-up spawn locations */
	UPROPERTY(EditDefaultsOnly,Category = "Powerups")
	float RequiredPowerupDistance;

	/* Amount of powerups to spawn during match start */
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	int32 DesiredPowerupCount;

	/* All power-up classes used to spawn with EQS at match start */
	//比如potion和powerup_credits
	UPROPERTY(EditDefaultsOnly,Category = "Powerups")
	TArray<TSubclassOf<AActor>> PowerupClasses;

	UPROPERTY(EditDefaultsOnly,Category = "Powerups")
	UEnvQuery* PowerupSpawnQuery;



	UFUNCTION()
	void SpawnBotTimeElapsed();
	UFUNCTION()
	void SpawnPowerupTimeElapsed();
	
	UFUNCTION()
	void OnBotSpawnQueryCompleted(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus);
	//不能用class来申明，因为type这个，所以必须涵盖整个namespace

	UFUNCTION()
	void OnPowerupSpawnQueryCompleted(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus);

	UFUNCTION()
	void RespawnPlayerElapsed(AController* Controller,ASCharacter* SCharacter);
	

	UFUNCTION(Exec)
	void KillAll();

public:

	ASGameModeBase();

	virtual void OnActorKilled(AActor* VictimActor, AActor* Killer);
	//这个是父类没有的函数，我用vritual是告诉编译器，以后asgame mode的子类可以override它

	virtual void StartPlay() override;

};
