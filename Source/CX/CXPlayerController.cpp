// Copyright Epic Games, Inc. All Rights Reserved.

#include "CXPlayerController.h"
#include "CXPawn.h"
#include "CXUI.h"
#include "EnhancedInputSubsystems.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"
#include "EngineUtils.h"
#include "Subsystem/ChatSubsystem.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"

void ACXPlayerController::STCChatLog_Implementation(const TArray<FChatData>& ChatLog)
{
    if (UChatSubsystem* ChatSubsystem = GetWorld()->GetSubsystem<UChatSubsystem>())
    {
        ChatSubsystem->SetChatLog(ChatLog);
    }
}

void ACXPlayerController::CTSChatMessage_Implementation(const FString& Message)
{
    if (Message.IsEmpty()) return;

    const FString PlayerName = PlayerState ? PlayerState->GetPlayerName() : TEXT("Player");
    if (UChatSubsystem* ChatSubsystem = GetWorld()->GetSubsystem<UChatSubsystem>())
    {
        ChatSubsystem->AddChatMessage(PlayerName, Message);
    }

    for(TActorIterator<ACXPlayerController> It(GetWorld()); It; ++It)
    {
        if (ACXPlayerController* PC = *It)
        {
            PC->STCChatMessage(PlayerName, Message);
        }
    }
}

void ACXPlayerController::STCChatMessage_Implementation(const FString& Sender, const FString& Message)
{
    if (VehicleUI)
    {
        VehicleUI->OnNewChatMessage(Sender, Message);
    }
}

void ACXPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (IsLocalController() == false)
    {
        return;
    }

    // spawn the UI widget and add it to the viewport
    VehicleUI = CreateWidget<UCXUI>(this, VehicleUIClass);
    check(VehicleUI);
    VehicleUI->AddToViewport();

    if (UChatSubsystem* ChatSubsystem = GetWorld()->GetSubsystem<UChatSubsystem>())
    {
        const TArray<FChatData>& ChatLog = ChatSubsystem->GetChatLog();
        for (const FChatData& ChatData : ChatLog)
        {
            VehicleUI->OnNewChatMessage(ChatData.Sender, ChatData.Message);
        }
        ChatSubsystem->ClearChatLog();
    }
}

void ACXPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // get the enhanced input subsystem
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        // add the mapping context so we get controls
        Subsystem->AddMappingContext(InputMappingContext, 0);

        // optionally add the steering wheel context
        if (bUseSteeringWheelControls && SteeringWheelInputMappingContext)
        {
            Subsystem->AddMappingContext(SteeringWheelInputMappingContext, 1);
        }
    }

    // Enter → 채팅 입력 시작
    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
    {
        if (ChatAction)
        {
            EnhancedInput->BindAction(ChatAction, ETriggerEvent::Started, this, &ACXPlayerController::EnterChatMode);
        }
    }
}

void ACXPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ThisClass, VehiclePawn);
}

void ACXPlayerController::Tick(float Delta)
{
    Super::Tick(Delta);

    if (IsValid(VehiclePawn) && IsValid(VehicleUI))
    {
        VehicleUI->UpdateSpeed(VehiclePawn->GetChaosVehicleMovement()->GetForwardSpeed());
        VehicleUI->UpdateGear(VehiclePawn->GetChaosVehicleMovement()->GetCurrentGear());
    }
}

void ACXPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // get a pointer to the controlled pawn
    VehiclePawn = CastChecked<ACXPawn>(InPawn);
}

void ACXPlayerController::EnterChatMode()
{
    if (bIsChatting || IsValid(VehicleUI) == false)
    {
        return;
    }
    bIsChatting = true;

    // 누르고 있던 W 등을 떼서 입력 중에 차가 혼자 달리지 않게
    FlushPressedKeys();

    FInputModeUIOnly InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    SetInputMode(InputMode);
    SetShowMouseCursor(true);

    VehicleUI->FocusChatInput();
}

void ACXPlayerController::ExitChatMode()
{
    if (bIsChatting == false)
    {
        return;
    }
    bIsChatting = false;

    SetInputMode(FInputModeGameOnly());
    SetShowMouseCursor(false);
}