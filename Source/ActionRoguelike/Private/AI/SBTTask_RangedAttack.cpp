// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/SBTTask_RangedAttack.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "SAttributeComponent.h"

USBTTask_RangedAttack::USBTTask_RangedAttack()
{
	MaxBulletError_Angle = 2.0f;
}


EBTNodeResult::Type USBTTask_RangedAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* MyController = OwnerComp.GetAIOwner();
	if (ensure(MyController))
	{
		ACharacter* MyPawn = Cast<ACharacter>(MyController->GetPawn());
		//cast it to character attribute for "GetMesh()",that's super convenient to get sockett--muuzle01
		if (MyPawn == nullptr)
		{
			return EBTNodeResult::Failed;
		}
		//prevent whip a corpse(bian shi)
		if (!USAttributeComponent::IsActorAlive(MyPawn))
		{
			return EBTNodeResult::Failed;
		}
		//// Omitted the step of getting AttributeComp and using it to check IsAlive()

		//--get params--
		AActor* TargetActor = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject("TargetActor"));
		if (TargetActor == nullptr)
		{
			return EBTNodeResult::Failed;
		}

		//---spawning---
		FVector MuzzleLocation = MyPawn->GetMesh()->GetSocketLocation("Muzzle_01");
		FVector Direction = TargetActor->GetActorLocation() - MuzzleLocation;
		FRotator MuzzleRotation = Direction.Rotation();
		//random const error angle
		MuzzleRotation.Pitch += FMath::RandRange(0.0f, +MaxBulletError_Angle);
		MuzzleRotation.Yaw += FMath::RandRange(-MaxBulletError_Angle, +MaxBulletError_Angle);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;//chcek if the actor can just move alittle bit to spawn without collision
		SpawnParams.Instigator = MyPawn;
		// This prevents the projectile from colliding with the AI itself

		AActor* NewProjectile = GetWorld()->SpawnActor<AActor>(ProjectileClass,MuzzleLocation, MuzzleRotation, SpawnParams);
		
		return NewProjectile ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
	}
	return EBTNodeResult::Failed;
	
	
	
}


