// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerup_HealthPotion.h"
#include "SAttributeComponent.h"
#include "SPlayerState.h"

//so we can omit 'NS'LOCTEXT
#define LOCTEXT_NAMESPACE "InteractableActors"

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
	//get AttributeComp��static funtcion��
	USAttributeComponent* AttributeComp = USAttributeComponent::GetAttributes(InstigatorPawn);
			//Cast<USAttributeComponent>(InstigatorPawn->GetComponentByClass(USAttributeComponent::StaticClass()));,// Cast is used because the return type is UAttributeComponent, not USAttributeComponent(no s)
			//you can do the below directly
			//USAttributeComponent* AttributeComp = InstigatorPawn->FindComponentByClass<USAttributeComponent>()
	
			//check if not already at max health
	if (ensure(AttributeComp) && !AttributeComp->IsFullHealth())
	{
		//only activate if healed successfully
		if (ASPlayerState* PS = InstigatorPawn->GetPlayerState<ASPlayerState>())//cast is also OK
		{
			if (PS->RemoveCredits(CreidtCost) && AttributeComp->ApplyHealthChange(this, AttributeComp->GetHealthMax()))//directly add 100hp -> to max
			{
				HideAndCooldownPowerup();
			}
		}
	}
}
FText ASPowerup_HealthPotion::GetInteractText_Implementation(APawn* InstigatorPawn)
{
	USAttributeComponent* AttributeComp = USAttributeComponent::GetAttributes(InstigatorPawn);
	if (AttributeComp && AttributeComp->IsFullHealth())
	{
		//"namespace","column","text you want pass"
		//NSLOCTEXT("InteractableActors"
		return LOCTEXT("HealthPotion_FullHealthWarning","Already at full health");
	}
	return FText::Format(LOCTEXT("HealthPotion_InteractMessage","Cost {0} credits,restore health to maximum"),CreidtCost);
}

//don't forget to undef
#undef LOCTEXT_NAMESPACE