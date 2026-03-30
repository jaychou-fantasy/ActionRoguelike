// Fill out your copyright notice in the Description page of Project Settings.


#include "SActionEffect.h"
#include "SActionComponent.h"

USActionEffect::USActionEffect()
{
	bAutoStart = true;
}

void USActionEffect::StartAction_Implementation(AActor* Instigator)
{
	// When calling super in an Implementation, you must also include the _Implementation suffix
	Super::StartAction_Implementation(Instigator);

	if (Duration > 0.0f)
	{
		FTimerDelegate Delegate;
		Delegate.BindUFunction(this, "StopAction", Instigator);
		// Triggers StopAction when the duration expires, no looping needed
		GetWorld()->GetTimerManager().SetTimer(DurationHandle, Delegate, Duration, false);
	}

	if (Period > 0.0f)
	{
		FTimerDelegate Delegate;
		Delegate.BindUFunction(this, "ExecutePeriodicEffect", Instigator);
		// Each time Period elapses, ExecutePeriodicEffect is triggered.
		// This means that when the last period ends, Duration also ends, triggering StopAction immediately,
		// which would cause one less execution. So we'll need to compensate by adding one more execution in StopAction.
		// Alternatively, we could check Period before Duration, which would keep the number of executions unchanged.
		// However, that approach would trigger ExecutePeriodicEffect instantly when damage is applied,
		// but the intended behavior is to apply the burning damage 1 second after receiving damage.
		GetWorld()->GetTimerManager().SetTimer(PeriodHandle, Delegate, Period, true);
	}
}

void USActionEffect::StopAction_Implementation(AActor* Instigator)
{
	// If the remaining time of the current period (when Duration is small enough to trigger StopAction) 
	// is less than KINDA_SMALL_NUMBER (a float infinitely close to 0) — meaning the period's remaining time is nearly zero — 
	// then trigger one more ExecutePeriodicEffect.
	if (GetWorld()->GetTimerManager().GetTimerRemaining(PeriodHandle) < KINDA_SMALL_NUMBER)
	{
		ExecutePeriodicEffect(Instigator);
	}

	Super::StopAction_Implementation(Instigator);

	GetWorld()->GetTimerManager().ClearTimer(DurationHandle);
	GetWorld()->GetTimerManager().ClearTimer(PeriodHandle);
	// Timers are cleared; next, remove the action from the component
	USActionComponent* Comp = GetOwningComponent();
	if (Comp)
	{
		Comp->RemoveAction(this);
	}
}

void USActionEffect::ExecutePeriodicEffect_Implementation(AActor* Instigator)
{
	// Left empty — the specific effect implementation will be overridden in Blueprint
}