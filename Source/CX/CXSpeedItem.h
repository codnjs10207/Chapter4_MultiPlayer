// CXSpeedItem.h

#pragma once

#include "CoreMinimal.h"
#include "CXItemBase.h"
#include "CXSpeedItem.generated.h"

UCLASS()
class CX_API ACXSpeedItem : public ACXItemBase
{
	GENERATED_BODY()

protected:
	virtual void OnPickedUp(class ACXPawn* Character) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Speed")
	float BonusSpeed = 10.0f;
};
