// Fill out your copyright notice in the Description page of Project Settings.


#include "SActionComponent.h"
#include "SAction.h"
#include "../ActionRoguelike.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"


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
		//这里就是把action的outer设置成了actioncomponent
		//这样action->getouter()返回的就是actioncomp->getouter,就是scharacter了
		USAction* NewAction = NewObject<USAction>(GetOuter(), ActionClass);
		NewAction->Initialize(this);
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
			//这样client里做的动作，在server也可以看到player做的相应的动作
			if (!GetOwner()->HasAuthority())
			{
				ServerStartActionByName(Instigator, ActionName);
			}
			Action->StartAction(Instigator);
			//@fixme：如果server做了动作，无法同步到client里，那么先replicate actions（action容器），还有bisrunning的状态
			//先把action同步了，然后再把startaction同步了

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

//这里的channel是server的某个UObject子类和client的 进行复制同步这种操作的channel
//bunch->网络数据包
//repflags->复制的配置
bool USActionComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	//之前super的。那么就是返回false，只有后面变化了（变成true），||才会返回true
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	for (USAction* Action : Actions) 
	{
		                           //把一个 UObject 的状态序列化到网络数据包里，并发送给对应客户端（或者接收端）
		WroteSomething |= Channel->ReplicateSubobject(Action, *Bunch, *RepFlags);
		//逻辑或 || ，两边有一个为true就是返回true，如果channel成功同步action的变化（server到client），那么就返回1（true）
		//意思是这么多个action只要有一个同步成功了，就return true（然后再同步那个是true的action）
	}

	return WroteSomething;
}
//没有使用return value没关系，因为他的Channel->ReplicateSubobject(Action, *Bunch, *RepFlags);就是已经同步了action（server的变化replicate到client）
//client到server是用serverstartactionbyname来进行的


void USActionComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(USActionComponent, Actions);
}