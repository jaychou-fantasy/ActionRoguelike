// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "SActionComponent.generated.h"

class USAction;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ACTIONROGUELIKE_API USActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Tags")
	FGameplayTagContainer ActiveGameplayTags;


	UFUNCTION(BlueprintCallable,Category = "Actions")
	void AddAction(AActor* Instigator,TSubclassOf<USAction> ActionClass);

	UFUNCTION(BlueprintCallable,Category = "Actions")
	void RemoveAction(USAction* ActionToRemove);


	UFUNCTION(BlueprintCallable,Category = "Actions")
	bool StartActionByName(AActor* Instigator, FName ActionName);

	UFUNCTION(BlueprintCallable, Category = "Actions")
	bool StopActionByName(AActor* Instigator, FName ActionName);



	// To replicate a UObject's subobject, you must set replication on the component it's attached to
	bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;



	USActionComponent();

protected:

	UFUNCTION(Server,Reliable)
	void ServerStartActionByName(AActor* Instigator, FName ActionName);

	UFUNCTION(Server, Reliable)
	void ServerStopActionByName(AActor* Instigator, FName ActionName);

	//accomplish in beginplay 
	//Grant Abilities at GameStart
	UPROPERTY(EditAnywhere, Category = "Actions")
	TArray<TSubclassOf<USAction>> DefaultActions;


	UPROPERTY(Replicated)
	TArray<USAction*> Actions;


	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
