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
	//let the server be the only owner of action£¬others are server'action,then replicated to client
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

void USActionComponent::AddAction(AActor* Instigator, TSubclassOf<USAction> ActionClass)
{
	if (ensure(ActionClass))
	{
		// The Outer can be thought of as a container. When we create a new action here with ActionComponent as the Outer,
		// if the ActionComponent (the Outer) is destroyed, all actions within it will also be destroyed.
		// Here, the action's Outer is set to ActionComponent.
		// This means action->GetOuter() returns ActionComponent, and action->GetOuter()->GetOuter() returns the SCharacter.
		USAction* NewAction = NewObject<USAction>(GetOuter(), ActionClass);
		NewAction->Initialize(this);
		// Similar to CreateDefaultSubobject

		if (ensure(NewAction))
		{
			Actions.Add(NewAction);
			UE_LOG(LogTemp, Log, TEXT("Add action named:%s"), *GetNameSafe(NewAction));

			// Next, we design a way to automatically add actions without needing to specify them in DefaultActions,
			// but instead via bAutoStart.
			// (This is essentially equivalent to manually adding them in UE, but by changing the bAutoStart value in specific action subclasses,
			// you can decide whether to auto-start the action during BeginPlay.)
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
				// -1 means create a new message every time

				continue;
			}
			// If running on the client, execute the RPC to notify the server to also start this action.
			// The local action will also call StartAction.
			// This ensures that actions performed on the client are visible to the server and other players.
			if (!GetOwner()->HasAuthority())
			{
				ServerStartActionByName(Instigator, ActionName);
			}
			Action->StartAction(Instigator);
			// @fixme: If actions performed on the server cannot replicate to the client,
			// we need to replicate the actions container and the bIsRunning state.
			// First replicate the actions, then replicate StartAction.

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


// Here, Channel refers to the replication channel between a UObject subclass on the server and its counterpart on the client.
// Bunch -> network data packet
// RepFlags -> replication configuration flags
bool USActionComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	// Call super first, which returns false initially. Only becomes true if any changes occur later, and the || operator will return true.
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	for (USAction* Action : Actions)
	{
		// Serializes the state of a UObject into the network data packet and sends it to the corresponding client (or receiver)
		WroteSomething |= Channel->ReplicateSubobject(Action, *Bunch, *RepFlags);
		// Logical OR: if either side is true, the result is true.
		// If Channel successfully replicates a change in any Action (from server to client), it returns true.
		// This means that as long as at least one Action successfully syncs, the function returns true (and that Action is replicated).
	}

	return WroteSomething;
}
// It's fine that the return value isn't used elsewhere because Channel->ReplicateSubobject(Action, *Bunch, *RepFlags) already handles replicating the Action
// (synchronizing server changes to the client).
// Client-to-server replication is handled via ServerStartActionByName.


void USActionComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(USActionComponent, Actions);
}