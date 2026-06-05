// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SAICharacter.generated.h"


class UPawnSensingComponent;
class USAttributeComponent;
class UUserWidget;
class USWorldUserWidget;
class USActionComponent;

UCLASS()
class ACTIONROGUELIKE_API ASAICharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASAICharacter();
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


protected:

	USWorldUserWidget* ActiveHealthBar;

	UPROPERTY(EditAnywhere,Category = "UI")
	TSubclassOf<UUserWidget> HealthBarWidgetClass;
	
	//Widget to display when bot fiest spot a player
	UPROPERTY(EditDefaultsOnly,Category = "UI")
	TSubclassOf<UUserWidget> SpottedWidgetClass;
	


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USAttributeComponent* AttributeComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UPawnSensingComponent* PawnSensingComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USActionComponent* ActionComp;


	// This TimeToHit name is tied to the material, so don't change it
	//Material paramter for Hitflash
	UPROPERTY(VisibleAnywhere,Category = "Effects")
	FName TimeToHitParamName;

	//Key for AI Blackboard 'TargetActor'
	UPROPERTY(VisibleAnywhere)
	FName TargetActorKey;
	

	virtual void PostInitializeComponents() override;

	//marked as unreliable,because this on pawn seen actually play a cosmetic role---losing it will not harm your play
	UFUNCTION(NetMulticast,Unreliable)
	void MulticastOnPawnSeen(APawn* Pawn);
	
	UFUNCTION()
	void OnPawnSeen(APawn* Pawn);
	
	UFUNCTION()
	void OnHealthChanged(AActor* InstigatorActor, USAttributeComponent* OwningComp, float NewHealth, float Delta);

	UFUNCTION(BlueprintCallable,Category = "AI")
	void SetTargetActor(AActor* NewTarget);
	
	UFUNCTION(BlueprintCallable,Category = "AI")
	AActor* GetTargetActor() const;

};
