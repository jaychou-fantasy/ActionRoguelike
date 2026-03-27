// Fill out your copyright notice in the Description page of Project Settings.


#include "SItemChest.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetWork.h"

// Sets default values
ASItemChest::ASItemChest()
{

	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>("BaseMesh");
	RootComponent = BaseMesh;

	LidMesh = CreateDefaultSubobject<UStaticMeshComponent>("LidMesh");
	LidMesh->SetupAttachment(BaseMesh);

	TargetPitch = 110.0;

	SetReplicates(true);
}



void ASItemChest::Interact_Implementation(APawn* InstigatorPawn) 
{
	bLidOpened = !bLidOpened;
	OnRep_LidOpened();
	//在server上也要手动触发一次，不然server上就少开盖的动作了
	//在client上则是自动触发
}

void ASItemChest::OnRep_LidOpened()
{
	float CurrPitch = bLidOpened ? TargetPitch : 0.0f;
	LidMesh->SetRelativeRotation(FRotator(CurrPitch, 0, 0));
}

void ASItemChest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//relicated只有server到client
	//光replicated不够，一定要在这里注册过才行
	//意思就是当这个blidopend的值变化了，就传输给所有的client
	DOREPLIFETIME(ASItemChest, bLidOpened);
}


