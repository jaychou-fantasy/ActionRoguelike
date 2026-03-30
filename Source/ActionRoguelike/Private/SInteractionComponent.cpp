// Fill out your copyright notice in the Description page of Project Settings.


#include "SInteractionComponent.h"
#include "SGamePlayInterface.h"
#include "DrawDebugHelpers.h"
#include "SWorldUserWidget.h"

static TAutoConsoleVariable<bool> CVarDrawInteraction(TEXT("su.InteractionDebugDraw"), false, TEXT("Enable Debug Lines for Interaction Component."), ECVF_Cheat);


// Sets default values for this component's properties
USInteractionComponent::USInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...

	TraceDistance = 500.0f;
	TraceRadius = 30.0f;
	CollisionChannel = ECC_WorldDynamic;
}



// Called when the game starts
void USInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
	// ...
}

// Called every frame
void USInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	

	APawn* MyPawn = Cast<APawn>(GetOwner());
	// For example, on the client, there are interaction components for you and other players.
	// On your client, all these interaction components would run Interact logic continuously.
	// This check ensures only the locally controlled player runs this function for interaction.
	// IsLocallyControlled allows us to execute this only for the character I am locally controlling.
	// However, this also means that on the client, Player 1's "Press E" widget will also be displayed for Player 2, Player 3, etc.
	if (MyPawn->IsLocallyControlled())
	{
		// Check for interactable objects every tick
		FindBestInteractable();
	}
}

void USInteractionComponent::FindBestInteractable()
{
	bool bDebugDraw = CVarDrawInteraction.GetValueOnGameThread();

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(CollisionChannel);

	AActor* MyOwner = GetOwner();

	FVector EyeLoaction;
	FRotator EyeRotation;
	MyOwner->GetActorEyesViewPoint(EyeLoaction, EyeRotation);

	FVector End = EyeLoaction + (EyeRotation.Vector() * TraceDistance);//1000cm

	//FHitResult Hit;
	//bool bBlockingHit = GetWorld()->LineTraceSingleByObjectType(Hit, EyeLoaction, End, ObjectQueryParams);
	// If there's a default, it can be ignored

	TArray<FHitResult> Hits;

	FCollisionShape Shape;
	Shape.SetSphere(TraceRadius);

	bool bBlockingHit = GetWorld()->SweepMultiByObjectType(Hits, EyeLoaction, End, FQuat::Identity, ObjectQueryParams, Shape);
	//FQuat::Identity  means "Empty Rotation";

	FColor LineColor = bBlockingHit ? FColor::Green : FColor::Red;


	//clear ref befrore filling it
	FocusedActor = nullptr;

	for (FHitResult Hit : Hits)
	{
		if (bDebugDraw)
		{
			DrawDebugSphere(GetWorld(), Hit.ImpactPoint, TraceRadius, 32, LineColor, false, 2.0f);
			// 32 is the number of segments for the sphere
		}

		AActor* HitActor = Hit.GetActor();//get the actor we hit
		if (HitActor)
		{
			if (HitActor->Implements<USGamePlayInterface>())
			{
				// Populate FocusedActor ¡ª essentially the previous hit actor
				FocusedActor = HitActor;
				
				break;
				//once i hit something,break the interface between hitactor&mypawn
				//that you wont hit multiple actors in one interaction
			}
		}
		//break the forloop and drawdebugsphere wont work then
	}
	
	// If a hit actor (interactable object) is found
	if (FocusedActor)
	{
		//create widget
		if (DefaultWidgetInstance == nullptr && DefaultWidgetClass)
		{
			DefaultWidgetInstance = CreateWidget<USWorldUserWidget>(GetWorld(), DefaultWidgetClass);
		}
		//attach to minion,  then add to viewport
		if (DefaultWidgetInstance)
		{
			DefaultWidgetInstance->AttachedActor = FocusedActor;

			if (!DefaultWidgetInstance->IsInViewport())
			{
				DefaultWidgetInstance->AddToViewport();
			}
		}
	}
	// If no interactable object is found: 
	// 1. Do nothing
	// 2. If it was previously added to the viewport, remove it
	else
	{
		if (DefaultWidgetInstance)
		{
			DefaultWidgetInstance->RemoveFromParent();
		}
	}



	if (bDebugDraw)
	{
		DrawDebugLine(GetWorld(), EyeLoaction, End, LineColor, false, 2.0f, 0, 2.0f);//2s,2cm
	}
}


void USInteractionComponent::PrimaryInteract()
{
	ServerInteract(FocusedActor);
}


// If you don't pass in an extra InFocus parameter, then if the server's view doesn't have a FocusedActor, 
// even if the client has a FocusedActor (only locally), calling PrimaryInteract won't work on the server.
// This means FocusedActor is a local variable ¡ª different clients can have different true/false values for their FocusedActor.
void USInteractionComponent::ServerInteract_Implementation(AActor* InFocus)
{
	// If the interact key is pressed but there's no object to interact with, show debug info
	if (InFocus == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, "No Focus Actor To Interact!");
		return;
	}

	// If there is an object to interact with (FindBestInteractable, called every tick, will display the interact widget), proceed with interaction.
	// The actual feasibility of the interaction is already determined by the focusedactor == nullptr check above.
	// The purpose of FindBestInteractable is to determine if there's a FocusedActor and display the widget accordingly.
	APawn* MyPawn = Cast<APawn>(GetOwner());

	/////// This line means InFocus->Interact(MyPawn); just written in the UE interface execution syntax.
	ISGamePlayInterface::Execute_Interact(InFocus, MyPawn);
	///////
}
