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

	//只在c++里面调用，无需在bp里面使用，所以不需要加function
	void FindBestInteractable();

	//要加uproperty，这样当交互的actor被销毁了，这个指针也会随之销毁，避免dangled pointer
	UPROPERTY()
	AActor* FocusedActor;


	//接下来的三个变量都是为了，更灵活的tweak交互范围 + 交互对象的collision type
	UPROPERTY(EditDefaultsOnly,Category = "Trace")
	float TraceDistance;
	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	float TraceRadius;
	//他只是一个枚举值
	//ECC_Visibility；ECC_Pawn；ECC_WorldDynamic这些都是他的枚举值，所以才写TEnumByte< >
	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	TEnumAsByte<ECollisionChannel> CollisionChannel;


	UPROPERTY()
	USWorldUserWidget* DefaultWidgetInstance;
	//上面那个是实例，是实打实的widget
	//下面那个是class类型，是在bp编辑器里面设置的，设置类别后，这里的这个是实例会变成这个class的实例
	UPROPERTY(EditDefaultsOnly,Category = "UI")
	TSubclassOf<USWorldUserWidget> DefaultWidgetClass;




public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
