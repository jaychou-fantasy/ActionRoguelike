// Fill out your copyright notice in the Description page of Project Settings.


#include "SInteractionComponent.h"
#include "SGamePlayInterface.h"
#include "DrawDebugHelpers.h"
#include "SWorldUserWidget.h"

static TAutoConsoleVariable<bool> CVarDrawInteraction(TEXT("su.InteractionDebugDraw"), false, TEXT("Enable Debug Lines for Interaction Component."), ECVF_Cheat);


// Sets default values for this component's properties
USInteractionComponent::USInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...

	TraceDistance = 500.0f;
	TraceRadius = 30.0f;
	CollisionChannel = ECC_WorldDynamic;
}



// Called when the game starts
void USInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
	// ...
}

// Called every frame
void USInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	

	APawn* MyPawn = Cast<APawn>(GetOwner());
	//就比如，在client上，有你和其他玩家的interactioncomp，在你的client上，这些intercomp会一直run interact，现在只让locally的player run this funct
	//但是islocallycontrole，就可以只计算我locally控制的这个角色-》进行交互
	//还会-》client上的player1显示“press e”，然后这个ui也会在player2，3...上面显示
	if (MyPawn->IsLocallyControlled())
	{
		//每个tick都要去寻找一下
		FindBestInteractable();
	}
}

void USInteractionComponent::FindBestInteractable()
{
	bool bDebugDraw = CVarDrawInteraction.GetValueOnGameThread();

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(CollisionChannel);

	AActor* MyOwner = GetOwner();

	FVector EyeLoaction;
	FRotator EyeRotation;
	MyOwner->GetActorEyesViewPoint(EyeLoaction, EyeRotation);

	FVector End = EyeLoaction + (EyeRotation.Vector() * TraceDistance);//1000cm

	//FHitResult Hit;
	//bool bBlockingHit = GetWorld()->LineTraceSingleByObjectType(Hit, EyeLoaction, End, ObjectQueryParams);
	//有default的可以不管

	TArray<FHitResult> Hits;

	FCollisionShape Shape;
	Shape.SetSphere(TraceRadius);

	bool bBlockingHit = GetWorld()->SweepMultiByObjectType(Hits, EyeLoaction, End, FQuat::Identity, ObjectQueryParams, Shape);
	//FQuat::Identity  means "Empty Rotation";

	FColor LineColor = bBlockingHit ? FColor::Green : FColor::Red;


	//clear ref befrore filling it
	FocusedActor = nullptr;

	for (FHitResult Hit : Hits)
	{
		if (bDebugDraw)
		{
			DrawDebugSphere(GetWorld(), Hit.ImpactPoint, TraceRadius, 32, LineColor, false, 2.0f);
			//32 是球体的细分段数（Segments）
		}

		AActor* HitActor = Hit.GetActor();//get the actor we hit
		if (HitActor)
		{
			if (HitActor->Implements<USGamePlayInterface>())
			{
				//填充focuedactor，其实就是执勤的hitactor
				FocusedActor = HitActor;
				
				break;
				//once i hit something,break the interface between hitactor&mypawn
				//that you wont hit multiple actors in one interaction
			}
		}
		//break the forloop and drawdebugsphere wont work then
	}
	
	//如果找到了hitactor（交互对象）
	if (FocusedActor)
	{
		//创建widget
		if (DefaultWidgetInstance == nullptr && DefaultWidgetClass)
		{
			DefaultWidgetInstance = CreateWidget<USWorldUserWidget>(GetWorld(), DefaultWidgetClass);
		}
		//attach到minion身上  然后add to viewport
		if (DefaultWidgetInstance)
		{
			DefaultWidgetInstance->AttachedActor = FocusedActor;

			if (!DefaultWidgetInstance->IsInViewport())
			{
				DefaultWidgetInstance->AddToViewport();
			}
		}
	}
	//如果没找到交互对象，1->什么都不做，2->如果之前add to viewport了，那就remove掉
	else
	{
		if (DefaultWidgetInstance)
		{
			DefaultWidgetInstance->RemoveFromParent();
		}
	}



	if (bDebugDraw)
	{
		DrawDebugLine(GetWorld(), EyeLoaction, End, LineColor, false, 2.0f, 0, 2.0f);//2s,2cm
	}
}


void USInteractionComponent::PrimaryInteract()
{
	ServerInteract(FocusedActor);
}


//如果不额外传入一个InFocus，那么如果server的视角里没有focusedActor，那你的client就算有foucusactor（只是他本地），primaryInteract了，还是无法在server上运行
//意思就是focusedActor是locally的变量，不同的client的foucuedactor的true/false是不一样的
void USInteractionComponent::ServerInteract_Implementation(AActor* InFocus)
{
	//如果遇到没有交互对象还一直按e的，就跳debug信息
	if (InFocus == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, "No Focus Actor To Interact!");
		return;
	}

	//如果有交互对象(那么前面每个tick调用一次的findbestinteractable就会帮你显示可交互的wiget)，那么就正常交互
	//其实交互能否完成的判断在前面那个focusedactor==nullptr已经判断好了，findbest那个函数的意义：：找到是否有focuedactor && 显示widget
	APawn* MyPawn = Cast<APawn>(GetOwner());

	///////意思就是InFocus->Interact(MyPawn);的意思，只是没有上面那个写法
	ISGamePlayInterface::Execute_Interact(InFocus, MyPawn);
	///////
}
