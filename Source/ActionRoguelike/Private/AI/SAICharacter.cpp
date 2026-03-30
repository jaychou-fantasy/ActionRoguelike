// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/SAICharacter.h"
#include "Perception/PawnSensingComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SAttributeComponent.h"
#include "BrainComponent.h"
#include "SWorldUserWidget.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SActionComponent.h"


// Sets default values
ASAICharacter::ASAICharacter()
{
 	
	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>("PawnSensingComp");

	AttributeComp = CreateDefaultSubobject<USAttributeComponent>("AttributeComp");

	ActionComp = CreateDefaultSubobject<USActionComponent>("ActionComp");

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);


	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	TimeToHitParamName = "TimeToHit";

}


// Called to bind functionality to input
void ASAICharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
}

void ASAICharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	// OnSeePawn is a dynamic delegate that is triggered when a new Pawn is perceived
	// This delegate then calls our defined OnPawnSeen function
	PawnSensingComp->OnSeePawn.AddDynamic(this,&ASAICharacter::OnPawnSeen);
	AttributeComp->OnHealthChanged.AddDynamic(this, &ASAICharacter::OnHealthChanged);
}

void ASAICharacter::SetTargetActor(AActor* T_Actor)
{
	AAIController* AIC = Cast<AAIController>(GetController());//usually, return a controller,but it's AI,so cast to it
	if (ensure(AIC))
	{
		AIC->GetBlackboardComponent()->SetValueAsObject("TargetActor", T_Actor);
	}
}

void ASAICharacter::OnPawnSeen(APawn* Pawn)
{
	SetTargetActor(Pawn);
	DrawDebugString(GetWorld(), GetActorLocation(), "PLAYER SPOTTED", nullptr, FColor::White, 4.0f, true);// True :adds a shadow to the string, making it more noticeable
}


void ASAICharacter::OnHealthChanged(AActor* InstigatorActor, USAttributeComponent* OwningComp, float NewHealth, float Delta)
{
	if (Delta < 0.0f)
	{
		//if attacked,get instigator as target
		if (InstigatorActor != this)
		{
			SetTargetActor(InstigatorActor);
		}

		if (ActiveHealthBar == nullptr)
		{
			ActiveHealthBar = CreateWidget<USWorldUserWidget>(GetWorld(), HealthBarWidgetClass);//actually,the widget is projected on the world,thus it
			if (ActiveHealthBar)
			{
				UE_LOG(LogTemp, Log, TEXT("create widget successfully!"));
				ActiveHealthBar->AttachedActor = this;
				ActiveHealthBar->AddToViewport();
				//when addtoviewport. it will call on "beginevent construct" in Blueprint;
			}
		}
		

		//Died
		GetMesh()->SetScalarParameterValueOnMaterials(TimeToHitParamName, GetWorld()->TimeSeconds);

		if (NewHealth <= 0.0f)
		{
			//stop bt
			AAIController* AIC = Cast<AAIController>(GetController());
			if (AIC)
			{
				AIC->GetBrainComponent()->StopLogic("Killed");// BrainComp is the brain component of the AI Controller ¡ª essentially the brain of the brain
			}
			//ragdoll
			GetMesh()->SetAllBodiesSimulatePhysics(true);
			GetMesh()->SetCollisionProfileName("RagDoll");
			// No collision box when dead
			GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			GetCharacterMovement()->DisableMovement();
			// This stops the ragdoll from jittering ¡ª after falling apart, immediately disable collision and stop jittering
			//set lifespan
			SetLifeSpan(3.0f);
		}
	}
}

