// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerup_Credits.h"
#include "SPlayerState.h"

ASPowerup_Credits::ASPowerup_Credits()
{
	CreditsAmount = 80;
}

void ASPowerup_Credits::Interact_Implementation(APawn* InstigatorPawn)
{
	//ensure has player
	// Not strictly necessary, just a safeguard — network lag could also cause the Pawn to not exist
	if (!ensure(InstigatorPawn))
	{
		return;
	}
	if (ASPlayerState* PS = InstigatorPawn->GetPlayerState<ASPlayerState>())
	{
		PS->AddCredits(CreditsAmount);
		HideAndCooldownPowerup();
	}
}
