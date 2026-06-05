// Fill out your copyright notice in the Description page of Project Settings.


#include "SActionEffect_Thorns.h"
#include "SAttributeComponent.h"
#include "SAction.h"
#include "SGameplayFunctionLibrary.h"

USActionEffect_Thorns::USActionEffect_Thorns()
{
	ReflectFraction = 0.2f;
	
	Duration = 0.0f;
	Period = 0.0f;
}


void USActionEffect_Thorns::StartAction_Implementation(AActor* Instigator)
{
	Super::StartAction_Implementation(Instigator);
	USAttributeComponent* AttributeComp = USAttributeComponent ::GetAttributes(Instigator);
	
	if (AttributeComp)
	{
		//then,the data of OnHealthChanged would be transported to the LOCAL function with the same name(OnHealthChanged)
		AttributeComp->OnHealthChanged.AddDynamic(this,&USActionEffect_Thorns::OnHealthChanged);
	}
}
void USActionEffect_Thorns::StopAction_Implementation(AActor* Instigator)
{
	Super::StopAction_Implementation(Instigator);
	USAttributeComponent* AttributeComp = USAttributeComponent ::GetAttributes(Instigator);
	
	if (AttributeComp)
	{
		AttributeComp->OnHealthChanged.RemoveDynamic(this,&USActionEffect_Thorns::OnHealthChanged);
	}
}

void USActionEffect_Thorns::OnHealthChanged(AActor* Instigator, USAttributeComponent* OwningComp, float NewHealth,
	float Delta)
{
	AActor* OwningActor = OwningComp->GetOwner();
	
	//trigger only when (Damaged,Not Hit Self)
	if (Delta < 0.0f && OwningActor != Instigator)
	{
		//Round to nearest integer to avoid "ugly" damage number and tiny reflection
		int32 ReflectAmount = FMath::RoundToInt(Delta * ReflectFraction);
		if (ReflectAmount == 0)
		{
			return;
		}
		//Flip to Positive, so we don't end up healing the damage_causer when passed into damage
		//'casue we use ApplyDamage function to apply damage(its parameter of "DamageAmount" are suppose to be postitive
		ReflectAmount = FMath::Abs(ReflectAmount);
		
		//return Damage back to Damage Sender
		//Q?:why use this function rather than simply make health changed
		//A:we need to pass "Instigator" parameter. -----in game ,the instigator info is pretty important
		USGameplayFunctionLibrary::ApplyDamage(OwningActor,Instigator,ReflectAmount);
	}
}
