// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SProjectileBase.generated.h"

class USphereComponent;
class UParticleSystem;
class UProjectileMovementComponent;
class UParticleSystemComponent;

class UCameraShakeBase;
class USoundCue;
class UAudioComponent;

UCLASS(ABSTRACT)
class ACTIONROGUELIKE_API ASProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASProjectileBase();


protected:
	//UPROPERTY(EditDefaultsOnly, Category = "DDDebug")
	//float DelayedDestroy;

	UPROPERTY(EditDefaultsOnly,Category = "Effects")
	UParticleSystem* ImpactVFX;//a visual effect
	
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	USoundCue* ImpactSound;

	//camera shake component and asset
	UPROPERTY(EditDefaultsOnly,Category = "Effects|Shake")
	TSubclassOf<UCameraShakeBase> ImpactShake;

	UPROPERTY(EditDefaultsOnly, Category = "Effects|Shake")
	float ImpactShakeInnerRadius;

	UPROPERTY(EditDefaultsOnly, Category = "Effects|Shake")
	float ImpactShakeOuterRadius;
	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* SphereComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProjectileMovementComponent* MoveComp;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "Components")
	UParticleSystemComponent* EffectComp; //a comp

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UAudioComponent* AudioComp;

	//任何的asset比如soundcue，impactvfx（visual effects）啥的，必须附着在components上面 

	

	UFUNCTION()
	virtual void OnActorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	// BlueprintNativeEvent = C++ base implementation, can be expanded in Blueprints--->
	// BlueprintCallable to allow child classes to trigger explosions     ===   blueprint readonly in varible <callable is for function>
	// BlueprintNativeEvent means can be overrideen in Blueprint,and has _implement version in C++'

	UFUNCTION(BlueprintNativeEvent,BlueprintCallable)
	void Explode();

	virtual void PostInitializeComponents() override;


	//FTimerHandle TimerHandle_TimeDestroyed;
	//void Delayed_Destroy();
};
