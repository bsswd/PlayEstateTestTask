#pragma once

#include "CoreMinimal.h"
#include "ApartmentTypes.generated.h"

UENUM(BlueprintType)
enum class EApartmentStatus : uint8
{
	Free UMETA(DisplayName = "Free"),
	Sold UMETA(DisplayName = "Sold")
};

// Удобная функция для UI.
// Возвращает человекочитаемый статус на русском.
inline FString ApartmentStatusToString(EApartmentStatus Status)
{
	switch (Status)
	{
	case EApartmentStatus::Sold:
		return TEXT("Продано");

	case EApartmentStatus::Free:
	default:
		return TEXT("Свободно");
	}
}

USTRUCT(BlueprintType)
struct FApartmentData
{
	GENERATED_BODY()

	// Уникальный идентификатор квартиры.
	UPROPERTY(BlueprintReadOnly)
	FString Id;

	// Статус: свободно / продано.
	UPROPERTY(BlueprintReadOnly)
	EApartmentStatus Status = EApartmentStatus::Free;

	// Площадь в квадратных метрах.
	UPROPERTY(BlueprintReadOnly)
	float Area = 0.f;

	// Точка фокуса камеры из JSON.
	// Хранится уже в единицах Unreal, обычно сантиметры.
	UPROPERTY(BlueprintReadOnly)
	FVector FocusPoint = FVector::ZeroVector;

	// Номер этажа, на котором находится квартира.
	// Дублируем сюда для удобства поиска и отладки.
	UPROPERTY(BlueprintReadOnly)
	int32 FloorNumber = INDEX_NONE;
};

USTRUCT(BlueprintType)
struct FFloorData
{
	GENERATED_BODY()

	// Номер этажа.
	UPROPERTY(BlueprintReadOnly)
	int32 FloorNumber = INDEX_NONE;

	// Список квартир на этаже.
	UPROPERTY(BlueprintReadOnly)
	TArray<FApartmentData> Apartments;
};

USTRUCT(BlueprintType)
struct FBuildingConfig
{
	GENERATED_BODY()

	// Все этажи здания.
	UPROPERTY(BlueprintReadOnly)
	TArray<FFloorData> Floors;
};