// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SInteractionComponent.generated.h"

class USWorldUserWidget;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ACTIONROGUELIKE_API USInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:


	/// <summary>
	void PrimaryInteract();
	/// <summary>



public:	
	// Sets default values for this component's properties
	USInteractionComponent();

protected:

	// Reliable - Will always arrive, eventually. Request will be re-sent unless an acknowledgment was received.
	// Unreliable - Not guaranteed, packet can get lost and won't retry.

	//that actually an RPC function--Server to Client
	//and Server means to run on the Server
	//Client requests an application to Server to run "ServerInteract" 
	UFUNCTION(Server,Reliable)
	void ServerInteract(AActor* InFocus);



	// Called when the game starts
	virtual void BeginPlay() override;

	// Only called in C++, doesn't need to be used in Blueprint, so UFUNCTION is not required
	void FindBestInteractable();

	// UPROPERTY must be added so that when the interactable actor is destroyed, this pointer is also properly cleared, preventing a dangling pointer
	UPROPERTY()
	AActor* FocusedActor;


	// The next three variables are for more flexible tweaking of interaction range + the collision type of interactable objects
	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	float TraceDistance;
	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	float TraceRadius;
	// This is an enum value
	// ECC_Visibility, ECC_Pawn, ECC_WorldDynamic are examples of its enum values, hence we use TEnumByte<>
	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	TEnumAsByte<ECollisionChannel> CollisionChannel;


	UPROPERTY()
	USWorldUserWidget* DefaultWidgetInstance;
	// The above is an instance — an actual widget
	// The following is a class type, set in the Blueprint editor. After setting the class type here, the instance above will become an instance of that class
	UPROPERTY(EditDefaultsOnly,Category = "UI")
	TSubclassOf<USWorldUserWidget> DefaultWidgetClass;




public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
