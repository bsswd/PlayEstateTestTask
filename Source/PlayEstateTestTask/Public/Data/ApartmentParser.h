#pragma once

#include "CoreMinimal.h"
#include "Data/ApartmentTypes.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"


class FApartmentParser
{
public:
	/**
	 * Парсит JSON-строку с конфигурацией здания.
	 *
	 * @param JsonString		JSON.
	 * @param OutConfig			Конфигурация.
	 * @param OutWarnings		Предупреждения парсинга: отсутствующие поля, некорректные значения и т.д.
	 * @param CoordinatesScale	Масштаб координат. Например, 100.f, если JSON хранит метры, а необходимы сантиметры.
	 * @return					true, если JSON удалось разобрать как объект и найти список этажей.
	 */
	static bool Parse(const FString& JsonString,
						FBuildingConfig& OutConfig,
							TArray<FString>& OutWarnings,
								float CoordinatesScale = 1.f);

private:
	// Читает вектор из поля:
	// как объект { "x": 1, "y": 2, "z": 3 }
	// как массив [1, 2, 3].
	static bool TryGetVector(
		const TSharedPtr<FJsonObject>& Object,
		const FString& FieldName,
		FVector& OutVector
	);

	// Преобразует строку статуса в enum.
	static EApartmentStatus ParseStatus(const FString& StatusString);

	// Генерирует дефолтный ID, если в JSON его нет.
	static FString MakeDefaultApartmentId(int32 FloorNumber, int32 ApartmentIndex);

	// Получает число из поля.
	// Поддерживает не только число, но и строку вида "42.5".
	static bool TryGetNumberFlexible(
		const TSharedPtr<FJsonObject>& Object,
		const FString& FieldName,
		double& OutNumber
	);

	// Получает число из JSON.
	// Используется для массива координат.
	static bool TryGetNumberFromValue(
		const FJsonValue& Value,
		double& OutNumber
	);
};