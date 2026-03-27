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



	//就是血量是replicated，而这个播报只不过是触发一些ui变化这种，所以其实没必要reliable浪费一大堆网络资源
	//relicated就是会侧重结果。100-80-60-40可能只会显示100-40，但是multicate则是注重每一次的变化。更突出连续的变化
	//还有就是unreliable：：在两个人物相距100km的时候，游戏可能不会把这两个东西视为relevant，但是一旦接近。才会触发同步，这就是unreliable
	//Net Relevancy / Net Cull Distance：：：Actor 远了就不再同步
	//当申明为unreliable，才会考虑relevancy（存疑）
	//如果是reliable，就会忽略relevancy（存疑）
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
