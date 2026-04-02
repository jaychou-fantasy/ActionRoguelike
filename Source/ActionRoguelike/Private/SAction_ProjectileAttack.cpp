// Fill out your copyright notice in the Description page of Project Settings.


#include "SAction_ProjectileAttack.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

USAction_ProjectileAttack::USAction_ProjectileAttack()
{
	AttackAnimDelay = 0.2f;
	HandSocketName = "Muzzle_01";
}

void USAction_ProjectileAttack::StartAction_Implementation(AActor* Instigator)
{
	Super::StartAction_Implementation(Instigator);
	ACharacter* Character = Cast<ACharacter>(Instigator);
	if (Character)
	{
		Character->PlayAnimMontage(AttackAnim);
		// The flash effect is attached to the hand's MeshComponent
		UGameplayStatics::SpawnEmitterAttached(CastingEffect, Character->GetMesh(), HandSocketName, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget);

		//the above sentence are same time triggered no matter you are server/client,and we just dont want the client accidently create an extra projectile
		//so we just need to limit the timer only
		if (Instigator->HasAuthority())
		{
			FTimerHandle TimerHandle_AttackDelay;
			FTimerDelegate Delegate;
			Delegate.BindUFunction(this, "AttackDelay_Elapsed", Character);

			GetWorld()->GetTimerManager().SetTimer(TimerHandle_AttackDelay, Delegate, AttackAnimDelay, false);// Normally, inside a Character subclass, you can directly use GetTimerManager, 
			// but outside of it, you need to use GetWorld
		}
	}
}

void USAction_ProjectileAttack::AttackDelay_Elapsed(ACharacter* InstigatorCharacter)
{
	//it would only warn at the first time "ensure return false" 
	// and the game process wont stop
	// while "check" would occur immediately halt;
	//ensureAlawys ---- that warns you every tick
	if (ensureAlways(ProjectileClass)) {

		FVector HandLocation = InstigatorCharacter->GetMesh()->GetSocketLocation(HandSocketName);
		//FTransform SpawnTM = FTransform(GetControlRotation(), HandLocation);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;//chcek if the actor can just move alittle bit to spawn without collision
		SpawnParams.Instigator = InstigatorCharacter;

		//this version is : aim at the direction of the controller,then we edit towards the "crosshair"

		//GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnTM, SpawnParams);//Spawn Transform Matrix

		FCollisionShape Shape;
		Shape.SetSphere(20.0f);

		//Ignore Player;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(InstigatorCharacter);
		// Query conditions (filter rules) ¡ª primarily used to specify which objects to ignore during collision detection, whether to use complex collision, whether to return physical materials, etc.
		/*
		Common uses:
			AddIgnoredActor(): Ignores a specific Actor, excluding it from collision
			Set bTraceComplex and other toggles
			Set debug information
			It does NOT determine which object types participate in the detection ¡ª it only tells the engine "how to query and who to ignore"
		*/

		FCollisionObjectQueryParams ObjParams;
		ObjParams.AddObjectTypesToQuery(ECC_WorldDynamic);
		ObjParams.AddObjectTypesToQuery(ECC_WorldStatic);
		ObjParams.AddObjectTypesToQuery(ECC_Pawn);
		// This is the "Object Type Filter," used to specify which ObjectType objects the collision detection should check against.

		FHitResult Hit;
		FVector TraceStart = InstigatorCharacter->GetPawnViewLocation();
		FVector TraceEnd = TraceStart + (InstigatorCharacter->GetControlRotation().Vector() * 5000);


		FRotator ProjRotation;
		//true if we got to a blocking hit (Alternative :SweepingSingleByChannel with ECC_WorldDynamic)

		if (GetWorld()->SweepSingleByObjectType(Hit, TraceStart, TraceEnd, FQuat::Identity, ObjParams, Shape, Params))
		{
			// Adjust location to end up at crosshair look-at
			// If the crosshair hits something, we can directly spawn the projectile at the impact point
			TraceEnd = Hit.ImpactPoint;
			/*
				Takes your provided XAxis as the local forward direction of the object
				Automatically calculates a suitable Y and Z axis (ensures an orthonormal basis)
				Finally generates an FRotationMatrix (rotation matrix)
			*/
		}
		// Fall back since we failed to find any blocking hit
		ProjRotation = FRotationMatrix::MakeFromX(TraceEnd - HandLocation).Rotator();
		// Settle for a slightly less accurate direction as a fallback(tui er qiu qi ci)

		//begin spawn projectile
		FTransform SpawnMT = FTransform(ProjRotation, HandLocation);
		GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnMT, SpawnParams);
	}

	StopAction(InstigatorCharacter);
}



