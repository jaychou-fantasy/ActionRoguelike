// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "SAction.generated.h"

class UWorld;
/**
 * 
 */


//Blueprintable就是表示这个类可以在蓝图里面创建子类，就比如创建一个BTTaskNode的子类，这里的BTTaskNode就是Blueprintable的
UCLASS(Blueprintable)
class ACTIONROGUELIKE_API USAction : public UObject
{
	GENERATED_BODY()
	

protected:
	//comp这种绝对会参与到replicate，bp环节的东西一定要uproperty
	UPROPERTY(Replicated)
	USActionComponent* ActionComp;

	/* Tags added to owning actor when activated, removed when action stops */
	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTagContainer GrantsTags;

	/* Action can only start if OwningActor has none of these Tags applied */
	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTagContainer BlockedTags;


	UPROPERTY(ReplicatedUsing = "OnRep_IsRunning")
	bool bIsRunning;

	//现在是client的action能传到server，但是server的action改变发生却传不到client
	UFUNCTION()
	void OnRep_IsRunning();

public:
	void Initialize(USActionComponent* NewActionComp);

	UPROPERTY(EditDefaultsOnly,Category = "Action")
	bool bAutoStart;

	UFUNCTION(BlueprintCallable,Category = "Action")
	bool IsRunning() const;

	UFUNCTION(BlueprintNativeEvent,Category = "Action")
	bool CanStart(AActor* Instigator);

	UFUNCTION(BlueprintCallable,Category = "Action")
	USActionComponent* GetOwningComponent() const;

	//只要是blueprint nativeevent的函数，在c++里面都要有一个默认的实现：也就是_implementation函数
	UFUNCTION(BlueprintNativeEvent,Category = "Action")
	void StartAction(AActor* Instigator);

	UFUNCTION(BlueprintNativeEvent,BlueprintCallable, Category = "Action")
	void StopAction(AActor* Instigator);


	UPROPERTY(EditDefaultsOnly,Category = "Action")
	FName ActionName;

	UWorld* GetWorld() const override;

	//virtual不用写，当我们开始override的的时候，直接override就行
	//
	// 只有Uobject需要写这个，来源于component的不需要（但是是setisreplicatedbydefault）
	// 
	//因为这个funct是默认返回false：不允许联网
	//现在改成true，就是允许联网
	bool IsSupportedForNetworking() const override
	{
		return true;
	}
};
