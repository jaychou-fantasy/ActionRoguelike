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
	
	//disabled on capsule to let projectiles pass through capsule and hit meshcomp instead
	//however this gonna prevent every world_dynamic object to try to overlap with this minion
	//so we create a "projectile" collision obejct to specificly prevent the collision between "projectile" and "pawn"
	/*GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);*/
	
	//enabled on mesh to react to the incoming projectiles
	GetMesh()->SetGenerateOverlapEvents(true);

	//ensure we receive a controller when spawned in the level by our gamemode
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	TimeToHitParamName = "TimeToHit";
	TargetActorKey = "TargetActor";

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

void ASAICharacter::SetTargetActor(AActor* NewTarget)
{
	AAIController* AIC = Cast<AAIController>(GetController());//usually, return a controller,but it's AI,so cast to it
	if (ensure(AIC))
	{
		AIC->GetBlackboardComponent()->SetValueAsObject(TargetActorKey, NewTarget);
	}
}

AActor* ASAICharacter::GetTargetActor() const
{
	AAIController* AIC = Cast<AAIController>(GetController());
	if (AIC)
	{
		return Cast<AActor>(AIC->GetBlackboardComponent()->GetValueAsObject(TargetActorKey));
	}
	return nullptr;
}

void ASAICharacter::OnPawnSeen(APawn* Pawn)
{
	//ignore if target already set
	if (GetTargetActor() != Pawn)
	{
		SetTargetActor(Pawn);
	}
	//NetMulticast will cast thi to all clients,as well as itself
	MulticastOnPawnSeen(Pawn);
}

void ASAICharacter::MulticastOnPawnSeen_Implementation(APawn* Pawn)
{
	
	//widget name use default.it will not bother us anyway/anyhow
	USWorldUserWidget* NewWidget = CreateWidget<USWorldUserWidget>(GetWorld(),SpottedWidgetClass);
	if (NewWidget)
	{
		NewWidget->AttachedActor = this;
		//Index of 10(or anything higher than default of 0) place this on top of any other widegt
		//May end up behind minion health widget otherwise
		NewWidget->AddToViewport(10);
	}
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
				AIC->GetBrainComponent()->StopLogic("Killed");// BrainComp is the brain component of the AI Controller �� essentially the brain of the brain
			}
			//ragdoll
			GetMesh()->SetAllBodiesSimulatePhysics(true);
			GetMesh()->SetCollisionProfileName("RagDoll");
			// No collision box when dead
			GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			GetCharacterMovement()->DisableMovement();
			// This stops the ragdoll from jittering �� after falling apart, immediately disable collision and stop jittering
			//set lifespan
			SetLifeSpan(3.0f);
		}
	}
}

