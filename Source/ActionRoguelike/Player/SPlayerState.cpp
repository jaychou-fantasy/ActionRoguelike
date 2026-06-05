// Fill out your copyright notice in the Description page of Project Settings.


#include "SPlayerState.h"
#include "SSaveGame.h"
#include "Net/UnrealNetwork.h"

ASPlayerState::ASPlayerState()
{
	SetReplicates(true);
}

void ASPlayerState::SavePlayerState_Implementation(USSaveGame* SaveObject)
{
	if (SaveObject)
	{
		SaveObject->Credits = Credits;
	}
}

void ASPlayerState::LoadPlayerState_Implementation(USSaveGame* SaveObject)
{
	if (SaveObject)
	{
		//just do the opppsite(on the contrary)
		
		//ensure we trigger credits changed event
		AddCredits(SaveObject->Credits);
		//Credits = SaveObject->Credits;
		UE_LOG(LogTemp,Warning,TEXT("PlayerState loaded"))
	}
	
}

int32 ASPlayerState::GetCredits() const
{
	return Credits;
}

void ASPlayerState::AddCredits(int32 Delta)
{
	//Avoid plus nagative delta
	//0 means unnecessary(but sometimes)
	if (!ensure(Delta >= 0.0f))
	{
		return;
	}

	Credits += Delta;
	OnCreditsChanged.Broadcast(this, Credits, Delta);
}

bool ASPlayerState::RemoveCredits(int32 Delta)
{
	//Avoid subtract nagative delta
	if (!ensure(Delta >= 0.0f))
	{
		return false;
	}
	//Avioid lacking credits
	if (Credits < Delta)
	{
		return false;
	}

	Credits -=Delta;
	OnCreditsChanged.Broadcast(this, Credits, -Delta);
	return true;
}

void ASPlayerState::OnRep_Credits(int32 OldCredits)
{
	OnCreditsChanged.Broadcast(this,Credits,Credits-OldCredits);
}
// void ASPlayerState::MulticastCredits_Implementation(float NewCredits, float Delta)
// {
// 	OnCreditsChanged.Broadcast(this, NewCredits, Delta);
// }

void ASPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) __const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ASPlayerState,Credits);
}