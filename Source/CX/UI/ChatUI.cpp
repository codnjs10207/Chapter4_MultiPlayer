// UI/ChatUI.cpp

#include "ChatUI.h"
#include "../CXPlayerController.h"
#include "Components/EditableTextBox.h"
#include "Components/MultiLineEditableTextBox.h"

void UChatUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (EditableTextBox_ChatInput->OnTextCommitted.IsAlreadyBound(this, &UChatUI::OnChatInputTextCommitted) == false)
	{
		EditableTextBox_ChatInput->OnTextCommitted.AddDynamic(this, &UChatUI::OnChatInputTextCommitted);
	}
}

void UChatUI::NativeDestruct()
{
	if (EditableTextBox_ChatInput->OnTextCommitted.IsAlreadyBound(this, &UChatUI::OnChatInputTextCommitted) == true)
	{
		EditableTextBox_ChatInput->OnTextCommitted.RemoveDynamic(this, &UChatUI::OnChatInputTextCommitted);
	}

	Super::NativeDestruct();
}

void UChatUI::OnNewChatMessage(const FString& InSender, const FString& InMessage)
{
	FString NewMessage = InSender + TEXT(": ") + InMessage;

	FString OldMessage = MultilineEditableTextBox_ChatLog->GetText().ToString();
	OldMessage += '\n';
	FText Message = FText::FromString(OldMessage + NewMessage);

	MultilineEditableTextBox_ChatLog->SetText(Message);
}

void UChatUI::OnChatInputTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	ACXPlayerController* PC = GetOwningPlayer<ACXPlayerController>();

	if (CommitMethod == ETextCommit::OnEnter)
	{
		const FString ChatMessage = Text.ToString().TrimStartAndEnd();

		// 빈 메시지는 보내지 않음 (유효성 검사)
		if (ChatMessage.IsEmpty() == false && IsValid(PC))
		{
			PC->CTSChatMessage(ChatMessage);
		}

		EditableTextBox_ChatInput->SetText(FText::GetEmpty());
	}

	// Enter든, Esc / 다른 곳 클릭이든 입력이 끝나면 게임 조작으로 복귀
	if (IsValid(PC))
	{
		PC->ExitChatMode();
	}
}

void UChatUI::FocusChatInput()
{
	if (IsValid(EditableTextBox_ChatInput))
	{
		EditableTextBox_ChatInput->SetKeyboardFocus();
	}
}
