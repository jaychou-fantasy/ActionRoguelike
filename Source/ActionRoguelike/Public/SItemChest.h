// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SGamePlayInterface.h"
#include "SItemChest.generated.h"
//"generate" library must be the last one to be included


UCLASS()
class ACTIONROGUELIKE_API ASItemChest : public AActor,public ISGamePlayInterface
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	float TargetPitch;

	void Interact_Implementation(APawn* InstigatorPawn);//this  _Implementation is a grammer requirement for implementing  funct Interact in SGamePlayInterface;
	
public:	
	// Sets default values for this actor's properties
	ASItemChest();

protected:
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BaseMesh;

	UPROPERTY(BlueprintReadOnly,VisibleAnywhere)
	UStaticMeshComponent* LidMesh;

	
	UPROPERTY(ReplicatedUsing = "OnRep_LidOpened", BlueprintReadOnly)// RepNotify: Whenever the value of bLidOpened changes, it notifies all clients and triggers this function on the client side
	bool bLidOpened;
	
	UFUNCTION()
	void OnRep_LidOpened();

};
