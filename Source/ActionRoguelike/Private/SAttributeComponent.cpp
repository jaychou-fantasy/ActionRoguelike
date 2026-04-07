// Fill out your copyright notice in the Description page of Project Settings.


#include "SAttributeComponent.h"
#include "SGameModeBase.h"
#include "Net/UnrealNetwork.h"
//for fully defined UWORLD class
#include "Engine/World.h"


static TAutoConsoleVariable<float> CVarDamageMultiplier(TEXT("su.DamageMultiplier"), 1.0f, TEXT("Global Damage Modifier for Attribute Component."), ECVF_Cheat);

// Sets default values for this component's properties
USAttributeComponent::USAttributeComponent()
{
	HealthMax = 100.0f;

	Health = HealthMax;
	
	Rage = 0.0f;
	
	RageMax = 100.0f;

	SetIsReplicatedByDefault(true);
}

USAttributeComponent* USAttributeComponent::GetAttributes(AActor* FromActor)
{
	if (FromActor)
	{
		return Cast<USAttributeComponent>(FromActor->GetComponentByClass(USAttributeComponent::StaticClass()));
	}
	//default return nullptr
	return nullptr;
}

bool USAttributeComponent::IsActorAlive(AActor* Check_Actor)
{
	USAttributeComponent* AttributeComp = GetAttributes(Check_Actor);
	if (AttributeComp)
	{
		return AttributeComp->IsAlive();
	}
	//defaultt return false
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

float USAttributeComponent::GetRage() const
{
	return Rage;
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
    
    //if(!GetOwner()->HasAuthority())
    //{
    //    return false;
    //}
    //you cant write like that,'casue if that,the projectile will never explode(casue the second proj was created by client),then it seems like pass through character's body like gods_mode
    //so just go to the essence->health change

	if (Delta < 0.0f)
	{
		float DamageMultiplier = CVarDamageMultiplier.GetValueOnGameThread();
		Delta *= DamageMultiplier;
	}

	// Health += Delta;
	float OldHealth = Health;
    float NewHealth = FMath::Clamp(Health + Delta, 0.0f, HealthMax); // Clamp health to valid range after adding Delta
    //we write it  above the HasAuthoirty() is bacuase:we want to convey the result of(do hit,and return true->so expode vfx will execute),Anyway,the actual delta would only be listened by delegate when HasAuthoirty();-->MulticastHealthChange();
    float ActualDelta = NewHealth - OldHealth;
    
    if(GetOwner()->HasAuthority())
    {
        Health = NewHealth;
        //***
        // OnHealthChanged.Broadcast(InstigatorActor, this, Health, ActualDelta);
        // The multicast version below is used so that when registered on the server, it gets called on all clients as well
        if (ActualDelta != 0)
        {
            MulticastHealthChanged(InstigatorActor, Health, ActualDelta);

			UE_LOG(LogTemp, Log, TEXT("ApplyHealthChange: Owner=%s NewHealth=%f Delta=%f"), *GetNameSafe(GetOwner()), Health, ActualDelta);
        }
        //***
		// 
        // Died
        if (ActualDelta < 0.0f && Health <= 0.0f)
        {
            ASGameModeBase* GM = GetWorld()->GetAuthGameMode<ASGameModeBase>();
            if (GM)
            {
                GM->OnActorKilled(GetOwner(), InstigatorActor);
            }
        }
    }

	return ActualDelta != 0;
	// Returns true if there was an actual change, false otherwise
}

bool USAttributeComponent::ApplyRage(AActor* Instigator, float Delta)
{
	float OldRage = Rage;
	float NewRage = FMath::Clamp(Rage + Delta,0.0f,RageMax);
	
	float ActualDelta = NewRage - OldRage;
	if (GetOwner()->HasAuthority())
	{
		Rage = NewRage;
		if (ActualDelta != 0.0f)
		{
			MulticastRageChanged(Instigator,Rage,ActualDelta);
			UE_LOG(LogTemp, Log, TEXT("ApplyRageChange: Owner=%s NewRage=%f Delta=%f"), *GetNameSafe(GetOwner()), Rage, ActualDelta);
		}
	}
	
	return ActualDelta != 0;
}

void USAttributeComponent::MulticastHealthChanged_Implementation(AActor* Instigator, float NewHealth, float Delta)
{
	OnHealthChanged.Broadcast(Instigator, this, Health,Delta);
}

void USAttributeComponent::MulticastRageChanged_Implementation(AActor* Instigator, float NewRage, float Delta)
{
	OnRageChanged.Broadcast(Instigator,this,NewRage,Delta);
}


void USAttributeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USAttributeComponent, Health);
	DOREPLIFETIME(USAttributeComponent, HealthMax);
	DOREPLIFETIME(USAttributeComponent,Rage);

	// DOREPLIFETIME_CONDITION(USAttributeComponent, HealthMax, COND_InitialOnly); 
	// This replicates HealthMax only once at the initial moment �� any subsequent changes to HealthMax will not be replicated
}
