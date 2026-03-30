// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerupActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"


// Sets default values
ASPowerupActor::ASPowerupActor()
{
 	
	SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	SphereComp->SetCollisionProfileName("Powerup");
	//profile£ºpei zhi
	// The powerup level corresponds to which projectile level we set in the editor
	RootComponent = SphereComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// Use SphereComponent as the collision box, MeshComponent is just the visible model
	//ues shpere comp  to detect collision,mesh comp is just for visual effect;
	MeshComp->SetupAttachment(RootComponent);

	SetReplicates(true);
}


void ASPowerupActor::HideAndCooldownPowerup()
{
	SetPowerupState(false);

	GetWorldTimerManager().SetTimer(TimerHandle_RespawnTimer, this, &ASPowerupActor::ShowPowerup, RespawnTime);

}

void ASPowerupActor::ShowPowerup()
{
	SetPowerupState(true);
}

void ASPowerupActor::SetPowerupState(bool bNewIsActive)
{
	SetActorEnableCollision(bNewIsActive);

	//set visibility on root  and all children
	RootComponent->SetVisibility(bNewIsActive, true);
}

void ASPowerupActor::Interact_Implementation(APawn* InstigatorPawn)
{
	// We'll leave this function for child classes to implement
	// logic in derived classes...
}
      