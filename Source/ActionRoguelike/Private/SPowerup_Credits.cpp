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
	//其实没有必要，只是为了防御，可能网络lag也会导致pawn不存在
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
