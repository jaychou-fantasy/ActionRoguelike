// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerup_Action.h"

#include "SActionComponent.h"

void ASPowerup_Action::Interact_Implementation(APawn* InstigatorPawn)
{
	Super::Interact_Implementation(InstigatorPawn);
	//make sure Instigator&ActionToGrant are set
	if (!ensure(InstigatorPawn && ActionToGrant))
	{
		return;
	}
	
	USActionComponent* ActionComp = Cast<USActionComponent>(GetComponentByClass(USActionComponent::StaticClass()));
	if (ActionComp)
	{
		//check if Instigator has already granted that action
		if (ActionComp->GetAction(ActionToGrant))
		{
			FString DebugMsg = FString::Printf(TEXT("Action '%s' already known."), *GetNameSafe(ActionToGrant));
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, DebugMsg);
			return;
		}
		//if not,then grant that action
		ActionComp->AddAction(InstigatorPawn,ActionToGrant);
		HideAndCooldownPowerup();
	}
}
