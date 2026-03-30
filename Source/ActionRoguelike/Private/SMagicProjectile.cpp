// Fill out your copyright notice in the Description page of Project Settings.


#include "SMagicProjectile.h"
#include "Components/SphereComponent.h"
#include "SAttributeComponent.h"
#include "SGameplayFunctionLibrary.h"
#include "SActionComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "SActionEffect.h"



// Sets default values
ASMagicProjectile::ASMagicProjectile()
{
	SphereComp->SetSphereRadius(20.0f);
	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &ASMagicProjectile::OnActorOverlap);
	
	DamageAmount = 20.0f;
}

void ASMagicProjectile::OnActorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Here, OtherActor and OtherComp are the hit actor and the specific component that was hit
	// The Instigator set in SpawnParams comes into play here ¡ª GetInstigator() retrieves it
	// If OtherActor (the actor overlapping with the projectile) is the same as Instigator, then none of the logic below executes, so no damage and no explosion
	if (OtherActor && OtherActor != GetInstigator())
	{
		//FName Muzzle = "Muzzle_01";
		//static FGameplayTag Tag = FGameplayTag::RequestGameplayTag("Status.Parrying");
		// This is the difference between requesting a tag and simply requesting a variable ¡ª later requests use the tag, 
		// and static ensures this request only happens once


		// Parry -> reverse velocity
		USActionComponent* ActionComp = Cast<USActionComponent>(OtherActor->GetComponentByClass(USActionComponent::StaticClass()));
		if (ActionComp && ActionComp->ActiveGameplayTags.HasTag(ParryTag))
		{
			MoveComp->Velocity = -MoveComp->Velocity;

			SetInstigator(Cast<APawn>(OtherActor));

			return;
			// If parried, don't trigger the explosion
		}

		//USAttributeComponent* AttributeComp = USAttributeComponent::GetAttributes(OtherActor);
		if (USGameplayFunctionLibrary::ApplyDirectionalDamage(GetInstigator(), OtherActor, DamageAmount, SweepResult))
		{
			Explode();
			// Add the burning action to OtherActor
			if (ActionComp && ensure(BurningActionClass))
			{
				ActionComp->AddAction(GetInstigator(), BurningActionClass);
			}
		}

		//Cast<USAttributeComponent>(OtherActor->GetComponentByClass(USAttributeComponent::StaticClass())); // "Give me the ID card for this class" ¡ª returns a unique identifier for the class
		// UE uses this "ID card" at runtime to identify and manipulate types
		//if (OtherComp)
		//{
			//if (AttributeComp)
			//{
				//AttributeComp->ApplyHealthChange(-20.0f);
				//Destroy();

				// Minus sign in front of DamageAmount to apply the change as damage, not healing
						  /*AttributeComp->ApplyHealthChange(GetInstigator(), -DamageAmount);*/
				// Here, AttributeComp is OtherActor's attribute component

				// ApplyHealthChange then triggers the OnHealthChanged delegate, which broadcasts to all functions bound via AddDynamic, triggering health reduction, etc.
						  /*UE_LOG(LogTemp, Warning, TEXT("SMagicProjectile: Applying %f damage to %s"), DamageAmount, *GetNameSafe(OtherActor));*/
				// Only explode when we hit something valid
						  /*Explode();*/
			//}
		//}
	}
}
