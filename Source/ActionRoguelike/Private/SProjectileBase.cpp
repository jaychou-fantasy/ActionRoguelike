// Fill out your copyright notice in the Description page of Project Settings.


#include "SProjectileBase.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Camera/CameraShakeBase.h"


// Sets default values
ASProjectileBase::ASProjectileBase()
{
	SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	SphereComp->SetCollisionProfileName("Projectile");  //就是我们再project setting里面设置的projectile类一样的
	SphereComp->OnComponentHit.AddDynamic(this, &ASProjectileBase::OnActorHit);
	RootComponent = SphereComp;

	EffectComp = CreateDefaultSubobject<UParticleSystemComponent>("EffectComp");
	EffectComp->SetupAttachment(RootComponent);

	AudioComp = CreateDefaultSubobject<UAudioComponent>("AudioComp");
	AudioComp->SetupAttachment(RootComponent);

	MoveComp = CreateDefaultSubobject<UProjectileMovementComponent>("MovementComp");
	MoveComp->bRotationFollowsVelocity = true;
	//这个意味着物体的旋转是跟着速度的，由速度决定方向，这样投射物如果是长方形，就不会出现飞行方向和头的朝向不一致的问题了
	MoveComp->bInitialVelocityInLocalSpace = true; //速度相对自生坐标系 not世界坐标系
	MoveComp->ProjectileGravityScale = 0.0f;
	MoveComp->InitialSpeed = 2000;


	ImpactShakeInnerRadius = 0.0f;
	ImpactShakeOuterRadius = 1500.0f;


	//DelayedDestroy = 0.05f;

	SetReplicates(true);
}

void ASProjectileBase::OnActorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Explode();
}

//_Implementation from it being marked as BlueprintNativeEvent
void ASProjectileBase::Explode_Implementation()//_implement means  native virsion
{
	// Check to make sure we aren't already being 'destroyed'
	// Adding ensure to see if we encounter this situation at all
	if(IsValid(this))
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, ImpactVFX, GetActorLocation(), GetActorRotation());

		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
		
		UGameplayStatics::PlayWorldCameraShake(this, ImpactShake, GetActorLocation(), ImpactShakeInnerRadius, ImpactShakeOuterRadius);


		/*EffectComp->DeactivateSystem();

		MoveComp->StopMovementImmediately();
		SetActorEnableCollision(false);*/
		Destroy();
		//GetWorldTimerManager().SetTimer(TimerHandle_TimeDestroyed, this, &ASProjectileBase::Delayed_Destroy, DelayedDestroy);
	}
}

/*void ASProjectileBase::Delayed_Destroy()
{
	Destroy();
}*/

void ASProjectileBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	// SphereComp->IgnoreActorWhenMoving(GetInstigator(), true);
}





