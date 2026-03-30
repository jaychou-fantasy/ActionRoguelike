// Fill out your copyright notice in the Description page of Project Settings.


#include "SDashProjectile.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
ASDashProjectile::ASDashProjectile()
{
	TeleportDelay = 0.2f;
	DetonateDelay = 0.2f;

	MoveComp->InitialSpeed = 6000.0f;
	
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASDashProjectile::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(TimerHandle_DelayedDetonate, this, &ASDashProjectile::Explode, DetonateDelay);
	
}

void ASDashProjectile::Explode_Implementation()
{
	// Clear timer if the Explode was already called through another source like OnActorHit
	GetWorldTimerManager().ClearTimer(TimerHandle_DelayedDetonate);

	UGameplayStatics::SpawnEmitterAtLocation(this, ImpactVFX, GetActorLocation(), GetActorRotation());

	EffectComp->DeactivateSystem();
	MoveComp->StopMovementImmediately();

	FTimerHandle TimerHandle_DelayedTeleport;
	GetWorldTimerManager().SetTimer(TimerHandle_DelayedTeleport, this, &ASDashProjectile::TeleportInstigator, TeleportDelay);
	
	// Skip base implementation as it will destroy actor
	// we need to stay alive a bit longer to finish the 2nd timer
	// Super::Explode_Implementation();
}


void ASDashProjectile::TeleportInstigator()
{
	
	AActor* ActorToTeleport = GetInstigator();
	if (ensure(ActorToTeleport))
	{
		// Keep instigator rotation or it may end up jarring
		//jarring:  Creates a shocking, uncomfortable, or unsettling sensation due to being uncoordinated, unexpected, or overly intense
		ActorToTeleport->TeleportTo(GetActorLocation(), ActorToTeleport->GetActorRotation(), false, true); // bNOCheck = true means it doesn't check for capsule collision with the ground or other overlaps ¡ª teleports immediately
		UE_LOG(LogTemp, Warning, TEXT("TeleportInstigator Called!"));

	}
	Destroy();
}
