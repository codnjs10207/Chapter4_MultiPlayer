// CXGameState.cpp

#include "CXGameState.h"
#include "CXPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"

void ACXGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACXGameState, GameStatus);
	DOREPLIFETIME(ACXGameState, RemainingTime);
	DOREPLIFETIME(ACXGameState, Winners);
}

void ACXGameState::SetGameStatus(EGameStatus NewStatus)
{
	if (!HasAuthority()) return;
	GameStatus = NewStatus;
	ForceNetUpdate();
	OnRep_GameStatus();
}

void ACXGameState::SetRemainingTime(float NewTime)
{
	if (!HasAuthority()) return;
	RemainingTime = NewTime;
	ForceNetUpdate();
	OnRep_RemainingTime();
}

void ACXGameState::AddWinner(ACXPlayerState* PlayerState)
{
	if (!HasAuthority()) return;
	if (PlayerState && !Winners.Contains(PlayerState))
	{
		Winners.Add(PlayerState);
		ForceNetUpdate();
	}
}

void ACXGameState::ClearWinners()
{
	if (!HasAuthority()) return;
	Winners.Empty();
	ForceNetUpdate();
}

void ACXGameState::OnRep_GameStatus()
{
}

void ACXGameState::OnRep_RemainingTime()
{
}

FString ACXGameState::GetGameStatusInfo() const
{
	const int32 Seconds = FMath::CeilToInt(RemainingTime);

	switch (GameStatus)
	{
	case EGameStatus::Waiting:
		return FString::Printf(TEXT("Waiting... Start in %d"), Seconds);
	case EGameStatus::Playing:
		return FString::Printf(TEXT("Playing - Time Left : %d"), Seconds);
	case EGameStatus::Finished:
		return TEXT("Finished");
	}
	return FString();
}

FString ACXGameState::GetResultText(APlayerState* LocalPlayerState) const
{
	if (GameStatus != EGameStatus::Finished || Winners.IsEmpty())
	{
		return FString();
	}

	FString WinnerNames;
	for (ACXPlayerState* Winner : Winners)
	{
		if (IsValid(Winner))
		{
			if (WinnerNames.IsEmpty() == false) WinnerNames += TEXT(", ");
			WinnerNames += Winner->GetPlayerName();
		}
	}

	const bool bIsWinner = Winners.Contains(Cast<ACXPlayerState>(LocalPlayerState));

	FString Result;
	if (bIsWinner)
	{
		Result = (Winners.Num() > 1) ? TEXT("DRAW - Co-Winner!") : TEXT("YOU WIN!");
	}
	else
	{
		Result = TEXT("YOU LOSE...");
	}

	return Result + TEXT("\nWinner : ") + WinnerNames;
}