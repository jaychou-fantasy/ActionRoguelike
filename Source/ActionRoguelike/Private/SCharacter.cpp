// Fill out your copyright notice in the Description page of Project Settings.


#include "SCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SInteractionComponent.h"
#include "SAttributeComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SActionComponent.h"

// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>("SpringArmComp");
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);  

	CameraComp = CreateDefaultSubobject<UCameraComponent>("CameraComp");
	CameraComp->SetupAttachment(SpringArmComp);


	InteractionComp = CreateDefaultSubobject<USInteractionComponent>("InteractionComp");

	AttributeComp = CreateDefaultSubobject<USAttributeComponent>("AtrtributeComp");

	ActionComp = CreateDefaultSubobject<USActionComponent>("ActionComp");


	GetCharacterMovement()->bOrientRotationToMovement = true;

	bUseControllerRotationYaw = false;

	TimeToHitName = "TimeToHit";
	

}

void ASCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	AttributeComp->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHealthChanged);
}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Axis is a continuous input that needs to be checked constantly
	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoverForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	// Action is a discrete input, only triggered when pressed or released
	PlayerInputComponent->BindAction("PrimaryAttack", IE_Pressed, this, &ASCharacter::PrimartAttack);
	PlayerInputComponent->BindAction("SecondaryAttack", IE_Pressed, this, &ASCharacter::BlackholeAttack);
	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &ASCharacter::Dash);


	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASCharacter::Jump);

	PlayerInputComponent->BindAction("PrimaryInteract", IE_Pressed, this, &ASCharacter::PrimaryInteract);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ASCharacter::SprintStart);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ASCharacter::SprintStop);

}

FVector ASCharacter::GetPawnViewLocation() const
// When overriding, it looks at the lowest-level override ¡ª for example, APawn can override AActor, and ACharacter can override APawn, and so on
{
	return CameraComp->GetComponentLocation();
}

void ASCharacter::MoverForward(float Value)
{
	FRotator ControlRot = GetControlRotation();//Get the rotation of the controller(not the vector)
	ControlRot.Pitch = 0.0f;//look up and dowm
	ControlRot.Roll = 0.0f;//like the jet rolling upside down
	//We set it to 0.0f to avoid rotator to disturb the unrelated axis-----we only focus to yaw axis

	AddMovementInput(ControlRot.Vector(),Value);//input is a vector---GetActorForwardVector()->ControlRot.Vector()----they can transfer smoothly
	//previously,GetActorForwardVector() means when we press W key,actor would just go to where the actor faces.
	//when we set the GetControlRotation, the actor would go to where the CAMERA(CONTROLLER) faces.
}

void ASCharacter::MoveRight(float Value)
{
	FRotator ControlRot = GetControlRotation();
	ControlRot.Pitch = 0.0f;
	ControlRot.Roll = 0.0f;
	
	//X = Forward(red)
	//Y = Right(green)
	//Z = Up(blue)

	FVector RightVector = FRotationMatrix(ControlRot).GetScaledAxis(EAxis::Y);
	//as you know,FRotator class's Vector() can only get the X vector,so when you need the RightVector,you need to use function to achieve it

	AddMovementInput(RightVector, Value);
}

void ASCharacter::PrimartAttack()
{
	ActionComp->StartActionByName(this, "PrimaryAttack");
	//GetWorldTimerManager().SetTimer(TimerHandle_PrimaryAttack, this, &ASCharacter::PrimaryAttack_TimeElapsed, AttackAnimDelay);
	//GetWorldTimerManager().ClearTimer(TimerHandle_PrimaryAttack);
	
}

void ASCharacter::BlackholeAttack()
{
	ActionComp->StartActionByName(this, "Blackhole");
	//GetWorldTimerManager().SetTimer(TimerHandle_BlackholeAttack, this, &ASCharacter::BlackholeAttack_TimeElapsed, AttackAnimDelay);
}

void ASCharacter::Dash()
{
	ActionComp->StartActionByName(this, "Dash");
	//GetWorldTimerManager().SetTimer(TimerHandle_BlackholeAttack, this, &ASCharacter::Dash_TimeElapsed, AttackAnimDelay);
}

//when pressed ,then sprint,released stop sprinting
void ASCharacter::SprintStart()
{
	ActionComp->StartActionByName(this, "Sprint");
}

void ASCharacter::SprintStop()
{
	ActionComp->StopActionByName(this, "Sprint");
}



void ASCharacter::PrimaryInteract()
{
	InteractionComp->PrimaryInteract();
}


void ASCharacter::OnHealthChanged(AActor* InstigatorActor, USAttributeComponent* OwningComp, float NewHealth, float Delta)
{
	// Damage effect
	// Uses the same approach as the TargetDummy's material
	if (Delta < 0.0f)
	{
		GetMesh()->SetScalarParameterValueOnMaterials(TimeToHitName, GetWorld()->TimeSeconds);
	}
	
	if (NewHealth <= 0.0f && Delta < 0.0f)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		DisableInput(PC);
	}
}

void ASCharacter::HealSelf(float amount  /* =100 */)
{
	AttributeComp->ApplyHealthChange(this, amount);
}
