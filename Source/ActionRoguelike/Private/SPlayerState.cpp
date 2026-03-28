// Fill out your copyright notice in the Description page of Project Settings.


#include "SPlayerState.h"

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