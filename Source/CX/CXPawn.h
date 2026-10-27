// CXPawn.h

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "CXPawn.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class UChaosWheeledVehicleMovementComponent;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateVehicle, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttackHitDelegate, AActor*, HitActor);

UCLASS(abstract)
class CX_API ACXPawn : public AWheeledVehiclePawn
{
	GENERATED_BODY()

	/** Spring Arm for the front camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* FrontSpringArm;

	/** Front Camera component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FrontCamera;

	/** Spring Arm for the back camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* BackSpringArm;

	/** Back Camera component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* BackCamera;

	/** Cast pointer to the Chaos Vehicle movement component */
	TObjectPtr<UChaosWheeledVehicleMovementComponent> ChaosVehicleMovement;

protected:

	/** Steering Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* SteeringAction;

	/** Throttle Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* ThrottleAction;

	/** Brake Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* BrakeAction;

	/** Handbrake Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* HandbrakeAction;

	/** Look Around Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* LookAroundAction;

	/** Toggle Camera Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* ToggleCameraAction;

	/** Reset Vehicle Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* ResetVehicleAction;

    /** Attack Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* AttackAction;

    /** Mine Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* MineAction;

	/** Keeps track of which camera is active */
	bool bFrontCameraActive = false;

	bool bHandbrakeLockedByGameState = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Speed")
	float BaseMaxSpeedKmh = 100.0f;

	float LastThrottleInput = 0.0f;

	float GetMaxSpeedKmh() const;

public:
	ACXPawn();

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	virtual void Tick(float Delta) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	/** Apply throttle malfunction for a duration */
	void ApplyThrottleMalfunction(float Duration);

protected:
	void Steering(const FInputActionValue& Value);
	void Throttle(const FInputActionValue& Value);
	void Brake(const FInputActionValue& Value);
	void StartBrake(const FInputActionValue& Value);
	void StopBrake(const FInputActionValue& Value);
	void StartHandbrake(const FInputActionValue& Value);
	void StopHandbrake(const FInputActionValue& Value);
	void LookAround(const FInputActionValue& Value);
	void ToggleCamera(const FInputActionValue& Value);
	void ResetVehicle(const FInputActionValue& Value);

    /** Attack Input */
    void OnAttackInput(const FInputActionValue& Value);
    /** Mine Input */
    void OnMineInput(const FInputActionValue& Value);

	UFUNCTION(BlueprintImplementableEvent, Category="Vehicle")
	void BrakeLights(bool bBraking);

    // --- New Features ---

    /** Server RPC for Attack */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerRPC_Attack();

    /** Server RPC for Mine */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerRPC_PlaceMine();

    /** Clear throttle malfunction */
    void ClearThrottleMalfunction();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
    float AttackRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mine")
    TSubclassOf<AActor> MineClass;

    UPROPERTY(Replicated)
    bool bThrottleMalfunction = false;

    FTimerHandle MalfunctionTimerHandle;

    UPROPERTY(BlueprintAssignable, Category = "Attack")
    FOnAttackHitDelegate OnAttackHit;

    /** Helper to check if game is in Playing state */
    bool IsInPlayingState() const;

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPC_PlayAttackFX(FVector Start, FVector End, bool bHitVehicle);

	bool IsWheelHit(const FHitResult& Hit) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	TObjectPtr<class UParticleSystem> HitParticle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float WheelHitRadius = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float WheelHitPenalty = 30.0f;

public:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	FORCEINLINE USpringArmComponent* GetFrontSpringArm() const { return FrontSpringArm; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FrontCamera; }
	FORCEINLINE USpringArmComponent* GetBackSpringArm() const { return BackSpringArm; }
	FORCEINLINE UCameraComponent* GetBackCamera() const { return BackCamera; }
	FORCEINLINE const TObjectPtr<UChaosWheeledVehicleMovementComponent>& GetChaosVehicleMovement() const { return ChaosVehicleMovement; }
};
