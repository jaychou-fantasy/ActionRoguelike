// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SAttributeComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnHealthChanged, AActor*, InstigatorActor, USAttributeComponent*, OwningComp, float, NewHealth, float, Delta);

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
	
	// EditAnywhere - edit in BP editor and per-instance in level.
	// VisibleAnywhere - 'read-only' in editor and level. (Use for Components)
	// EditDefaultsOnly - hide variable per-instance, edit in BP editor only               ---cant be editted in detailed panel for instance placed in level
	// VisibleDefaultsOnly - 'read-only' access for variable, only in BP editor (uncommon)
	// EditInstanceOnly - allow only editing of instance (eg. when placed in level)
	// --
	// BlueprintReadOnly - read-only in the Blueprint scripting (does not affect 'details'-panel)
	// BlueprintReadWrite - read-write access in Blueprints
	// --
	// Category = "" - display only for detail panels and blueprint context menu.

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Replicated,Category = "Attributes")
	float Health;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Replicated,Category = "Attributes")
	float HealthMax;
	
	//HealthMax,Stamina,Strength.



	// Health is replicated, but the broadcast is only for triggering UI changes, etc., so it doesn't need to be reliable — that would waste a lot of network resources
	// Replication focuses on the end result. Values like 100 → 80 → 60 → 40 might only show 100 → 40, 
	// whereas Multicast emphasizes every change, highlighting continuous transitions
	// Additionally, Unreliable: when two characters are 100km apart, the game may not consider them relevant. 
	// Synchronization only happens when they get closer — this is what Unreliable does
	// Net Relevancy / Net Cull Distance: Actors that are far away are no longer replicated
	// When declared as Unreliable, relevancy is taken into account (this is uncertain)
	// If it's Reliable, relevancy is ignored (this is also uncertain)
	UFUNCTION(NetMulticast, Reliable)//@fixme:mark as unreliable once we move the 'state' out of our scharacter
	void MulticastHealthChanged(AActor* Instigator, float NewHealth, float Delta);


public:	
	// 'const' means you can only get value rather than tweak it
	UFUNCTION(BlueprintCallable)
	bool IsAlive() const; 

	

	UFUNCTION(BlueprintCallable)
	bool ApplyHealthChange(AActor* InstigatorActor,float Delta);


	UFUNCTION(BlueprintCallable)
	bool IsFullHealth() const;

	UFUNCTION(BlueprintCallable)
	float GetHealthMax() const;

	UFUNCTION(BlueprintCallable)
	float GetHealth() const;
	
	
	UFUNCTION(BlueprintCallable)
	bool Kill(AActor* Instigator);


	UPROPERTY(BlueprintAssignable)
	FOnHealthChanged OnHealthChanged;


};
