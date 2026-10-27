// CXSpawner.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CXSpawner.generated.h"

class ACXItemBase;

UCLASS()
class CX_API ACXSpawner : public AActor
{
	GENERATED_BODY()

public:
	ACXSpawner();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	TSubclassOf<ACXItemBase> ItemClassToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	float RespawnDelay = 2.0f;

private:
	void SpawnItem();

	UFUNCTION()
	void HandleItemDestroyed(AActor* DestroyedActor);

	UPROPERTY()
	TObjectPtr<ACXItemBase> SpawnedItem;

	FTimerHandle RespawnTimerHandle;
};
