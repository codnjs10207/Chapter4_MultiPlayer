// CXPlayerState.cpp (업데이트)

#include "CXPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

ACXPlayerState::ACXPlayerState()
{
	bReplicates = true;
}

void ACXPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ACXPlayerState, Coins, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ACXPlayerState, BonusMaxSpeed, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ACXPlayerState, SpeedPenalty, COND_OwnerOnly);
}

void ACXPlayerState::AddCoins(int32 Amount)
{
	if (!HasAuthority()) return;
	Coins += Amount;
	ForceNetUpdate();
	OnCoinsChanged.Broadcast(Coins);
}

bool ACXPlayerState::RemoveCoins(int32 Amount)
{
	if (!HasAuthority() || Coins < Amount) return false;
	Coins -= Amount;
	ForceNetUpdate();
	OnCoinsChanged.Broadcast(Coins);
	return true;
}

void ACXPlayerState::AddBonusMaxSpeed(float Amount)
{
	if (!HasAuthority()) return;
	BonusMaxSpeed += Amount;
	ForceNetUpdate();
	OnBonusMaxSpeedChanged.Broadcast(GetEffectiveBonusMaxSpeed());
}

void ACXPlayerState::ApplyWheelPenalty(float Penalty)
{
	if (!HasAuthority()) return;

	// 어떤 값이 들어와도 음수 패널티로 처리, 최저값 제한
	SpeedPenalty = FMath::Max(SpeedPenalty - FMath::Abs(Penalty), MinSpeedPenalty);
	ForceNetUpdate();
	OnBonusMaxSpeedChanged.Broadcast(GetEffectiveBonusMaxSpeed());

	if (!GetWorldTimerManager().IsTimerActive(PenaltyRecoveryTimerHandle))
	{
		GetWorldTimerManager().SetTimer(PenaltyRecoveryTimerHandle, this,
			&ACXPlayerState::RecoverSpeedPenalty, 0.1f, true);
	}
}

void ACXPlayerState::RecoverSpeedPenalty()
{
	// 0을 넘어서 양수가 되지 않도록 Min(0, ...)
	SpeedPenalty = FMath::Min(0.0f, SpeedPenalty + PenaltyRecoveryPerSecond * 0.1f);
	ForceNetUpdate();
	OnBonusMaxSpeedChanged.Broadcast(GetEffectiveBonusMaxSpeed());

	if (SpeedPenalty >= 0.0f)
	{
		GetWorldTimerManager().ClearTimer(PenaltyRecoveryTimerHandle);
	}
}

void ACXPlayerState::OnRep_Coins() { OnCoinsChanged.Broadcast(Coins); }
void ACXPlayerState::OnRep_BonusMaxSpeed() { OnBonusMaxSpeedChanged.Broadcast(GetEffectiveBonusMaxSpeed()); }

FString ACXPlayerState::GetPlayerStateInfo() const
{
	return FString::Printf(TEXT("Coins : %d\nBonus Max Speed : %+.0f km/h"),
		Coins, GetEffectiveBonusMaxSpeed());
}