// Fill out your copyright notice in the Description page of Project Settings.


#include "SAttributeComponent.h"
#include "SGameModeBase.h"
#include "Net/UnrealNetwork.h"


static TAutoConsoleVariable<float> CVarDamageMultiplier(TEXT("su.DamageMultiplier"), 1.0f, TEXT("Global Damage Modifier for Attribute Component."), ECVF_Cheat);

// Sets default values for this component's properties
USAttributeComponent::USAttributeComponent()
{
	HealthMax = 100.0f;

	Health = HealthMax;

	SetIsReplicatedByDefault(true);
}

USAttributeComponent* USAttributeComponent::GetAttributes(AActor* FromActor)
{
	if (FromActor)
	{
		return Cast<USAttributeComponent>(FromActor->GetComponentByClass(USAttributeComponent::StaticClass()));
	}
	//defalut retun nullptr
	return nullptr;
}

bool USAttributeComponent::IsActorAlive(AActor* Check_Actor)
{
	USAttributeComponent* AttributeComp = GetAttributes(Check_Actor);
	if (AttributeComp)
	{
		return AttributeComp->IsAlive();
	}
	//defalut retun false
	return false;
}




bool USAttributeComponent::IsFullHealth() const
{
	return Health == HealthMax;
}

float USAttributeComponent::GetHealthMax() const
{
	return HealthMax;
}

float USAttributeComponent::GetHealth() const
{
	return Health;
}

bool USAttributeComponent::IsAlive() const
{
	return Health > 0.0f;
}

bool USAttributeComponent::Kill(AActor* Instigator)
{
	return ApplyHealthChange(Instigator, -GetHealthMax());
}

bool USAttributeComponent::ApplyHealthChange(AActor* InstigatorActor, float Delta)
{
	// If god mode is enabled, the actor won't take damage
	if (!GetOwner()->CanBeDamaged() && Delta < 0.0f)
	{
		return false;
	}

	if (Delta < 0.0f)
	{
		float DamageMultiplier = CVarDamageMultiplier.GetValueOnGameThread();
		Delta *= DamageMultiplier;
	}

	// Health += Delta;
	float OldHealth = Health;

	Health = FMath::Clamp(Health + Delta, 0.0f, HealthMax); // Clamp health to valid range after adding Delta

	float ActualDelta = Health - OldHealth;

	UE_LOG(LogTemp, Log, TEXT("ApplyHealthChange: Owner=%s NewHealth=%f Delta=%f"), *GetNameSafe(GetOwner()), Health, ActualDelta);

	//***
	// OnHealthChanged.Broadcast(InstigatorActor, this, Health, ActualDelta);
	// The multicast version below is used so that when registered on the server, it gets called on all clients as well
	if (ActualDelta != 0)
	{
		MulticastHealthChanged(InstigatorActor, Health, ActualDelta);
	}
	//***


	// Died
	if (ActualDelta < 0.0f && Health <= 0.0f)
	{
		ASGameModeBase* GM = GetWorld()->GetAuthGameMode<ASGameModeBase>();
		if (GM)
		{
			GM->OnActorKilled(GetOwner(), InstigatorActor);
		}
	}
	return ActualDelta != 0;
	// Returns true if there was an actual change, false otherwise
}

void USAttributeComponent::MulticastHealthChanged_Implementation(AActor* Instigator, float NewHealth, float Delta)
{
	OnHealthChanged.Broadcast(Instigator, this, Health,Delta);
}


void USAttributeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USAttributeComponent, Health);
	DOREPLIFETIME(USAttributeComponent, HealthMax);

	// DOREPLIFETIME_CONDITION(USAttributeComponent, HealthMax, COND_InitialOnly); 
	// This replicates HealthMax only once at the initial moment ¡ª any subsequent changes to HealthMax will not be replicated
}