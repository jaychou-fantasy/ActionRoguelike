// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SAttributeComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnHealthChanged, AActor*, InstigatorActor, USAttributeComponent*, OwningComp, float, NewHealth, float, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnRageChanged,AActor*, InstigatorActor,USAttributeComponent*,OwningComp, float, NewRage,float ,Delta);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ACTIONROGUELIKE_API USAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USAttributeComponent();

	//static: belong to class not instance
	UFUNCTION(BlueprintCallable ,Category = "Attributes")
	static USAttributeComponent* GetAttributes(AActor* FromActor);

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	static bool IsActorAlive(AActor* Check_Actor);


protected:

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Replicated,Category = "Attributes")
	float Health;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Replicated,Category = "Attributes")
	float HealthMax;
	
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Replicated,Category = "Attributes")
	float Rage;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Replicated,Category = "Attributes")
	float RageMax;
	//HealthMax,Stamina,Strength.
	
	
	// Health is replicated, but the broadcast is only for triggering UI changes, etc., so it doesn't need to be reliable — that would waste a lot of network resources
	// Replication focuses on the end result. Values like 100 → 80 → 60 → 40 might only show 100 → 40, 
	// whereas Multicast emphasizes every change, highlighting continuous transitions
	// Additionally, Unreliable: when two characters are 100km apart, the game may not consider them relevant. 
	// Synchronization only happens when they get closer — this is what Unreliable does
	// Net Relevancy / Net Cull Distance: Actors that are far away are no longer replicated
	// When declared as Unreliable, relevancy is taken into account (this is uncertain)
	// If it's Reliable, relevancy is ignored (this is also uncertain)
	UFUNCTION(NetMulticast, Reliable)//@note:could mark as unreliable once we moved the 'state' out of scharacter (eg. once its cosmetic only)
	void MulticastHealthChanged(AActor* Instigator, float NewHealth, float Delta);

	UFUNCTION(NetMulticast,Unreliable)//@note:since we did't assign some dynamic UI change for rage,just set to unreliable
	void MulticastRageChanged(AActor* Instigator,float NewRage,float Delta);	

public:	
	// 'const' means you can only get value rather than tweak it
	UFUNCTION(BlueprintCallable,Category = "Attributes")
	bool IsAlive() const; 

	

	UFUNCTION(BlueprintCallable,Category = "Attributes")
	bool ApplyHealthChange(AActor* InstigatorActor,float Delta);


	UFUNCTION(BlueprintCallable,Category = "Attributes")
	bool IsFullHealth() const;

	UFUNCTION(BlueprintCallable,Category = "Attributes")
	float GetHealthMax() const;
	
	UFUNCTION(BlueprintCallable,Category = "Attributes")
	float GetRage() const;
	
	UFUNCTION(BlueprintCallable,Category = "Attributes")
	bool ApplyRage(AActor* Instigator, float Delta);

	UFUNCTION(BlueprintCallable,Category = "Attributes")
	float GetHealth() const;
	
	
	UFUNCTION(BlueprintCallable,Category = "Attributes")
	bool Kill(AActor* Instigator);


	UPROPERTY(BlueprintAssignable,Category = "Attributes")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable,Category = "Attributes")
	FOnRageChanged OnRageChanged;

};
