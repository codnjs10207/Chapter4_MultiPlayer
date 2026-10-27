// CXSpeedItem.cpp

#include "CXSpeedItem.h"
#include "CXPawn.h"
#include "CXPlayerController.h"
#include "CXPlayerState.h"

void ACXSpeedItem::OnPickedUp(ACXPawn* Character)
{
	

	if (HasAuthority() && Character)
	{
		APlayerController* PC = Cast<APlayerController>(Character->GetController());
		if (PC && PC->GetPlayerState<ACXPlayerState>())
		{
			ACXPlayerState* PS = PC->GetPlayerState<ACXPlayerState>();
			if (PS)
			{
				PS->AddBonusMaxSpeed(BonusSpeed);
			}
		}
	}

	Super::OnPickedUp(Character);
}
