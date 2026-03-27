// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerupActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"


// Sets default values
ASPowerupActor::ASPowerupActor()
{
 	
	SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	SphereComp->SetCollisionProfileName("Powerup");
	//profile：配置
	//powerup的等级就是我们在editor里面设置的哪个projectile一个等级
	RootComponent = SphereComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//将spherecomp作为碰撞箱，meshcomp只是可见模型
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
	//可是我们把这个函数留给子类去实现吧
	// logic in derived classes...
}
      