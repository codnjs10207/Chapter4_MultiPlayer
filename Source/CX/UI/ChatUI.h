// UI/ChatUI.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatUI.generated.h"

class UEditableTextBox;
class UScrollBox;
class UMultiLineEditableTextBox;

UCLASS()
class CX_API UChatUI : public UUserWidget
{
	GENERATED_BODY()

public:
	void OnNewChatMessage(const FString& InSender, const FString& InMessage);
	void FocusChatInput();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void OnChatInputTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> EditableTextBox_ChatInput;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ChatScrollBox;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UMultiLineEditableTextBox> MultilineEditableTextBox_ChatLog;
};