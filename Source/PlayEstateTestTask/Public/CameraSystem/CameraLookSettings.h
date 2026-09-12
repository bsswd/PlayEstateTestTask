#pragma once

#include "CoreMinimal.h"
#include "CameraLookSettings.generated.h"

// Параметры орбитального вида камеры вокруг точки.
// Описывает ракурс: на каком расстоянии стоять, под каким углом смотреть,
// и смещение точки обзора по вертикали.
USTRUCT(BlueprintType)
struct FOrbitViewParams
{
    GENERATED_BODY()

    // Дистанция от точки обзора до камеры.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "View")
    float Distance = 1800.f;

    // Угол наклона камеры. Отрицательный = смотрим сверху вниз.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "View")
    float Pitch = -35.f;

    // Смещение точки обзора по Z относительно центра.
    // Например, 100 = камера смотрит чуть выше центра этажа.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "View")
    float TargetZOffset = 0.f;
};