// Fill out your copyright notice in the Description page of Project Settings.


#include "SAction.h"
#include "SActionComponent.h"
#include "../ActionRoguelike.h"//上一个文件夹的headerfile
#include "Net/UnrealNetwork.h"

//在action被Newobject<USAction>( , )的时候调用，进行初始化
void USAction::Initialize(USActionComponent* NewActionComp)
{
	ActionComp = NewActionComp;
}

bool USAction::CanStart_Implementation(AActor* Instigator)
{
	USActionComponent* Comp = GetOwningComponent();
	if (IsRunning()) // If already running, cannot start — prevents the same action from being started multiple times simultaneously before the previous one finishes
		// Simply add its own tag to BlockedTags; since it's running before the tag is added, it won't affect the current action instance
	{
		UE_LOG(LogTemp, Warning, TEXT("Action already running"));
		return false;
	}
	if (Comp->ActiveGameplayTags.HasAny(BlockedTags))
	{
		return false;
	}

	return true;
}


// This implements the basic start and end notifications in UASAction. 
// The actual start and end logic is called from the component.

void USAction::StartAction_Implementation(AActor* Instigator)
{
	UE_LOG(LogTemp, Log, TEXT("Running: %s"), *GetNameSafe(this));
	//LogOnScreen(this, FString::Printf(TEXT("Started: %s"), *ActionName.ToString()), FColor::Green);

	USActionComponent* Comp = GetOwningComponent();
	// No need to check with Ensure, because the Action is created via NewObject in ActionComponent's AddAction, so it definitely exists
	Comp->ActiveGameplayTags.AppendTags(GrantsTags);

	RepData.bIsRunning = true;
	RepData.Instigator = Instigator;
}

void USAction::StopAction_Implementation(AActor* Instigator)
{
	UE_LOG(LogTemp, Log, TEXT("Stopped: %s"), *GetNameSafe(this));
	//LogOnScreen(this, FString::Printf(TEXT("Stopped: %s"), *ActionName.ToString()), FColor::White);

	// ensureAlways(bIsRunning); // Since StopAction is only called when ActionComponent has confirmed IsRunning == false, use ensureAlways directly — no if statement needed (the if is handled in the component)

	USActionComponent* Comp = GetOwningComponent();
	Comp->ActiveGameplayTags.RemoveTags(GrantsTags);

	RepData.bIsRunning = false;
	RepData.Instigator = Instigator;
}



// Ensures GetWorld can properly return a UWorld pointer. If the Character doesn't exist, its GetWorld becomes invalid, so we add an ensure here as a safeguard
// Only AActor, UActorComponent, and UWorldSubsystem have native awareness of the world
UWorld* USAction::GetWorld() const
{
	// Outer is set when creating the action via NewObject<T> (the "this" at that time becomes the Outer, which is of type ActionComponent)
	AActor* Actor = Cast<AActor>(GetOuter());
	if (Actor)
	{
		return Actor->GetWorld();
	}
	return nullptr;
}

USActionComponent* USAction::GetOwningComponent() const
{
	//AActor* Actor = Cast<AActor>(GetOuter());
	//return Actor->GetComponentByClass(USActionComponent::StaticClass());
	// Using GetComponentByClass would iterate through all the Actor's components to find ActionComponent, which is too costly.
	// Instead, we directly store the corresponding ActionComponent in a UProperty within the Action class
	return ActionComp;
}

// On the client: if the server starts an action, IsRunning becomes true, and OnRep triggers StartAction on the client.
// This way, the server player's action is also displayed on the client.
// Therefore, we need to modify the ensureAlways check in StopAction above:
// Because when the client calls StopAction for the server player, bIsRunning has already been replicated as false.
void USAction::OnRep_RepData()
{
	if (RepData.bIsRunning)
	{
		StartAction(RepData.Instigator);
	}
	else
	{
		StopAction(RepData.Instigator);
	}
}


bool USAction::IsRunning() const
{
	return RepData.bIsRunning;
}


void USAction::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USAction,RepData);
	DOREPLIFETIME(USAction, ActionComp);
}