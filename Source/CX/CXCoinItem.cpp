// CXCoinItem.cpp

#include "CXCoinItem.h"
#include "CXPawn.h"
#include "CXPlayerController.h"
#include "CXPlayerState.h"

void ACXCoinItem::OnPickedUp(ACXPawn* Character)
{
	

	if (HasAuthority() && Character)
	{
		APlayerController* PC = Cast<APlayerController>(Character->GetController());
		if (PC && PC->GetPlayerState<ACXPlayerState>())
		{
			ACXPlayerState* PS = PC->GetPlayerState<ACXPlayerState>();
			if (PS)
			{
				PS->AddCoins(CoinAmount);
			}
		}
	}

	Super::OnPickedUp(Character);
}
