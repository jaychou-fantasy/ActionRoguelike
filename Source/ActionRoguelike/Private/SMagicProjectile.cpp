// Fill out your copyright notice in the Description page of Project Settings.


#include "SMagicProjectile.h"
#include "Components/SphereComponent.h"
#include "SAttributeComponent.h"
#include "SGameplayFunctionLibrary.h"
#include "SActionComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "SActionEffect.h"



// Sets default values
ASMagicProjectile::ASMagicProjectile()
{
	SphereComp->SetSphereRadius(20.0f);
	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &ASMagicProjectile::OnActorOverlap);
	
	DamageAmount = 20.0f;
}


void ASMagicProjectile::OnActorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//这里otheractor，comp都是被击中的actor和具体被击中的component
	//这里spawnparams得到的instigator就排上用场了，getinstigaor()就是得到这个
	//然后如果otheractor(也就是和projectile onoverlap的那个actor) == instigator,那么下面的所有都不执行，也就没有damage和emitter了
	if (OtherActor && OtherActor != GetInstigator())
	{
		//FName Muzzle = "Muzzle_01";
		//static FGameplayTag Tag = FGameplayTag::RequestGameplayTag("Status.Parrying");
		//这是请求tag和单纯请求一个变量的区别，后面取药request，然后static就可以让这种请求只发生一次


		//格挡->速度反向
		USActionComponent* ActionComp = Cast<USActionComponent>(OtherActor->GetComponentByClass(USActionComponent::StaticClass()));
		if (ActionComp && ActionComp->ActiveGameplayTags.HasTag(ParryTag))
		{
			MoveComp->Velocity = -MoveComp->Velocity;
			
			SetInstigator(Cast<APawn>(OtherActor));

			return;
			//意思是如果被防反了，那么就不触发爆炸
		}
		
		//USAttributeComponent* AttributeComp = USAttributeComponent::GetAttributes(OtherActor);
		if (USGameplayFunctionLibrary::ApplyDirectionalDamage(GetInstigator(), OtherActor, DamageAmount, SweepResult))
		{
			Explode();
			//添加给otheractor以burnningaction
			if (ActionComp && ensure(BurningActionClass))
			{
				ActionComp->AddAction(GetInstigator(), BurningActionClass);
			}
		}

		//Cast<USAttributeComponent>(OtherActor->GetComponentByClass(USAttributeComponent::StaticClass()));//"给我这个类的身份证" - 返回一个代表该类的唯一标识
		//UE 运行时通过这个"身份证"来识别和操作类型
		//if (OtherComp)
		//{
			//if (AttributeComp)
			//{
				//AttributeComp->ApplyHealthChange(-20.0f);
				//Destroy();

				//minus in front of DamageAmount to apply the change as damage,not healing
				          /*AttributeComp->ApplyHealthChange(GetInstigator(), -DamageAmount);*/
				//这里的attributecomp是otheractor的attributecomp
				
				//然后apply healthchange会触发onhealthchange delegate，然后broadcast给所有用adddynamic绑定的的函数，触发扣血啥的
				          /*UE_LOG(LogTemp, Warning, TEXT("SMagicProjectile: Applying %f damage to %s"), DamageAmount, *GetNameSafe(OtherActor));*/
				//only explode when we hit something valid
				          /*Explode();*/
			//}
		//}
	}
}
