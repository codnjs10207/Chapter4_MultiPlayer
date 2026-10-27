// Copyright Epic Games, Inc. All Rights Reserved.

#include "CXUI.h"
#include "UI/ChatUI.h"

void UCXUI::OnNewChatMessage(const FString& InSender, const FString& InMessage)
{
    // UI_Chat이 유효한지만 검사하고 바로 함수 호출
    if (UI_Chat)
    {
        UI_Chat->OnNewChatMessage(InSender, InMessage);
    }
}

void UCXUI::UpdateSpeed(float NewSpeed)
{
    float FormattedSpeed = FMath::Abs(NewSpeed) * (bIsMPH ? 0.022f : 0.036f);
    OnSpeedUpdate(FormattedSpeed);
}

void UCXUI::UpdateGear(int32 NewGear)
{
    OnGearUpdate(NewGear);
}

void UCXUI::FocusChatInput()
{
    if (UI_Chat)
    {
        UI_Chat->FocusChatInput();
    }
}
