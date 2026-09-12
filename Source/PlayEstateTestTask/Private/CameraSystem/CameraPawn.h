//  Test task for Playestate.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "CameraPawn.generated.h"

UENUM(BlueprintType)
enum class ECameraMode : uint8
{
    Genplan,
    Floor,
    Apartment
};

USTRUCT(BlueprintType)
struct FCameraView
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FVector TargetPoint = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    float Distance = 3000.f;

    UPROPERTY(BlueprintReadOnly)
    float Yaw = 45.f;

    UPROPERTY(BlueprintReadOnly)
    float Pitch = -25.f;
};

USTRUCT()
struct FCameraHistoryEntry
{
    GENERATED_BODY()

    UPROPERTY()
    ECameraMode Mode = ECameraMode::Genplan;

    UPROPERTY()
    FCameraView View;
};

/*
*   Класс предназначен для управления камерой.
*   
*   В режиме Генплана можно совершать облет здания и перемещаться вверх и вниз. Здесь же находится и логика управления.
*   Свободная камера не используется,
*   так как приложением могут пользоваться люди незнакомые с классическим WASD + Мышь (как в играх)
*   в UI будет подробная подсказка какие клавиши нажимать для конкретных действий.
*   
*   Переходы в режим этажа и режим квартиры совершаются автоматически. В этих режимах управление камерой клавишами заблокировано.  
*/

UCLASS()
class ACameraPawn : public APawn
{
    GENERATED_BODY()

public:
    ACameraPawn();

    virtual void Tick(float DeltaTime) override;
    virtual void PossessedBy(AController* NewController) override;

    // Задать центр здания и дистанцию для Genplan.
    UFUNCTION(BlueprintCallable)
    void SetBuildingView(const FVector& Center, float Distance);

    // Перейти к генплану.
    UFUNCTION(BlueprintCallable)
    void EnterGenplan();

    // Перейти к этажу.
    UFUNCTION(BlueprintCallable)
    void EnterFloor(const FVector& Target, float Distance, float Pitch);

    // Перейти к квартире.
    UFUNCTION(BlueprintCallable)
    void EnterApartment(const FVector& FocusPoint, float Distance, float Pitch);

    // Переход на шаг назад.
    UFUNCTION(BlueprintCallable)
    void GoBack();

    // Текущий режим камеры.
    UFUNCTION(BlueprintCallable)
    ECameraMode GetCameraMode() const;

    // Идет ли сейчас перелет.
    UFUNCTION(BlueprintCallable)
    bool IsTransitioning() const;

protected:
    virtual void BeginPlay() override;
    void Transition(float DeltaTime);

    void ApplyCamera();
    void ClampView(FCameraView& View) const;
    void StartTransition(const FCameraView& NewView, ECameraMode NewMode);
    void PushHistory(ECameraMode Mode, const FCameraView& View);
    bool UpdateGenplanInput(float DeltaTime);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USceneComponent> Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UCameraComponent> Camera;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float TransitionDuration = 0.8f;

    // Скорость вращения в Genplan, градусы/сек.
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float OrbitSpeed = 90.f;

    // Скорость вертикального перемещения в Genplan, юниты/сек.
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float VerticalSpeed = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float MinPitch = -80.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float MaxPitch = -5.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float MinDistance = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float MaxDistance = 15000.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float MinTargetZ = -2000.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float MaxTargetZ = 15000.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 MaxHistoryCount = 8;

private:
    ECameraMode CurrentMode = ECameraMode::Genplan;
    ECameraMode TargetMode = ECameraMode::Genplan;

    FCameraView CurrentView;
    FCameraView StartView;
    FCameraView TargetView;
    FCameraView GenplanView;

    TArray<FCameraHistoryEntry> History;

    FVector BuildingCenter = FVector::ZeroVector;
    float BuildingDistance = 3000.f;

    float TransitionTime = 0.f;

    bool bTransitioning = false;
    bool bHasInitialBuildingView = false;
};
