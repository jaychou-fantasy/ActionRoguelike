// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SPlayerState.generated.h"

//DELEGATE必须使用UPROPERTY来声明----BlueprintAssignable，不然无法在bp中使用
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCreditsChanged, ASPlayerState*, PlayerState, int32, NewCredits, int32, Delta);
//创建delegate，到时候broadcast。再在需要接受这个信息的函数那边，adddynamic

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API ASPlayerState : public APlayerState
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly,Category = "Credits")
	int32 Credits;

public:
	UFUNCTION(BlueprintCallable,Category = "Credits")
	int32 GetCredits() const;

	UFUNCTION(BlueprintCallable,Category = "Credits") // <Category|SubCategory
	void AddCredits(int32 Delta);

	//因为需要用于判断条件，比如能减去50credits才能购买，所以需要返回bool来判断
	UFUNCTION(BlueprintCallable, Category = "Credits")
	bool RemoveCredits(int32 Delta);

	UPROPERTY(BlueprintAssignable,Category = "Evenet")
	FOnCreditsChanged OnCreditsChanged;


};
