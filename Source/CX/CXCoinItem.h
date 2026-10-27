// CXCoinItem.h

#pragma once

#include "CoreMinimal.h"
#include "CXItemBase.h"
#include "CXCoinItem.generated.h"

UCLASS()
class CX_API ACXCoinItem : public ACXItemBase
{
	GENERATED_BODY()

protected:
	virtual void OnPickedUp(class ACXPawn* Character) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Coin")
	int32 CoinAmount = 10;
};
