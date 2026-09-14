//  Test task for Playestate.

#include "CameraSystem/CameraPawn.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/KismetMathLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogCameraPawn, Log, All);

ACameraPawn::ACameraPawn()
{
    PrimaryActorTick.bCanEverTick = true;

    AutoPossessPlayer = EAutoReceiveInput::Player0;

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(Root);
    Camera->bUsePawnControlRotation = false;
}

void ACameraPawn::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    APlayerController* PlayerController = Cast<APlayerController>(NewController);

    if (!PlayerController)
    {
        UE_LOG(LogCameraPawn, Error, TEXT("PlayerController is NULL"));
        return;
    }

    PlayerController->SetViewTarget(this);

    FInputModeGameAndUI InputMode;
    PlayerController->SetInputMode(InputMode);
}

void ACameraPawn::BeginPlay()
{
    Super::BeginPlay();

    // Устанавливаем дефолтную точку обзора.
    if (!bHasInitialBuildingView)
    {
        BuildingCenter = FVector(0.f, 0.f, 1000.f);
        BuildingDistance = 3000.f;
        bHasInitialBuildingView = true;
    }

    CurrentView.TargetPoint = BuildingCenter;
    CurrentView.Distance = BuildingDistance;
    CurrentView.Yaw = 45.f;
    CurrentView.Pitch = -25.f;

    GenplanView = CurrentView;

    ApplyCamera();
    bHasDoneFirstView = true; 
}

void ACameraPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bTransitioning)
    {
        Transition(DeltaTime);
        return;
    }

    if (CurrentMode == ECameraMode::Genplan)
    {
        if (UpdateGenplanInput(DeltaTime)) ApplyCamera();
    }
}

void ACameraPawn::SetBuildingView(const FVector& Center, float Distance)
{
    BuildingCenter = Center;
    BuildingDistance = FMath::Clamp(Distance, MinDistance, MaxDistance);
    bHasInitialBuildingView = true;

    GenplanView.TargetPoint = BuildingCenter;
    GenplanView.Distance = BuildingDistance;

    if (!bTransitioning && CurrentMode == ECameraMode::Genplan)
    {
        if (History.Num() == 0 && !bHasDoneFirstView)
        {
            CurrentView = GenplanView;
            ApplyCamera();
            bHasDoneFirstView = true;
        }
        else
        {
            EnterGenplan();
        }
    }
}

void ACameraPawn::EnterGenplan()
{
    if((!bTransitioning && CurrentMode == ECameraMode::Genplan) ||
        (bTransitioning && TargetMode == ECameraMode::Genplan)) return;

    PushHistory(CurrentMode, CurrentView);
    StartTransition(GenplanView, ECameraMode::Genplan);
}

void ACameraPawn::EnterFloorView(const FVector& FloorCenter)
{
    FVector TargetPoint = FloorCenter;
    TargetPoint.Z += FloorViewParams.TargetZOffset;

    SetLookPoint(TargetPoint, FloorViewParams.Distance, FloorViewParams.Pitch);
}

void ACameraPawn::EnterApartmentView(const FVector& ApartmentLocation)
{
    const FVector TargetPoint = ApartmentLocation;

    // Вычисляем позицию камеры.
    const FRotator OrbitRotation(ApartmentViewPitch, ApartmentViewYaw, 0.f);
    const FVector ForwardVector = OrbitRotation.Vector();
    const FVector CameraPosition = TargetPoint - ForwardVector * ApartmentViewDistance;

    // Точная ориентация камеры на квартиру через LookAt
    const FRotator LookRotation = UKismetMathLibrary::FindLookAtRotation(CameraPosition, TargetPoint);

    FCameraView NewView;
    NewView.TargetPoint = TargetPoint;
    NewView.Distance = ApartmentViewDistance;
    NewView.Yaw = LookRotation.Yaw;
    NewView.Pitch = LookRotation.Pitch;

    if (CurrentMode != ECameraMode::Apartment)
    {
        PushHistory(CurrentMode, CurrentView);
    }

    StartTransition(NewView, ECameraMode::Apartment);    
}

void ACameraPawn::SetLookPoint(const FVector& Target, float Distance, float Pitch)
{
    if (Distance <= 0.f) Distance = 2000.f;

    const bool bAlreadySame =
        (!bTransitioning && CurrentMode == ECameraMode::Floor &&
         FVector::DistSquared(CurrentView.TargetPoint, Target) < FMath::Square(10.f)) ||
        (bTransitioning && TargetMode == ECameraMode::Floor &&
         FVector::DistSquared(TargetView.TargetPoint, Target) < FMath::Square(10.f));

    if (bAlreadySame) return;

    FCameraView NewView;
    NewView.TargetPoint = Target;
    NewView.Distance = Distance;
    NewView.Yaw = CurrentView.Yaw;
    NewView.Pitch = Pitch;

    if (CurrentMode != ECameraMode::Floor) PushHistory(CurrentMode, CurrentView);

    StartTransition(NewView, ECameraMode::Floor);
}

void ACameraPawn::StepBack()
{
    if (History.Num() == 0)
    {
        return;
    }

    const FCameraHistoryEntry PreviousEntry = History.Pop();

    StartTransition(PreviousEntry.View, PreviousEntry.Mode);
}

ECameraMode ACameraPawn::GetCameraMode() const
{
    return CurrentMode;
}

void ACameraPawn::StartTransition(const FCameraView& NewView, ECameraMode NewMode)
{
    StartView = CurrentView;
    TargetView = NewView;

    ClampView(TargetView);

    const float StartYawRad = FMath::DegreesToRadians(StartView.Yaw);
    const float TargetYawRad = FMath::DegreesToRadians(TargetView.Yaw);
    const float DeltaYawRad = FMath::FindDeltaAngleRadians(StartYawRad, TargetYawRad);

    TargetView.Yaw = StartView.Yaw + FMath::RadiansToDegrees(DeltaYawRad);

    TargetMode = NewMode;
    TransitionTime = 0.f;
    bTransitioning = true;
    
    CurrentMode = NewMode;    
}

void ACameraPawn::Transition(float DeltaTime)
{
    // Логика плавного перелета камеры.
    TransitionTime += DeltaTime;

    const float Duration = FMath::Max(TransitionDuration, 0.05f);
    const float AlphaClamp = FMath::Clamp(TransitionTime / Duration, 0.f, 1.f);
    const float Alpha = FMath::InterpEaseInOut(0.f, 1.f, AlphaClamp, 2.f);

    CurrentView.TargetPoint = FMath::Lerp(StartView.TargetPoint, TargetView.TargetPoint, Alpha);
    CurrentView.Distance = FMath::Lerp(StartView.Distance, TargetView.Distance, Alpha);
    CurrentView.Yaw = FMath::Lerp(StartView.Yaw, TargetView.Yaw, Alpha);
    CurrentView.Pitch = FMath::Lerp(StartView.Pitch, TargetView.Pitch, Alpha);

    ApplyCamera();

    if (AlphaClamp >= 1.f)
    {
        bTransitioning = false;

        if (CurrentMode == ECameraMode::Genplan)
        {
            GenplanView = CurrentView;
        }
    }
}

bool ACameraPawn::IsTransitioning() const
{
    return bTransitioning;
}

bool ACameraPawn::UpdateGenplanInput(float DeltaTime)
{
    APlayerController* PlayerController = Cast<APlayerController>(GetController());

    if (!PlayerController)
    {
        return false;
    }

    bool bChanged = false;

    if (PlayerController->IsInputKeyDown(EKeys::A))
    {
        CurrentView.Yaw -= OrbitSpeed * DeltaTime;
        bChanged = true;
    }

    if (PlayerController->IsInputKeyDown(EKeys::D))
    {
        CurrentView.Yaw += OrbitSpeed * DeltaTime;
        bChanged = true;
    }

    if (PlayerController->IsInputKeyDown(EKeys::W))
    {
        CurrentView.TargetPoint.Z += VerticalSpeed * DeltaTime;
        bChanged = true;
    }

    if (PlayerController->IsInputKeyDown(EKeys::S))
    {
        CurrentView.TargetPoint.Z -= VerticalSpeed * DeltaTime;
        bChanged = true;
    }

    if (bChanged)
    {
        ClampView(CurrentView);
        GenplanView = CurrentView;
    }

    return bChanged;
}

FVector ACameraPawn::ComputeCameraLocation(const FCameraView& View) const
{
    const FRotator Rotation(View.Pitch, View.Yaw, 0.f);
    const FVector Forward = Rotation.Vector();
    return View.TargetPoint - Forward * View.Distance;
}

void ACameraPawn::PushHistory(ECameraMode Mode, const FCameraView& View)
{
    if (MaxHistoryCount <= 0)
    {
        return;
    }

    if (History.Num() >= MaxHistoryCount)
    {
        History.RemoveAt(0);
    }

    FCameraHistoryEntry Entry;
    Entry.Mode = Mode;
    Entry.View = View;

    History.Add(Entry);
}

void ACameraPawn::ApplyCamera()
{
    if (!Camera)
    {
        UE_LOG(LogCameraPawn, Warning, TEXT("Camera is not set"));
        return;
    }

    ClampView(CurrentView);

    const FRotator CameraRotation(CurrentView.Pitch, CurrentView.Yaw, 0.f);
    const FVector ForwardVector = CameraRotation.Vector();
    const FVector CameraLocation = CurrentView.TargetPoint - ForwardVector * CurrentView.Distance;

    Camera->SetWorldLocationAndRotation(CameraLocation, CameraRotation);
}

void ACameraPawn::ClampView(FCameraView& View) const
{
    View.Distance = FMath::Clamp(View.Distance, MinDistance, MaxDistance);
    View.Pitch = FMath::Clamp(View.Pitch, MinPitch, MaxPitch);
    View.TargetPoint.Z = FMath::Clamp(View.TargetPoint.Z, MinTargetZ, MaxTargetZ);

    if (
        !FMath::IsFinite(View.TargetPoint.X) ||
        !FMath::IsFinite(View.TargetPoint.Y) ||
        !FMath::IsFinite(View.TargetPoint.Z)
    )
    {
        View.TargetPoint = BuildingCenter;
    }
}