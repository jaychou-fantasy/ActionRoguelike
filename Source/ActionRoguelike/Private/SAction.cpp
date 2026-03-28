// Fill out your copyright notice in the Description page of Project Settings.


#include "SAction.h"
#include "SActionComponent.h"
#include "../ActionRoguelike.h"//上一个文件夹的headerfile
#include "Net/UnrealNetwork.h"

//在action被Newobject<USAction>( , )的时候调用，进行初始化
void USAction::Initialize(USActionComponent* NewActionComp)
{
	ActionComp = NewActionComp;
}

bool USAction::CanStart_Implementation(AActor* Instigator)
{
	USActionComponent* Comp = GetOwningComponent();
	if (IsRunning())//如果正在运行，那么就不能启动，即防止一个action多次同时启动，还没结束就启动下一个了
		//其实只要把自己的tag加到blockedtags就行了，反正是先运行再加入tag的，不会影响自己的那一次action
	{
		UE_LOG(LogTemp, Warning, TEXT("Action already running"));
		return false;
	}
	if (Comp->ActiveGameplayTags.HasAny(BlockedTags))
	{
		return false;
	}

	return true;
}


//这里是在实现uasaction里最基本的进行和结束的通告内容，至于真正的开始和结束，是在component里面去调用的

void USAction::StartAction_Implementation(AActor* Instigator)
{
	//UE_LOG(LogTemp, Log, TEXT("Running: %s"), *GetNameSafe(this));
	LogOnScreen(this, FString::Printf(TEXT("Started: %s"), *ActionName.ToString()), FColor::Green);

	USActionComponent* Comp = GetOwningComponent();
	//都不用检查ensure，因为action就是在actioncomponent的addaction里面newobject出来的，所以肯定是有的
	Comp->ActiveGameplayTags.AppendTags(GrantsTags);

	bIsRunning = true;
}

void USAction::StopAction_Implementation(AActor* Instigator)
{
	//UE_LOG(LogTemp, Log, TEXT("Stopped: %s"), *GetNameSafe(this));
	LogOnScreen(this, FString::Printf(TEXT("Stopped: %s"), *ActionName.ToString()), FColor::White);

	//ensureAlways(bIsRunning);//因为只有在actioncomp里面确定了isrunning==0了才会启动stopaction，所以直接ensurealways，无需if了（在comp里才if）

	USActionComponent* Comp = GetOwningComponent();
	Comp->ActiveGameplayTags.RemoveTags(GrantsTags);

	bIsRunning = false;
}



//确保getworld函数能够正确的返回一个uworld指针，万一character不存在，那么他的getworld也就失效了，这里就顺带做一个ensure
//AActor；UActorComponent；UWorldSubsystem这些才天然知道world
UWorld* USAction::GetWorld() const
{
	// Outer is set "when!" creating action via   NewObject<T>(当时的this就是outer，是ActionComponent类型的)
	AActor* Actor = Cast<AActor>(GetOuter());
	if (Actor)
	{
		return Actor->GetWorld();
	}
	return nullptr;
}

USActionComponent* USAction::GetOwningComponent() const
{
	//AActor* Actor = Cast<AActor>(GetOuter());
	//return Actor->GetComponentByClass(USActionComponent::StaticClass());
	//*就是getcomponentbyclass是iterate所有Actor的comp来寻找actioncomp，太影响开销了，所以不如直接在action.cpp的uproperty就设置好它对应的actioncomp
	return ActionComp;
}

//在client里，如果server start了，那么isrunning变成true，onrep在client run startaction，这样server的player在client里就显示同样的动作了
//所以要把上面stop的ensurealways给改了：因为client给sever_player执行stop的时候，bisrunning早就replicate为false了
void USAction::OnRep_IsRunning()
{
	if (bIsRunning)
	{
		StartAction(nullptr);
	}
	else
	{
		StopAction(nullptr);
	}
}


bool USAction::IsRunning() const
{
	return bIsRunning;
}


void USAction::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USAction, bIsRunning);
	DOREPLIFETIME(USAction, ActionComp);
}