// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerupActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"


// Sets default values
ASPowerupActor::ASPowerupActor()
{
 	
	SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	SphereComp->SetCollisionProfileName("Powerup");
	//profile��pei zhi
	// The powerup level corresponds to which projectile level we set in the editor
	RootComponent = SphereComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// Use SphereComponent as the collision box, MeshComponent is just the visible model
	//ues shpere comp  to detect collision,mesh comp is just for visual effect;
	MeshComp->SetupAttachment(RootComponent);

	bIsActive = true;
	
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
	bIsActive = bNewIsActive;
	OnRep_IsActive();
}

void ASPowerupActor::OnRep_IsActive()
{
	SetActorEnableCollision(bIsActive);

	//set visibility on root  and all children----the second parameter's meaning
	RootComponent->SetVisibility(bIsActive, true);
}

void ASPowerupActor::Interact_Implementation(APawn* InstigatorPawn)
{
	// We'll leave this function for child classes to implement
	// logic in derived classes...
}
      
void ASPowerupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ASPowerupActor , bIsActive);
}
