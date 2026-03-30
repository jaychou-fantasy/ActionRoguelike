// Fill out your copyright notice in the Description page of Project Settings.


#include "SItemChest.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetWork.h"

// Sets default values
ASItemChest::ASItemChest()
{

	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>("BaseMesh");
	RootComponent = BaseMesh;

	LidMesh = CreateDefaultSubobject<UStaticMeshComponent>("LidMesh");
	LidMesh->SetupAttachment(BaseMesh);

	TargetPitch = 110.0;

	SetReplicates(true);
}



void ASItemChest::Interact_Implementation(APawn* InstigatorPawn) 
{
	bLidOpened = !bLidOpened;
	OnRep_LidOpened();
	// On the server, it must also be triggered manually; otherwise, the server would miss the lid-opening action
	// On the client, it is triggered automatically
}

void ASItemChest::OnRep_LidOpened()
{
	float CurrPitch = bLidOpened ? TargetPitch : 0.0f;
	LidMesh->SetRelativeRotation(FRotator(CurrPitch, 0, 0));
}

void ASItemChest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replication only works from server to client
	// Simply marking it as replicated is not enough — it must also be registered here
	// This means when the value of bLidOpened changes, it gets transmitted to all clients
	DOREPLIFETIME(ASItemChest, bLidOpened);
}


