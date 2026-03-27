// Fill out your copyright notice in the Description page of Project Settings.


#include "SActionComponent.h"
#include "SAction.h"
#include "../ActionRoguelike.h"


USActionComponent::USActionComponent()
{

	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
}


void USActionComponent::BeginPlay()
{
	Super::BeginPlay();

	//server only
	//只让server有action，其他的都是server的action再replicated
	if (GetOwner()->HasAuthority())
	{
		for (TSubclassOf<USAction> ActionClass : DefaultActions)
		{
			AddAction(GetOwner(), ActionClass);
		}
	}

}
void USActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//FString DebugMsg = GetNameSafe(GetOwner()) + " : " + ActiveGameplayTags.ToStringSimple();
	//GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White, DebugMsg);
	// Draw All Actions
	for (USAction* Action : Actions)
	{
		FColor TextColor = Action->IsRunning() ? FColor::Blue : FColor::White;

		FString ActionMsg = FString::Printf(TEXT("[%s] Action: %s : IsRunning: %s : Outer: %s"),
			*GetNameSafe(GetOwner()),
			*Action->ActionName.ToString(),
			Action->IsRunning() ? TEXT("true") : TEXT("false"),
			*GetNameSafe(Action->GetOuter()));

		LogOnScreen(this, ActionMsg, TextColor, 0.0f);
	}

}


void USActionComponent::AddAction(AActor* Instigator,TSubclassOf<USAction> ActionClass)
{
	if (ensure(ActionClass))
	{
		//Outer可以看作是一个容器，就比如现在这里new了一个action，而outer是actioncomponent，那么当comp这个outer没了，里面的所有action也都会销毁
		USAction* NewAction = NewObject<USAction>(this, ActionClass);
		//类似于createDefaultSuboject
		if (ensure(NewAction))
		{
			Actions.Add(NewAction);
			UE_LOG(LogTemp, Log, TEXT("Add action named:%s"),*GetNameSafe(NewAction));

			//接下来我们设计一种，可以不在defaultActions里面添加action，而是通过bautostart来自动添加。
			// （其实就是等于在ue里面手动添加），只不过可以在特定的action子类里面，通过改变bautostart的值，来决定是否要在beginplay的时候自动添加这个action
			if (NewAction->bAutoStart && NewAction->CanStart(Instigator))
			{
				NewAction->StartAction(Instigator);
			}
		}
	}
}

void USActionComponent::RemoveAction(USAction* ActionToRemove)
{
	if (!ensure(ActionToRemove && !ActionToRemove->IsRunning()))
	{
		return;
	}

	Actions.Remove(ActionToRemove);
}



bool USActionComponent::StartActionByName(AActor* Instigator, FName ActionName)
{
	for (USAction* Action : Actions)
	{
		if (Action && Action->ActionName == ActionName)
		{
			UE_LOG(LogTemp, Warning, TEXT("Checking Action: %s"), *Action->ActionName.ToString());
			if (!Action->CanStart(Instigator))
			{
				FString FailingMsg = FString::Printf(TEXT("Failed to run: %s"), *ActionName.ToString());
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FailingMsg);
				//-1 means create new msg every time

				continue;
			}
			//如果是client运行的，那么就run RPC,然后通知server也start这个action。然后本地也会action->startaction
			if (!GetOwner()->HasAuthority())
			{
				ServerStartActionByName(Instigator, ActionName);
			}
			Action->StartAction(Instigator);

			return true;
		}
	}
	return false;
}

bool USActionComponent::StopActionByName(AActor* Instigator, FName ActionName)
{
	for (USAction* Action : Actions)
	{
		if (Action && Action->ActionName == ActionName)
		{
			if (Action->IsRunning())
			{
				Action->StopAction(Instigator);
				return true;
			}
		}
	}
	return false;
}


void USActionComponent::ServerStartActionByName_Implementation(AActor* Instigator, FName ActionName)
{
	StartActionByName(Instigator, ActionName);
}
