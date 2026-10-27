// CXGameMode.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CXGameMode.generated.h"

class ACXGameState;

UCLASS()
class CX_API ACXGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACXGameMode();

protected:
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual void BeginPlay() override;
	virtual void Tick(float Delta) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

	UPROPERTY(EditDefaultsOnly)
	float GameDuration = 120.0f;

	UPROPERTY(EditDefaultsOnly)
	float CountdownDuration = 3.0f;

	UPROPERTY(EditDefaultsOnly)
	int32 MinPlayersToStart = 1;

private:
	bool IsPlayerStartAvailable(const APlayerStart* PlayerStart) const;

	UFUNCTION(BlueprintCallable, Category = "Game Status")
	void StartGameCountdown();

	UFUNCTION(BlueprintCallable, Category = "Game Status")
	void StartGame();

	UFUNCTION(BlueprintCallable, Category = "Game Status")
	void EndGame();

	UFUNCTION(BlueprintCallable, Category = "Game Status")
	void DetermineWinners();

	float CountdownTimer = 0.0f;
	float GameTimer = 0.0f;
	bool bCountdownStarted = false;
	bool bGameStarted = false;

	FTimerHandle GameTimerHandle;
	void OnGameTimerTick();

	UPROPERTY(EditDefaultsOnly)
	FVector VehicleSpawnCheckHalfExtent = FVector(250.0, 120.0, 80.0);

	UPROPERTY(EditDefaultsOnly)
	FVector VehicleSpawnCheckOffset = FVector(0.0, 0.0, 80.0);
};