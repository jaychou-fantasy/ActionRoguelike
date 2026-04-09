// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "SSaveGame.h"
#include "SGameModeBase.generated.h"


class ASCharacter;
class UEnvQueryInstanceBlueprintWrapper;
class UEnvQuery;
class UCurveFloat;
class USSaveGame;

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API ASGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	

protected:
	FString SlotName;

	UPROPERTY()
	USSaveGame* CurrentSaveGame;


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
	//����potion��powerup_credits
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
	// Cannot use "class" to declare this because of the type �� the entire namespace must be included

	UFUNCTION()
	void OnPowerupSpawnQueryCompleted(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus);

	UFUNCTION()
	void RespawnPlayerElapsed(AController* Controller,ASCharacter* SCharacter);
	

	UFUNCTION(Exec)
	void KillAll();

public:
	//really really early,even is before BeginPlay()
	void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)override;
	
	//the parent function is "BlueprintNativeEvent",so _implementation
	//a function calls when any new player logging in the game
	void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)override;
	
	ASGameModeBase();

	virtual void OnActorKilled(AActor* VictimActor, AActor* Killer);
	// This is a function that doesn't exist in the parent class. 
	// I use virtual to tell the compiler that subclasses of ASGameMode can override it in the future

	virtual void StartPlay() override;


	UFUNCTION(BlueprintCallable,Category = "SaveGame")
	void WriteSaveGame();

	void LoadSaveGame();
	void LoadActorData();

};
