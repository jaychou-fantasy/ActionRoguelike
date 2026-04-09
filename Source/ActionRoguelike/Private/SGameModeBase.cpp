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
#include "SGamePlayInterface.h"
#include "SPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"


static TAutoConsoleVariable<bool> CVarSpawnBots(TEXT("su.SpawnBots"),true,TEXT("Enable spawning of bots via timer."),ECVF_Cheat);


ASGameModeBase::ASGameModeBase()
{
	// This PlayerStateClass is a built-in option in GameMode , the kind you select in Blueprint by choosing which State Class to assign
	PlayerStateClass = ASPlayerState::StaticClass();
	
	SlotName = "SaveGame01";

	SpawnTimerInterval = 2.0f;
	CreditsPerKill = 20;

	DesiredPowerupCount = 10;
	RequiredPowerupDistance = 2000.0f;
}

void ASGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	
	LoadSaveGame();
}


void ASGameModeBase::WriteSaveGame()
{
	//Iterate all player states,we don't have proper ID to match yet(Require Steam or EOS)
	for (int32 i =0;i<GameState->PlayerArray.Num();i++)
		//gamestate is an array that save all player's states
	{
		ASPlayerState* PS = Cast<ASPlayerState>(GameState->PlayerArray[i]);
		if (PS)
		{
			PS->SavePlayerState(CurrentSaveGame);
			break; //single player only at this point
		}
	}
	//we first save credit(update latedly in state) to currentSaveGame->Credit
	//then save CurrentSaveGame
	
	//since we have decided to overwrite a new savegame,then actually we can just empty it and then write new data to avoid data overlap
	CurrentSaveGame->SavedActors.Empty();
	//iterate all actors to save
	for (FActorIterator It(GetWorld());It;++It)
	{
		AActor* Actor = *It;
		//only interested in our "Gameplay Actors"
		if (!Actor->Implements<USGamePlayInterface>())
		{
			continue;
		}
		
		FActorSaveData ActorData;
		ActorData.ActorName = Actor->GetName();
		ActorData.Transform = Actor->GetTransform();
		
		//memorywriter is a so-called proxy,archive write data in it,and it just write the data in ByteData
		FMemoryWriter MemWriter(ActorData.ByteData);
		//FArchive is a rule of how to save the data
		//FArchive--FProxyArchive--FObjectAnd....
		//this mean we save the data not in pointers,but the string address like /Game/Weapon/SM_Sword
		FObjectAndNameAsStringProxyArchive Ar(MemWriter,true);
		
		//find only varibles with UPROPERTY(SaveGame)
		Ar.ArIsSaveGame = true;
		//convert actor'variable into binary array;
		Actor->Serialize(Ar);
		
		CurrentSaveGame->SavedActors.Add(ActorData);
		UE_LOG(LogTemp,Warning,TEXT("ActorData: %s successfully added"),*Actor->GetName());//dereference,FString(actually a address)->*->*TChar--->so %s can use
	}
	
	// save game to the ponited slot
	UGameplayStatics::SaveGameToSlot(CurrentSaveGame,SlotName,0);
	//save game to slot function would only save "CurrentSaveGame"(the things that you declare in SSaveGame class)
}

void ASGameModeBase::LoadSaveGame()
{
	UE_LOG(LogTemp,Warning,TEXT("Load SaveGame Called."));
	if(UGameplayStatics::DoesSaveGameExist(SlotName,0))
	{
		//if gamesave exists,then load it from slot;
		CurrentSaveGame = Cast<USSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName,0));
	    if(CurrentSaveGame ==nullptr)
		{
			UE_LOG(LogTemp,Warning,TEXT("Failed to load SaveGame Data."));
			return;
		}
		//this place changed
	}
	else
		//no need to iterate when first create a savegame object
	{
		//if not,then create a gamesave
		CurrentSaveGame = Cast<USSaveGame>(UGameplayStatics::CreateSaveGameObject(USSaveGame::StaticClass()));
	
		UE_LOG(LogTemp,Log,TEXT("Created New SaveGame Data."));
	}
}

void ASGameModeBase::LoadActorData()
{
	for (FActorIterator It(GetWorld());It;++It)
	{
		AActor* Actor = *It;
		if (!Actor->Implements<USGamePlayInterface>())
		{
			continue;
		}
		for (FActorSaveData ActorData : CurrentSaveGame->SavedActors)
		{
			if (ActorData.ActorName == Actor->GetName())
			{
				UE_LOG(LogTemp,Warning,TEXT("MATCHED ACTOR: %s"), *Actor->GetName());
				if (Actor->SetActorTransform(ActorData.Transform))
				{
					UE_LOG(LogTemp,Warning,TEXT("Loaded ActorTransform Data."));
				}
					
				FMemoryReader MemReader(ActorData.ByteData);
				FObjectAndNameAsStringProxyArchive Ar(MemReader,true);
				//convert back(same function,inverse usage)
				Actor->Serialize(Ar);
				UE_LOG(LogTemp,Warning,TEXT("Serialize succeed."));
				//serialize and deserialize is just to save UPROPERTY(SaveGame),more convenient than just save the variable you declared in SaveGame class
					
				ISGamePlayInterface::Execute_OnActorLoaded(Actor);
					
				break;//one actor data only corresponding to one actor
				//so if found,just break out for next actor to save workflow
			}
		}
	}
}

//why we put our state_load here is because:
//gamemode create(and actors)->init->loadsavegame->gamestate create->player controller->player state->handlestatringnewplayer->then can we load player_state
//handleStartingNewPlayer------>to load playerstate
void ASGameModeBase::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	
	ASPlayerState* PS = NewPlayer->GetPlayerState<ASPlayerState>();
	if (PS)
	{
		PS->LoadPlayerState(CurrentSaveGame);
	}
}

void ASGameModeBase::StartPlay()
{
	Super::StartPlay();
	
	//LoadSaveGame();
	LoadActorData();

	// Actually, this SpawnBotTimeElapsed could be written directly in StartPlay, but doing it this way is a bit clearer
	GetWorldTimerManager().SetTimer(TimerHandle_SpawnBot, this, &ASGameModeBase::SpawnBotTimeElapsed, SpawnTimerInterval, true);//true��Bloop
	
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

			// When the query finishes, it's a delegate �� once completed, it transmits information. Our new function needs this data, so we use AddDynamic
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

		if (AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(RandomPowerupClass, PickedLocation, FRotator::ZeroRotator))   // The SpawnParameter uses default values �� what's the difference?
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