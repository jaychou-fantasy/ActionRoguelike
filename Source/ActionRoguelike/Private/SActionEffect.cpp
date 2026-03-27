// Fill out your copyright notice in the Description page of Project Settings.


#include "SActionEffect.h"
#include "SActionComponent.h"

USActionEffect::USActionEffect()
{
	bAutoStart = true;
}

void USActionEffect::StartAction_Implementation(AActor* Instigator)
{
	//implementation的super也要带implementation
	Super::StartAction_Implementation(Instigator);
	
	if (Duration > 0.0f)
	{
		FTimerDelegate Delegate;
		Delegate.BindUFunction(this, "StopAction",Instigator);
		//时间到了就触发结束，无需loop
		GetWorld()->GetTimerManager().SetTimer(DurationHandle, Delegate, Duration, false);
	}

	if (Period > 0.0f)
	{
		FTimerDelegate Delegate;
		Delegate.BindUFunction(this, "ExecutePeriodicEffect", Instigator);
		//每过一次period，都触发一次，这样会导致在最后一次period结束的时候，duration也结束了，直接触发stopaction，会少一次，所以到时候得在stopaction里面补一次
		//当然可以先判断period，然后再判断duration，（这样相比于上一个方案，次数不变），但是他会在任务收到伤害的一瞬间，然后就触发ExecutePeriodicEffect。但是我要的是收到伤害后，过1s，然后在收到灼伤伤害
		GetWorld()->GetTimerManager().SetTimer(PeriodHandle, Delegate, Period, true);
	}
}

void USActionEffect::StopAction_Implementation(AActor* Instigator) 
{
	//就是如果这一段的period（最后duration小到触发stopaction时，那“一段”的period的剩余时间） 小于 KINDA_SMALL_NUMBER(一个无限接近0的float数)----意思就是period的剩余时间也近乎0，那么就再触发最后一次periodeffect
	if (GetWorld()->GetTimerManager().GetTimerRemaining(PeriodHandle) < KINDA_SMALL_NUMBER)
	{
		ExecutePeriodicEffect(Instigator);
	}

	Super::StopAction_Implementation(Instigator);

	GetWorld()->GetTimerManager().ClearTimer(DurationHandle);
	GetWorld()->GetTimerManager().ClearTimer(PeriodHandle);
	//计时器什么的删了，接下来就是把这个action给remove掉
	USActionComponent* Comp = GetOwningComponent();
	if (Comp)
	{
		Comp->RemoveAction(this);
	}
}

void USActionEffect::ExecutePeriodicEffect_Implementation(AActor* Instigator)
{
	//空着，反正到时候的具体实现效果是去blueprint里面override。
}