// CXGameState.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CXGameState.generated.h"

class ACXPlayerState;
class APlayerState;

UENUM()
enum class EGameStatus : uint8
{
	Waiting,
	Playing,
	Finished
};

UCLASS()
class CX_API ACXGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "Game Status")
	EGameStatus GetGameStatus() const { return GameStatus; }

	UFUNCTION(BlueprintCallable, Category = "Game Status")
	void SetGameStatus(EGameStatus NewStatus);

	UFUNCTION(BlueprintPure, Category = "Game Status")
	float GetRemainingTime() const { return RemainingTime; }

	UFUNCTION(BlueprintCallable, Category = "Game Status")
	void SetRemainingTime(float NewTime);

	UFUNCTION(BlueprintPure, Category = "Winners")
	const TArray<ACXPlayerState*>& GetWinners() const { return Winners; }

	UFUNCTION(BlueprintCallable, Category = "Winners")
	void AddWinner(ACXPlayerState* PlayerState);

	UFUNCTION(BlueprintCallable, Category = "Winners")
	void ClearWinners();

	// UI 표시용: 게임 상태 + 남은 시간
	UFUNCTION(BlueprintPure, Category = "UI")
	FString GetGameStatusInfo() const;

	// UI 표시용: 로컬 플레이어 기준 승리/패배 결과 (Finished가 아니면 빈 문자열)
	UFUNCTION(BlueprintPure, Category = "UI")
	FString GetResultText(APlayerState* LocalPlayerState) const;

 protected:
	UPROPERTY(ReplicatedUsing = OnRep_GameStatus, VisibleAnywhere, Category = "Game Status")
	EGameStatus GameStatus = EGameStatus::Waiting;

	UPROPERTY(ReplicatedUsing = OnRep_RemainingTime, VisibleAnywhere, Category = "Game Status")
	float RemainingTime = 0.0f;

	UPROPERTY(Replicated, VisibleAnywhere, Category = "Winners")
	TArray<ACXPlayerState*> Winners;

	UFUNCTION()
	void OnRep_GameStatus();

	UFUNCTION()
	void OnRep_RemainingTime();
};