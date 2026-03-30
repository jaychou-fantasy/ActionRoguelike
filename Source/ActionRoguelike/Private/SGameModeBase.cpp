// Fill out your copyright notice in the Description page of Project Settings.


#include "SGameModeBase.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "EnvironmentQuery/EnvQueryInstanceBlueprintWrapper.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EngineUtils.h"
#include "AI/SAICharacter.h"
#include "SAttributeComponent.h"
#include "Curves/CurveFloat.h"
#include "DrawDebugHelpers.h"
#include "SCharacter.h"
#include "SPlayerState.h"



static TAutoConsoleVariable<bool> CVarSpawnBots(TEXT("su.SpawnBots"),true,TEXT("Enable spawning of bots via timer."),ECVF_Cheat);



ASGameModeBase::ASGameModeBase()
{
	// This PlayerStateClass is a built-in option in GameMode ¡ª the kind you select in Blueprint by choosing which State Class to assign
	PlayerStateClass = ASPlayerState::StaticClass();

	SpawnTimerInterval = 2.0f;
	CreditsPerKill = 20;

	DesiredPowerupCount = 10;
	RequiredPowerupDistance = 2000.0f;
}


void ASGameModeBase::KillAll()
{
	for (TActorIterator<ASAICharacter> It(GetWorld()); It; ++It)
	{
		ASAICharacter* Bot = *It;
		USAttributeComponent* AttributeComp = USAttributeComponent::GetAttributes(Bot);
		if (ensure(AttributeComp) && AttributeComp->IsAlive())
		{
			AttributeComp->Kill(this);//@fixme : add kill character for credits
		}
	}
}

void ASGameModeBase::StartPlay()
{
	Super::StartPlay();

	// Actually, this SpawnBotTimeElapsed could be written directly in StartPlay, but doing it this way is a bit clearer
	GetWorldTimerManager().SetTimer(TimerHandle_SpawnBot, this, &ASGameModeBase::SpawnBotTimeElapsed, SpawnTimerInterval, true);//trueÊÇBloop
	
	SpawnPowerupTimeElapsed();
}

void ASGameModeBase::SpawnBotTimeElapsed()
{
	// If the console sets it to 0, then just return and don't allow spawning
	if (!CVarSpawnBots.GetValueOnGameThread())
	{
		UE_LOG(LogTemp,Warning,TEXT("Bot spawming disabled via cvar 'CVarSapwnBots'."))
		return;
	}
	
	//*limitation*
	int32 NrOfAliveBots = 0;
	for (TActorIterator<ASAICharacter> It(GetWorld()); It; ++It)
	{
		ASAICharacter* Bot = *It;
		USAttributeComponent* AttributeComp = USAttributeComponent::GetAttributes(Bot);
		if (ensure(AttributeComp) && AttributeComp->IsAlive())
		{
			NrOfAliveBots++;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Found %i alive bots."), NrOfAliveBots);

	float MaxBotCount = 10.0f;
	if (DifficultyCurve)
	{
		MaxBotCount = DifficultyCurve->GetFloatValue(GetWorld()->TimeSeconds);
		UE_LOG(LogTemp, Log, TEXT("At maximun bot = %f"),MaxBotCount);
	}
	if (NrOfAliveBots >= MaxBotCount)
	{
		UE_LOG(LogTemp, Warning, TEXT("At maximun bot capacity.Skipping bot spawn."));
		return;
	}

	// First, check if it's necessary to run the EQS query
	// Because running EQS is very expensive


	UEnvQueryInstanceBlueprintWrapper* QueryInstance = UEnvQueryManager::RunEQSQuery(this, SpawnBotQuery, this, EEnvQueryRunMode::RandomBest5Pct, nullptr); // The last parameter (wrapper) never seems to be used
	// You can pass in an existing EQS Query Wrapper to reuse it; if you pass nullptr, the system will automatically create a new Query Wrapper
	// The result from the query will be broadcast via the Query Wrapper's OnQueryFinishedEvent delegate
	if (ensure(QueryInstance))
	{
		QueryInstance->GetOnQueryFinishedEvent().AddDynamic(this, &ASGameModeBase::OnBotSpawnQueryCompleted);
	}
}

void ASGameModeBase::OnBotSpawnQueryCompleted(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus)
{
	UE_LOG(LogTemp,Log, TEXT("OnQueryCompleted status=%d"), (int)QueryStatus);

	if (QueryStatus != EEnvQueryStatus::Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spawn bot EQS Query Failed!"));

		return;
	}

	TArray<FVector> Locations = QueryInstance->GetResultsAsLocations();
	UE_LOG(LogTemp, Warning, TEXT("EQS returned %d locations."), Locations.Num());
	if (Locations.IsValidIndex(0))// .Num() > 0 would also work, since we only need to call one location from the results
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		GetWorld()->SpawnActor<AActor>(MinionClass, Locations[0], FRotator::ZeroRotator,SpawnParams);

		DrawDebugSphere(GetWorld(), Locations[0], 50.0f, 20, FColor::Blue, false, 60.0f);
	}
}

//spawn power_ups
void ASGameModeBase::SpawnPowerupTimeElapsed()
{
	UE_LOG(LogTemp, Log, TEXT("SpawnPowerupTimeElapsed CALLED"));
	//make sure we have assigned at least one powerup class to spawn
	if (ensure(PowerupClasses.Num() > 0))
	{
		UE_LOG(LogTemp, Log, TEXT("PowerupClasses count: %d"), PowerupClasses.Num());

		// nullptr refers to the returned wrapper class, but what we need is UEnvQueryInstanceBlueprintWrapper, so we'll just use nullptr without adding anything extra
		UEnvQueryInstanceBlueprintWrapper* QueryInstance = UEnvQueryManager::RunEQSQuery(this, PowerupSpawnQuery, this, EEnvQueryRunMode::AllMatching, nullptr);
		if (ensure(QueryInstance))
		{
			UE_LOG(LogTemp, Log, TEXT("EQS Query Started"));

			// When the query finishes, it's a delegate ¡ª once completed, it transmits information. Our new function needs this data, so we use AddDynamic
			QueryInstance->GetOnQueryFinishedEvent().AddDynamic(this, &ASGameModeBase::OnPowerupSpawnQueryCompleted);
		}
	}
}

void ASGameModeBase::OnPowerupSpawnQueryCompleted(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus)
{
	// First, check the query status
	if (QueryStatus != EEnvQueryStatus::Success)
	{
		UE_LOG(LogTemp, Error, TEXT("Spawn Power up EQS Query Failed! Status: %d"), (int)QueryStatus);
		return;
	}
	// The most important step: extract EQS results as an array of locations
	TArray<FVector> Locations = QueryInstance->GetResultsAsLocations();
	UE_LOG(LogTemp, Warning, TEXT("Locations found: %d"), Locations.Num());

	// Keep track of used locations to easily check distance between points
	TArray<FVector> UsedLocations;

	// In bot spawning: first check if the number of bots is below the limit in the time elapsed function, then run EQS + on query completed
	int32 SpawnCounter = 0;

	while (SpawnCounter < DesiredPowerupCount && Locations.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Entering Spawn Loop"));
		// Pick a random location from the remaining points
		int32 RandomLocationIndex = FMath::RandRange(0, Locations.Num() - 1); // RandRange is inclusive; a 5-element array contains indices [0,4]
		FVector PickedLocation = Locations[RandomLocationIndex];

		// Remove to avoid picking again
		Locations.RemoveAt(RandomLocationIndex);

		// Check minimum distance requirement; if accepted, spawn power-up at the picked location
		bool bValidLocation = true;
		// The first picked location will be spawned directly because UsedLocations is empty, so the for loop is skipped
		for (FVector OtherLocation : UsedLocations)
		{
			float DistanceTo = (PickedLocation - OtherLocation).Size(); // Size() calculates the distance between the two 3D vectors
			if (DistanceTo < RequiredPowerupDistance)
			{
				// Show skipped location due to distance
				// DrawDebugSphere(GetWorld(), PickedLocation, 0.0f, 20, FColor::Red, false, 10.0f);

				// Too close, skip to next attempt
				bValidLocation = false;
				break;
			}
		}

		// Failed the distance test
		if (!bValidLocation)
		{
			continue;
			// Jump directly to the next random picked location
		}

		// Pick a random power-up class
		int32 RandomClassIndex = FMath::RandRange(0, PowerupClasses.Num() - 1);
		TSubclassOf<AActor> RandomPowerupClass = PowerupClasses[RandomClassIndex];
		if (RandomClassIndex)
		{
			UE_LOG(LogTemp, Error, TEXT("RandomPowerupClass is NULL"));
		}

		if (AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(RandomPowerupClass, PickedLocation, FRotator::ZeroRotator))   // The SpawnParameter uses default values ¡ª what's the difference?
		{
			UE_LOG(LogTemp, Log, TEXT("Spawn SUCCESS: %s"), *SpawnedActor->GetName());
		}

		// Keep for distance check
		UsedLocations.Add(PickedLocation);
		SpawnCounter++;
	}

}


// Respawn character
void ASGameModeBase::OnActorKilled(AActor* VictimActor, AActor* Killer)
{
	UE_LOG(LogTemp, Log, TEXT("OnActorKilled: Victim: %s, Killer: %s"), *GetNameSafe(VictimActor), *GetNameSafe(Killer));
	// The * operator dereferences to get all characters in the FString

	ASCharacter* Player = Cast<ASCharacter>(VictimActor);
	if (Player)
	{
		FTimerHandle TimerHandle_RespawnDelay;
		// Must be localized so that each player's TimerHandle doesn't conflict in a multiplayer server
		// Since all TimerHandles would be the same, creating one in the header would cause later ones to overwrite earlier ones
		FTimerDelegate Delegate;
		Delegate.BindUFunction(this, "RespawnPlayerElapsed", Player->GetController(), Player);
		// The delegate allows passing variables to the timer's function

		float RespawnDelay = 2.0f;

		GetWorldTimerManager().SetTimer(TimerHandle_RespawnDelay, Delegate, RespawnDelay, false);
	}

	// Give credits for killing minion (AI)
	APawn* KillerPawn = Cast<APawn>(Killer);
	if (KillerPawn)
	{
		ASPlayerState* PS = KillerPawn->GetPlayerState<ASPlayerState>();
		if (PS) // Can cast and check for nullptr within the if statement
		{
			PS->AddCredits(CreditsPerKill);
		}
	}
}

void ASGameModeBase::RespawnPlayerElapsed(AController* Controller, ASCharacter* SCharacter)
{
	if (ensure(Controller))
	{
		Controller->UnPossess();
		// Release control of the character
		SCharacter->Destroy();
		// Destroy the corpse
		RestartPlayer(Controller);
	}
}