// Fill out your copyright notice in the Description page of Project Settings.


#include "SExplosiveBarrel.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"

// Sets default values
ASExplosiveBarrel::ASExplosiveBarrel()
{
 	
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
	MeshComp->SetSimulatePhysics(true);
	RootComponent = MeshComp;

	ForceComp = CreateDefaultSubobject<URadialForceComponent>("ForceComp");
	ForceComp->SetupAttachment(MeshComp);

	ForceComp->SetAutoActivate(false);

	ForceComp->Radius = 750.0f;
	ForceComp->ImpulseStrength = 2500.0f;
	//Alternative 200000.0f if bImpulseVelChange = false;
	//Optional,ignores "Mass" of other objects(if false, the impulse will be much higher to push most objects depending on Mass)
	ForceComp->bImpulseVelChange = true;
	
	ForceComp->AddCollisionChannelToAffect(ECC_WorldDynamic);
	
	
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}


void ASExplosiveBarrel::PostInitializeComponents() 
{

	Super::PostInitializeComponents();

	MeshComp->OnComponentHit.AddDynamic(this, &ASExplosiveBarrel::OnActorHit);
	//thisÊÇASExplosiveBarrel*  // The pointer to the current bucket instance
}

void ASExplosiveBarrel::OnActorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalInpulse, const FHitResult& Hit)
{

	ForceComp->FireImpulse();
	//logging example to make sure we reached the event
	UE_LOG(LogTemp, Log, TEXT("OnActorHit in Explosiv eBarrel"));

	//%s :string
	//%f :float
	//logs : "OtherActor:My_Actor1,at gametime:124.4"
	UE_LOG(LogTemp, Warning, TEXT("OtherActor: %s, at game time: %f"), *GetNameSafe(OtherActor), GetWorld()->TimeSeconds );//when the type is string ,REMEMBER to add *
	
	FString CombinedString = FString::Printf(TEXT("Hit at location: %s"), *Hit.ImpactPoint.ToString());
	//that works as well::::FString CombinedString = TEXT("Hit at location: ") + Hit.ImpactPoint.ToString();
	// Printf is a common method in UE for creating dynamic strings, especially suited for complex string construction that requires variable insertion.
	// Using Format is equivalent to Printf: FString CombinedString = FString::Format(TEXT("Hit at location: {0}"), {Hit.ImpactPoint.ToString()});





	DrawDebugString(GetWorld(), Hit.ImpactPoint, CombinedString, nullptr, FColor::Green, true);
	//we've already fix it to the imapctpoint,so the passing actor parameter should be nullptr,otherwise it would always attatched to the explosive barrel
	//P.S. this params usually be "nullptr"
}


// Called when the game starts or when spawned
void ASExplosiveBarrel::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASExplosiveBarrel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


