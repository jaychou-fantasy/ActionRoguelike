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
	//这个PlayerStateClass是gamemode自带的选项，是bp里面那种需要选择“哪个state class填入的”那种
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

	//其实这个spawnbottimeElapsed可以直接写在startplay里面，只是这样清晰一点
	GetWorldTimerManager().SetTimer(TimerHandle_SpawnBot, this, &ASGameModeBase::SpawnBotTimeElapsed, SpawnTimerInterval, true);//true是Bloop
	
	SpawnPowerupTimeElapsed();
}

void ASGameModeBase::SpawnBotTimeElapsed()
{
	//console里面设置了0，那么就直接return，不让spawn了
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

	//先检查是否有必要runEQSquery
	//因为runEQS 很expensive

	
	UEnvQueryInstanceBlueprintWrapper* QueryInstance  = UEnvQueryManager::RunEQSQuery(this, SpawnBotQuery, this, EEnvQueryRunMode::RandomBest5Pct, nullptr);//最后那个wrapper好像从来不会用到
	//你可以传入一个已有的 EQS Query Wrapper 来复用它；如果传 nullptr，系统会自动创建一个新的 Query Wrapper
	//最后得到query得到的结果会通过 Query Wrapper 的 OnQueryFinishedEvent 委托广播出去
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
	if (Locations.IsValidIndex(0))//.Num() > 0也可以，因为我们只调用这之中一个位置
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

		//nullptr就是返回的wrapper class，但是我们需要的是UEnvQueryInstanceBlueprintWrapper，所以直接nullptr不额外添加了
		UEnvQueryInstanceBlueprintWrapper* QueryInstance = UEnvQueryManager::RunEQSQuery(this, PowerupSpawnQuery, this, EEnvQueryRunMode::AllMatching, nullptr);
		if (ensure(QueryInstance))
		{
			UE_LOG(LogTemp, Log, TEXT("EQS Query Started"));

			//就是query完成是一个delegate，然后一旦完成就会传输信息，我们的新funct需要这些，就adddynamic
			QueryInstance->GetOnQueryFinishedEvent().AddDynamic(this, &ASGameModeBase::OnPowerupSpawnQueryCompleted);
		}
	}
}

void ASGameModeBase::OnPowerupSpawnQueryCompleted(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus)
{
	//依旧先检查querystatus(状况情况)，state(状态)
	if (QueryStatus != EEnvQueryStatus::Success)
	{
		UE_LOG(LogTemp, Error, TEXT("Spawn Power up EQS Query Failed! Status: %d"), (int)QueryStatus);
		return;
	}
	//最重要的一步，提取EQS的结果as array of locations
	TArray<FVector>Locations = QueryInstance->GetResultsAsLocations();
	UE_LOG(LogTemp, Warning, TEXT("Locations found: %d"), Locations.Num());

	//keeep used locations to easily check distance between points
	TArray<FVector>UsedLocations;
	
	//在bot spawn里面就是先在time elapse里面判断人数是否少于上限 后，才进行run EQS + on query completed
	int32 SpawnCounter = 0;

	while (SpawnCounter < DesiredPowerupCount && Locations.Num() >0)
	{
		UE_LOG(LogTemp, Log, TEXT("Entering Spawn Loop"));
		//pick a random location from remaining points
		int32 RandomLocationIndex = FMath::RandRange(0, Locations.Num()-1);//因为randrange是包含上界的，5个数的array只包含[0,4]
		FVector PickedLocation = Locations[RandomLocationIndex];

		//Remove to avoid picking again
		Locations.RemoveAt(RandomLocationIndex);

		//check minimum distance requirement,if accepted,then spawn power_up at that picked location
		bool bValidLocation = true;
		//这样就会导致，第一个picked的位置直接生成，因为usedLocation没东西，所以直接跳过forloop
		for (FVector OtherLocation : UsedLocations)
		{
			float DistanceTo = (PickedLocation - OtherLocation).Size();//Size()就是把两个vector3D的差，算成距离大小
			if (DistanceTo < RequiredPowerupDistance)
			{
				//Show skipped location due to distance
				//DrawDebugSphere(GetWorld(),PickedLocation,0.0f, 20, FColor::Red, false, 10.0f);

				//too close,skip to next attempt;
				bValidLocation = false;
				break;
			}
		}

		//Failed the distance test
		if (!bValidLocation)
		{
			continue;
			//直接random到下一个pickedLocation了
		}

		//pick a random power_up class
		int32 RandomClassIndex = FMath::RandRange(0,PowerupClasses.Num()-1);  
		TSubclassOf<AActor> RandomPowerupClass = PowerupClasses[RandomClassIndex];
		if (RandomClassIndex)
		{
			UE_LOG(LogTemp, Error, TEXT("RandomPowerupClass is NULL"));
		}

		if(AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(RandomPowerupClass, PickedLocation, FRotator::ZeroRotator))   //后面那个spawnparameter就用默认的了：但是有啥区别？
		{
			UE_LOG(LogTemp, Log, TEXT("Spawn SUCCESS: %s"), *SpawnedActor->GetName());
		}

		//keep for distance check
		UsedLocations.Add(PickedLocation);
		SpawnCounter++;
	}

}


//respawn character
void ASGameModeBase::OnActorKilled(AActor* VictimActor, AActor* Killer)
{
	UE_LOG(LogTemp, Log, TEXT("OnActorKilled: Victim: %s, Killer: %s"), *GetNameSafe(VictimActor), *GetNameSafe(Killer));
	//*被operator了，就是变成了get FSrting里所有的char了

	ASCharacter* Player = Cast<ASCharacter>(VictimActor);
	if (Player)
	{
		FTimerHandle TimerHandle_RespawnDelay;
		//必须得本地化，这样多人游戏的服务器中，每个人的timerhandle才不会冲突
		//因为每个人的timerhandle都是一样的，这样如果创建再hfile，后一个就会覆盖前一个
		FTimerDelegate Delegate;
		Delegate.BindUFunction(this, "RespawnPlayerElapsed", Player->GetController(),Player);
		//delegate的好处就是可以给timer的函数传递一个变量
		
		float RespawnDelay = 2.0f;

		GetWorldTimerManager().SetTimer(TimerHandle_RespawnDelay, Delegate, RespawnDelay, false);
	}

	//give credits for kill minion(AI)
	APawn* KillerPawn = Cast<APawn>(Killer);
	if (KillerPawn)
	{
		ASPlayerState* PS = KillerPawn->GetPlayerState<ASPlayerState>();
		if (PS)// <can cast and check for nullptr within id_statement
		{
			PS->AddCredits(CreditsPerKill);
		}
	}
}

void ASGameModeBase::RespawnPlayerElapsed(AController* Controller,ASCharacter* SCharacter)
{
	if (ensure(Controller))
	{
		Controller->UnPossess();
		//不让控制 character了
		SCharacter->Destroy();
		//销毁尸体
		RestartPlayer(Controller);
	}
}