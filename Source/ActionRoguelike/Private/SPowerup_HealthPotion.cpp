// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerup_HealthPotion.h"
#include "SAttributeComponent.h"
#include "SPlayerState.h"

// Sets default values
ASPowerup_HealthPotion::ASPowerup_HealthPotion()
{
	CreidtCost = 50;
}

void ASPowerup_HealthPotion::Interact_Implementation(APawn* InstigatorPawn)
{
	if (!ensure(InstigatorPawn))
	{
		return;
	}
	//get AttributeComp（static funtcion）
	USAttributeComponent* AttributeComp = USAttributeComponent::GetAttributes(InstigatorPawn);
			//Cast<USAttributeComponent>(InstigatorPawn->GetComponentByClass(USAttributeComponent::StaticClass()));,cast是因为返回的是uattributecomponent，不带s
			//可以直接：
			//USAttributeComponent* AttributeComp = InstigatorPawn->FindComponentByClass<USAttributeComponent>()
	
			//check if not already at max health
	if (ensure(AttributeComp) && !AttributeComp->IsFullHealth())
	{
		//only activate if healed successfully
		if (ASPlayerState* PS = InstigatorPawn->GetPlayerState<ASPlayerState>())//cast也可以
		{
			if (PS->RemoveCredits(CreidtCost) && AttributeComp->ApplyHealthChange(this, AttributeComp->GetHealthMax()))//直接就加100滴血--直接加满了
			{
				HideAndCooldownPowerup();
			}
		}
	}
}


