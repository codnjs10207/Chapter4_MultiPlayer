// CXGameMode.cpp

#include "CXGameMode.h"
#include "CXPlayerController.h"
#include "CXPlayerState.h"
#include "CXGameState.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "Subsystem/ChatSubsystem.h"

ACXGameMode::ACXGameMode()
{
	PlayerControllerClass = ACXPlayerController::StaticClass();
	PlayerStateClass = ACXPlayerState::StaticClass();
	GameStateClass = ACXGameState::StaticClass();
	PrimaryActorTick.bCanEverTick = true;
}

void ACXGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (ACXPlayerController* CXPlayerController = Cast<ACXPlayerController>(NewPlayer))
	{
		// 새로 들어온 클라이언트에게 이전 채팅 로그 전송
		if (UChatSubsystem* ChatSubsystem = GetWorld()->GetSubsystem<UChatSubsystem>())
		{
			const TArray<FChatData> ChatLog = ChatSubsystem->GetChatLog();
			if (!ChatLog.IsEmpty())
			{
				CXPlayerController->STCChatLog(ChatLog);
			}
		}
	}

	// 첫 플레이어 접속 시 대기 카운트다운 시작
	if (bCountdownStarted == false && GetNumPlayers() >= MinPlayersToStart)
	{
		StartGameCountdown();
	}
}

void ACXGameMode::BeginPlay()
{
	Super::BeginPlay();
	CountdownTimer = CountdownDuration;
	GameTimer = GameDuration;
}

void ACXGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ACXGameMode::StartGameCountdown()
{
	bCountdownStarted = true;
	if (ACXGameState* CXGameState = GetGameState<ACXGameState>())
	{
		CXGameState->SetGameStatus(EGameStatus::Waiting);
		CXGameState->SetRemainingTime(CountdownDuration);
	}

	// 서버 타이머: 1초마다 남은 시간 감소
	GetWorldTimerManager().SetTimer(GameTimerHandle, this, &ACXGameMode::OnGameTimerTick, 1.0f, true);
}

void ACXGameMode::StartGame()
{
	bGameStarted = true;
	if (ACXGameState* CXGameState = GetGameState<ACXGameState>())
	{
		CXGameState->SetGameStatus(EGameStatus::Playing);
		CXGameState->SetRemainingTime(GameDuration);
	}
}

void ACXGameMode::EndGame()
{
	if (ACXGameState* CXGameState = GetGameState<ACXGameState>())
	{
		CXGameState->SetGameStatus(EGameStatus::Finished);
		CXGameState->SetRemainingTime(0.0f);
		DetermineWinners();
	}
}

void ACXGameMode::DetermineWinners()
{
	ACXGameState* CXGameState = GetGameState<ACXGameState>();
	if (!CXGameState) return;

	CXGameState->ClearWinners();

	int32 MaxCoins = -1;
	TArray<ACXPlayerState*> TopPlayers;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			if (ACXPlayerState* PS = PC->GetPlayerState<ACXPlayerState>())
			{
				int32 PlayerCoins = PS->GetCoins();
				if (PlayerCoins > MaxCoins)
				{
					MaxCoins = PlayerCoins;
					TopPlayers.Empty();
					TopPlayers.Add(PS);
				}
				else if (PlayerCoins == MaxCoins)
				{
					TopPlayers.Add(PS);
				}
			}
		}
	}

	for (ACXPlayerState* WinnerPS : TopPlayers)
	{
		CXGameState->AddWinner(WinnerPS);
	}
}

AActor* ACXGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<APlayerStart*> AvailablePlayerStarts;

	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* PlayerStart = *It;

		if (IsPlayerStartAvailable(PlayerStart))
		{
			AvailablePlayerStarts.Add(PlayerStart);
		}
	}

	checkf(AvailablePlayerStarts.IsEmpty() == false, TEXT("There is no available Vehicle PlayerStart."));

	const int32 RandomIndex = FMath::RandHelper(AvailablePlayerStarts.Num());
	return Cast<AActor>(AvailablePlayerStarts[RandomIndex]);
}

bool ACXGameMode::IsPlayerStartAvailable(const APlayerStart* PlayerStart) const
{
	if (IsValid(PlayerStart) == false || IsValid(GetWorld()) == false)
	{
		return false;
	}

	const FTransform PlayerStartTransform = PlayerStart->GetActorTransform();
	const FVector CheckLocation = PlayerStartTransform.TransformPosition(FVector(0,0,80));
	const FQuat CheckRotation = PlayerStartTransform.GetRotation();
	const FCollisionShape CheckShape = FCollisionShape::MakeBox(FVector(250,120,80));
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Vehicle);

	const FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(VehiclePlayerStartCheck), false);

	const bool bVehicleExists =
		GetWorld()->OverlapAnyTestByObjectType(
			CheckLocation,
			CheckRotation,
			ObjectQueryParams,
			CheckShape,
			QueryParams
		);

	return !bVehicleExists;
}

void ACXGameMode::OnGameTimerTick()
{
	ACXGameState* CXGameState = GetGameState<ACXGameState>();
	if (!CXGameState) return;

	const float NewTime = FMath::Max(0.0f, CXGameState->GetRemainingTime() - 1.0f);
	CXGameState->SetRemainingTime(NewTime);

	if (NewTime > 0.0f) return;

	if (CXGameState->GetGameStatus() == EGameStatus::Waiting)
	{
		StartGame();   // Playing + 남은 시간 = GameDuration
	}
	else if (CXGameState->GetGameStatus() == EGameStatus::Playing)
	{
		GetWorldTimerManager().ClearTimer(GameTimerHandle);
		EndGame();     // Finished + 우승자 결정
	}
}