// Copyright Epic Games, Inc. All Rights Reserved.

#include "CXPawn.h"
#include "CXWheelFront.h"
#include "CXWheelRear.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "CXGameState.h"
#include "CXPlayerState.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/DamageType.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystem.h"

#define LOCTEXT_NAMESPACE "VehiclePawn"

DEFINE_LOG_CATEGORY(LogTemplateVehicle);

ACXPawn::ACXPawn()
{
	// construct the front camera boom
	FrontSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Front Spring Arm"));
	FrontSpringArm->SetupAttachment(GetMesh());
	FrontSpringArm->TargetArmLength = 0.0f;
	FrontSpringArm->bDoCollisionTest = false;
	FrontSpringArm->bEnableCameraRotationLag = true;
	FrontSpringArm->CameraRotationLagSpeed = 15.0f;
	FrontSpringArm->SetRelativeLocation(FVector(30.0f, 0.0f, 120.0f));

	FrontCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Front Camera"));
	FrontCamera->SetupAttachment(FrontSpringArm);
	FrontCamera->bAutoActivate = false;

	// construct the back camera boom
	BackSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Back Spring Arm"));
	BackSpringArm->SetupAttachment(GetMesh());
	BackSpringArm->TargetArmLength = 650.0f;
	BackSpringArm->SocketOffset.Z = 150.0f;
	BackSpringArm->bDoCollisionTest = false;
	BackSpringArm->bInheritPitch = false;
	BackSpringArm->bInheritRoll = false;
	BackSpringArm->bEnableCameraRotationLag = true;
	BackSpringArm->CameraRotationLagSpeed = 2.0f;
	BackSpringArm->CameraLagMaxDistance = 50.0f;

	BackCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Back Camera"));
	BackCamera->SetupAttachment(BackSpringArm);

	// Configure the car mesh
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(FName("Vehicle"));

	// get the Chaos Wheeled movement component
	ChaosVehicleMovement = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());

    bReplicates = true;
}

void ACXPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACXPawn, bThrottleMalfunction);
}

void ACXPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &ACXPawn::Steering);
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Completed, this, &ACXPawn::Steering);

		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &ACXPawn::Throttle);
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &ACXPawn::Throttle);

		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &ACXPawn::Brake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Started, this, &ACXPawn::StartBrake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Completed, this, &ACXPawn::StopBrake);

		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &ACXPawn::StartHandbrake);
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &ACXPawn::StopHandbrake);

		EnhancedInputComponent->BindAction(LookAroundAction, ETriggerEvent::Triggered, this, &ACXPawn::LookAround);
		EnhancedInputComponent->BindAction(ToggleCameraAction, ETriggerEvent::Triggered, this, &ACXPawn::ToggleCamera);
		EnhancedInputComponent->BindAction(ResetVehicleAction, ETriggerEvent::Triggered, this, &ACXPawn::ResetVehicle);

        EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ACXPawn::OnAttackInput);
        EnhancedInputComponent->BindAction(MineAction, ETriggerEvent::Started, this, &ACXPawn::OnMineInput);
	}
}

void ACXPawn::Tick(float Delta)
{
	Super::Tick(Delta);

	bool bMovingOnGround = ChaosVehicleMovement->IsMovingOnGround();
	GetMesh()->SetAngularDamping(bMovingOnGround ? 0.0f : 3.0f);

	float CameraYaw = BackSpringArm->GetRelativeRotation().Yaw;
	CameraYaw = FMath::FInterpTo(CameraYaw, 0.0f, Delta, 1.0f);
	BackSpringArm->SetRelativeRotation(FRotator(0.0f, CameraYaw, 0.0f));

    // 게임 상태에 따른 제약 사항
    if (!IsInPlayingState())
    {
        ChaosVehicleMovement->SetThrottleInput(0.0f);
        ChaosVehicleMovement->SetSteeringInput(0.0f);
        ChaosVehicleMovement->SetHandbrakeInput(true);
        bHandbrakeLockedByGameState = true;
    }
    else if (bHandbrakeLockedByGameState)
    {
        // Playing으로 바뀌면 게임 상태용 핸드브레이크 잠금 자동 해제
        ChaosVehicleMovement->SetHandbrakeInput(false);
        bHandbrakeLockedByGameState = false;
    }

    // 내가 조종하는 차만: 최대 속도 제한 + 가속 페달 고장 적용
    if (IsLocallyControlled() && IsInPlayingState())
    {
        float ThrottleValue = bThrottleMalfunction ? 0.0f : LastThrottleInput;

        const float SpeedKmh = ChaosVehicleMovement->GetForwardSpeed() * 0.036f;   // cm/s → km/h
        if (ThrottleValue > 0.0f && SpeedKmh >= GetMaxSpeedKmh())
        {
            ThrottleValue = 0.0f;
        }
        ChaosVehicleMovement->SetThrottleInput(ThrottleValue);
    }
}

bool ACXPawn::IsInPlayingState() const
{
    if (ACXGameState* GS = GetWorld()->GetGameState<ACXGameState>())
    {
        return GS->GetGameStatus() == EGameStatus::Playing;
    }
    return false;
}

void ACXPawn::Steering(const FInputActionValue& Value)
{
    if (!IsInPlayingState()) return;
	ChaosVehicleMovement->SetSteeringInput(Value.Get<float>());
}

void ACXPawn::Throttle(const FInputActionValue& Value)
{
    LastThrottleInput = Value.Get<float>();   // 원래 입력값 기억

    if (!IsInPlayingState() || bThrottleMalfunction)
    {
        ChaosVehicleMovement->SetThrottleInput(0.0f);
        return;
    }
    ChaosVehicleMovement->SetThrottleInput(LastThrottleInput);
}

float ACXPawn::GetMaxSpeedKmh() const
{
    float Bonus = 0.0f;
    if (const ACXPlayerState* PS = GetPlayerState<ACXPlayerState>())
    {
        Bonus = PS->GetEffectiveBonusMaxSpeed();
    }
    return FMath::Max(20.0f, BaseMaxSpeedKmh + Bonus);
}

void ACXPawn::Brake(const FInputActionValue& Value)
{
    if (!IsInPlayingState()) return;
	ChaosVehicleMovement->SetBrakeInput(Value.Get<float>());
}

void ACXPawn::StartBrake(const FInputActionValue& Value) { BrakeLights(true); }
void ACXPawn::StopBrake(const FInputActionValue& Value) { BrakeLights(false); ChaosVehicleMovement->SetBrakeInput(0.0f); }

void ACXPawn::StartHandbrake(const FInputActionValue& Value)
{
    if (!IsInPlayingState()) return;
	ChaosVehicleMovement->SetHandbrakeInput(true);
	BrakeLights(true);
}

void ACXPawn::StopHandbrake(const FInputActionValue& Value)
{
    if (!IsInPlayingState()) return;
	ChaosVehicleMovement->SetHandbrakeInput(false);
	BrakeLights(false);
}

void ACXPawn::LookAround(const FInputActionValue& Value) { BackSpringArm->AddLocalRotation(FRotator(0.0f, Value.Get<float>(), 0.0f)); }
void ACXPawn::ToggleCamera(const FInputActionValue& Value) { bFrontCameraActive = !bFrontCameraActive; FrontCamera->SetActive(bFrontCameraActive); BackCamera->SetActive(!bFrontCameraActive); }

void ACXPawn::ResetVehicle(const FInputActionValue& Value)
{
    if (!IsInPlayingState()) return;
	FVector ResetLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
	FRotator ResetRotation = GetActorRotation();
	ResetRotation.Pitch = 0.0f; ResetRotation.Roll = 0.0f;
	SetActorTransform(FTransform(ResetRotation, ResetLocation, FVector::OneVector), false, nullptr, ETeleportType::TeleportPhysics);
	GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);
}

void ACXPawn::OnAttackInput(const FInputActionValue& Value) { if (IsInPlayingState()) ServerRPC_Attack(); }
void ACXPawn::OnMineInput(const FInputActionValue& Value) { if (IsInPlayingState()) ServerRPC_PlaceMine(); }

bool ACXPawn::ServerRPC_Attack_Validate() { return true; }

void ACXPawn::ServerRPC_Attack_Implementation()
{
    if (!IsInPlayingState()) return;   // 서버에서 한 번 더 검사

    const FVector Start = GetActorLocation() + GetActorForwardVector() * 100.0f + FVector(0, 0, 50);
    const FVector End = Start + GetActorForwardVector() * AttackRange;

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHitVehicle = false;
    FVector EndPoint = End;

    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
    {
        EndPoint = HitResult.ImpactPoint;

        if (ACXPawn* Victim = Cast<ACXPawn>(HitResult.GetActor()))
        {
            bHitVehicle = true;

            // 데미지 0이면 ApplyPointDamage가 아무것도 안 하므로 1을 줌
            UGameplayStatics::ApplyPointDamage(Victim, 1.0f, GetActorForwardVector(), HitResult,
                GetController(), this, UDamageType::StaticClass());

            OnAttackHit.Broadcast(Victim);
        }
    }

    // 모든 클라이언트 화면에 선 + 파티클 표시
    MulticastRPC_PlayAttackFX(Start, EndPoint, bHitVehicle);
}

void ACXPawn::MulticastRPC_PlayAttackFX_Implementation(FVector Start, FVector End, bool bHitVehicle)
{
    if (GetNetMode() == NM_DedicatedServer) return;   // 화면 없는 서버는 생략

    DrawDebugLine(GetWorld(), Start, End, bHitVehicle ? FColor::Red : FColor::Green, false, 1.0f, 0, 3.0f);

    if (bHitVehicle && HitParticle)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle, End);
    }
}

// ApplyPointDamage를 받으면 피해 차량 쪽에서 실행됨 (서버)
float ACXPawn::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    const float Result = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (!HasAuthority()) return Result;

    ACXPawn* Attacker = Cast<ACXPawn>(DamageCauser);
    if (!IsValid(Attacker) || Attacker == this) return Result;

    ACXPlayerState* VictimPS = GetPlayerState<ACXPlayerState>();
    ACXPlayerState* AttackerPS = Attacker->GetPlayerState<ACXPlayerState>();
    if (!VictimPS || !AttackerPS) return Result;

    // 5원 미만이면 RemoveCoins가 false → 아무 일도 안 일어남
    if (VictimPS->RemoveCoins(5))
    {
        AttackerPS->AddCoins(5);
    }

    // 바퀴 명중 → 추가 최대 속도 음수 패널티
    if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
    {
        const FPointDamageEvent& PointEvent = static_cast<const FPointDamageEvent&>(DamageEvent);
        if (IsWheelHit(PointEvent.HitInfo))
        {
            VictimPS->ApplyWheelPenalty(WheelHitPenalty);
        }
    }

    return Result;
}

bool ACXPawn::IsWheelHit(const FHitResult& Hit) const
{
    // 1) 맞은 Bone 이름에 Wheel이 들어있으면 바퀴
    if (Hit.BoneName.ToString().Contains(TEXT("Wheel")))
    {
        return true;
    }

    // 2) 맞은 지점이 바퀴 Bone(Phys_Wheel_FL 등) 근처면 바퀴
    for (const FChaosWheelSetup& WheelSetup : ChaosVehicleMovement->WheelSetups)
    {
        if (WheelSetup.BoneName.IsNone()) continue;

        const FVector WheelLocation = GetMesh()->GetBoneLocation(WheelSetup.BoneName);
        if (FVector::Dist(WheelLocation, FVector(Hit.ImpactPoint)) <= WheelHitRadius)
        {
            return true;
        }
    }
    return false;
}

bool ACXPawn::ServerRPC_PlaceMine_Validate() { return true; }
void ACXPawn::ServerRPC_PlaceMine_Implementation()
{
    if (!IsInPlayingState()) return;   // 서버에서 한 번 더 검사
    if (MineClass)
    {
        FVector SpawnLoc = GetActorLocation() - GetActorForwardVector() * 250.0f + FVector(0,0,20);
        FActorSpawnParameters Params;
        Params.Owner = this;
        GetWorld()->SpawnActor<AActor>(MineClass, SpawnLoc, GetActorRotation(), Params);
    }
}

void ACXPawn::ApplyThrottleMalfunction(float Duration)
{
    if (!HasAuthority()) return;
    bThrottleMalfunction = true;
    GetWorldTimerManager().SetTimer(MalfunctionTimerHandle, this, &ACXPawn::ClearThrottleMalfunction, Duration, false);
}

void ACXPawn::ClearThrottleMalfunction()
{
    bThrottleMalfunction = false;
}

#undef LOCTEXT_NAMESPACE
