// CXPlayerState.h (업데이트)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CXPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCoinsChangedDelegate, int32, NewCoins);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBonusMaxSpeedChangedDelegate, float, NewBonusMaxSpeed);

UCLASS()
class CX_API ACXPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ACXPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Coins
	UFUNCTION(BlueprintPure, Category = "Coins")
	int32 GetCoins() const { return Coins; }

	UFUNCTION(BlueprintCallable, Category = "Coins")
	void AddCoins(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Coins")
	bool RemoveCoins(int32 Amount);

	// Bonus Max Speed
	UFUNCTION(BlueprintPure, Category = "Speed")
	float GetBonusMaxSpeed() const { return BonusMaxSpeed; }

	// 실제 적용되는 추가 최대 속도 = 보너스 + 패널티(0 이하)
	UFUNCTION(BlueprintPure, Category = "Speed")
	float GetEffectiveBonusMaxSpeed() const { return BonusMaxSpeed + SpeedPenalty; }

	// UI 표시용: 원화 + 추가 최대 속도를 한 줄 문자열로
	UFUNCTION(BlueprintPure, Category = "UI")
	FString GetPlayerStateInfo() const;

	UFUNCTION(BlueprintCallable, Category = "Speed")
	void AddBonusMaxSpeed(float Amount);

    // 추가: 바퀴 패널티 적용
    UFUNCTION(BlueprintCallable, Category = "Speed")
    void ApplyWheelPenalty(float Penalty);

	// Delegates for UI binding
	UPROPERTY(BlueprintAssignable, Category = "Coins")
	FOnCoinsChangedDelegate OnCoinsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Speed")
	FOnBonusMaxSpeedChangedDelegate OnBonusMaxSpeedChanged;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_Coins, VisibleAnywhere, BlueprintReadOnly, Category = "Coins")
	int32 Coins = 0;

	UPROPERTY(ReplicatedUsing = OnRep_BonusMaxSpeed, VisibleAnywhere, BlueprintReadOnly, Category = "Speed")
	float BonusMaxSpeed = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_BonusMaxSpeed, VisibleAnywhere, BlueprintReadOnly, Category = "Speed")
	float SpeedPenalty = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Speed")
	float MinSpeedPenalty = -60.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Speed")
	float PenaltyRecoveryPerSecond = 5.0f;

	void RecoverSpeedPenalty();
	FTimerHandle PenaltyRecoveryTimerHandle;

	UFUNCTION()
	void OnRep_Coins();

	UFUNCTION()
	void OnRep_BonusMaxSpeed();
};
