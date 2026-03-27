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
		//闪光附着在 手的meshcomp上
		UGameplayStatics::SpawnEmitterAttached(CastingEffect, Character->GetMesh(), HandSocketName, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget);

		FTimerHandle TimerHandle_AttackDelay;
		FTimerDelegate Delegate;
		Delegate.BindUFunction(this, "AttackDelay_Elapsed", Character);

		GetWorld()->GetTimerManager().SetTimer(TimerHandle_AttackDelay, Delegate, AttackAnimDelay, false);//因为正常在character的子类里面，可以直接用gettimermanager，但是在外面需要用getworld
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

		//这个版本是aim at the direction of the controller,then we edit towards the "crosshair"

		//GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnTM, SpawnParams);//Spawn Transform Matrix

		FCollisionShape Shape;
		Shape.SetSphere(20.0f);

		//Ignore Player;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(InstigatorCharacter);
		//查询条件（过滤规则）”，主要用来指定在碰撞检测过程中要忽略哪些对象、是否进行复杂碰撞、是否返回物理材质等。
		/*
		常见用途：
			AddIgnoredActor()：忽略某个 Actor，不参与碰撞
			设置 bTraceComplex 等开关
			设置调试信息
			它 不决定参与检测的对象类型，只是告诉引擎“怎么查、忽略谁”
		*/

		FCollisionObjectQueryParams ObjParams;
		ObjParams.AddObjectTypesToQuery(ECC_WorldDynamic);
		ObjParams.AddObjectTypesToQuery(ECC_WorldStatic);
		ObjParams.AddObjectTypesToQuery(ECC_Pawn);
		// 这是 “对象类型过滤器”，用于指定碰撞检测应当对哪些 ObjectType 的物体做检测。

		FHitResult Hit;
		FVector TraceStart = InstigatorCharacter->GetPawnViewLocation();
		FVector TraceEnd = TraceStart + (InstigatorCharacter->GetControlRotation().Vector() * 5000);


		FRotator ProjRotation;
		//true if we got to a blocking hit (Alternative :SweepingSingleByChannel with ECC_WorldDynamic)
		if (GetWorld()->SweepSingleByObjectType(Hit, TraceStart, TraceEnd, FQuat::Identity, ObjParams, Shape, Params))
		{
			//adjust location to end uo at crosshair look-at
			//如果准心碰到东西了，那么可以直接让projectile 发射到  碰撞点
			TraceEnd = Hit.ImpactPoint;
			/*
				把你提供的 XAxis 当作物体局部坐标系的 X 轴（前向）
				自动计算出一个合理的 Y 轴和 Z 轴（保证是正交基向量）
				最终生成一个 FRotationMatrix（旋转矩阵）
			*/
		}
		//fall_back since we failed to find any blocking hit
		ProjRotation = FRotationMatrix::MakeFromX(TraceEnd - HandLocation).Rotator();
		//只能退而求其次往一个稍微没有这么偏的位置

		//begin spawn projectile
		FTransform SpawnMT = FTransform(ProjRotation, HandLocation);
		GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnMT, SpawnParams);
	}

	StopAction(InstigatorCharacter);
}



