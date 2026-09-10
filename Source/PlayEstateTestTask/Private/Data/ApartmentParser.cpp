#include "Data/ApartmentParser.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Internationalization/Internationalization.h"


bool FApartmentParser::Parse(const FString& JsonString,
                                FBuildingConfig& OutConfig,
                                    TArray<FString>& OutWarnings,
                                        float CoordinatesScale)
{
    OutConfig = FBuildingConfig();
    OutWarnings.Reset();

    if (JsonString.IsEmpty())
    {
        OutWarnings.Add(TEXT("JSON string is empty."));
        return false;
    }

    const float SafeCoordinatesScale = FMath::Max(CoordinatesScale, 0.0001f);

    TSharedPtr<FJsonObject> RootObject;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
    {
        OutWarnings.Add(TEXT("Cannot deserialize root JSON object. File is empty or malformed."));
        return false;
    }

    const TArray<TSharedPtr<FJsonValue>>* FloorsArray = nullptr;

    if (!RootObject->TryGetArrayField(TEXT("floors"), FloorsArray))
    {
        const TSharedPtr<FJsonObject>* BuildingObject = nullptr;

        if (RootObject->TryGetObjectField(TEXT("building"), BuildingObject)
            && BuildingObject != nullptr
            && (*BuildingObject).IsValid())
        {
            if (!(*BuildingObject)->TryGetArrayField(TEXT("floors"), FloorsArray))
            {
                OutWarnings.Add(TEXT("Root object has 'building', but 'building.floors' array was not found."));
                return false;
            }
        }
        else
        {
            OutWarnings.Add(TEXT("Cannot find 'floors' array in root JSON object."));
            return false;
        }
    }

    if (FloorsArray == nullptr)
    {
        OutWarnings.Add(TEXT("Floors array is null."));
        return false;
    }

    TSet<FString> UsedApartmentIds;

    for (int32 FloorIndex = 0; FloorIndex < FloorsArray->Num(); ++FloorIndex)
    {
        const TSharedPtr<FJsonValue>& FloorValue = (*FloorsArray)[FloorIndex];

        if (!FloorValue.IsValid())
        {
            OutWarnings.Add(FString::Printf(TEXT("Floor value at index %d is null."), FloorIndex));
            continue;
        }

        const TSharedPtr<FJsonObject>* FloorObjectPtr = nullptr;

        if (!FloorValue->TryGetObject(FloorObjectPtr)
            || FloorObjectPtr == nullptr
            || !FloorObjectPtr->IsValid())
        {
            OutWarnings.Add(FString::Printf(TEXT("Floor at index %d is not a JSON object."), FloorIndex));
            continue;
        }

        const TSharedPtr<FJsonObject>& FloorObject = *FloorObjectPtr;

        FFloorData FloorData;

        // Если номер этажа не задан, используем индекс + 1.
        FloorData.FloorNumber = FloorIndex + 1;

        double FloorNumberDouble = 0.0;

        if (TryGetNumberFlexible(FloorObject, TEXT("floor"), FloorNumberDouble)
            || TryGetNumberFlexible(FloorObject, TEXT("floor_number"), FloorNumberDouble)
            || TryGetNumberFlexible(FloorObject, TEXT("number"), FloorNumberDouble))
        {
            FloorData.FloorNumber = static_cast<int32>(FloorNumberDouble);
        }
        else
        {
            OutWarnings.Add(FString::Printf(
                TEXT("Floor at index %d has no valid 'floor'/'floor_number'/'number' field. Default number %d is used."),
                FloorIndex,
                FloorData.FloorNumber
            ));
        }

        const TArray<TSharedPtr<FJsonValue>>* ApartmentsArray = nullptr;

        if (!FloorObject->TryGetArrayField(TEXT("apartments"), ApartmentsArray))
        {
            OutWarnings.Add(FString::Printf(
                TEXT("Floor %d has no 'apartments' array."),
                FloorData.FloorNumber
            ));

            // Этаж можно оставить пустым, чтобы панель этажей все равно могла его показать.
            OutConfig.Floors.Add(FloorData);
            continue;
        }

        if (ApartmentsArray == nullptr)
        {
            OutWarnings.Add(FString::Printf(
                TEXT("Floor %d apartments array is null."),
                FloorData.FloorNumber
            ));

            OutConfig.Floors.Add(FloorData);
            continue;
        }

        for (int32 ApartmentIndex = 0; ApartmentIndex < ApartmentsArray->Num(); ++ApartmentIndex)
        {
            const TSharedPtr<FJsonValue>& ApartmentValue = (*ApartmentsArray)[ApartmentIndex];

            if (!ApartmentValue.IsValid())
            {
                OutWarnings.Add(FString::Printf(
                    TEXT("Apartment value at floor %d index %d is null."),
                    FloorData.FloorNumber,
                    ApartmentIndex
                ));
                continue;
            }

            const TSharedPtr<FJsonObject>* ApartmentObjectPtr = nullptr;

            if (!ApartmentValue->TryGetObject(ApartmentObjectPtr)
                || ApartmentObjectPtr == nullptr
                || !ApartmentObjectPtr->IsValid())
            {
                OutWarnings.Add(FString::Printf(
                    TEXT("Apartment at floor %d index %d is not a JSON object."),
                    FloorData.FloorNumber,
                    ApartmentIndex
                ));
                continue;
            }

            const TSharedPtr<FJsonObject>& ApartmentObject = *ApartmentObjectPtr;

            FApartmentData ApartmentData;
            ApartmentData.FloorNumber = FloorData.FloorNumber;

            // -------------------------------------------------
            // ID
            // -------------------------------------------------

            FString ApartmentId;

            const bool bHasId =
                ApartmentObject->TryGetStringField(TEXT("id"), ApartmentId)
                || ApartmentObject->TryGetStringField(TEXT("Id"), ApartmentId)
                || ApartmentObject->TryGetStringField(TEXT("ID"), ApartmentId);

            if (!bHasId)
            {
                ApartmentId = MakeDefaultApartmentId(FloorData.FloorNumber, ApartmentIndex);

                OutWarnings.Add(FString::Printf(
                    TEXT("Apartment at floor %d index %d has no 'id' field. Generated id: %s"),
                    FloorData.FloorNumber,
                    ApartmentIndex,
                    *ApartmentId
                ));
            }

            ApartmentId = ApartmentId.TrimStartAndEnd();

            if (ApartmentId.IsEmpty())
            {
                ApartmentId = MakeDefaultApartmentId(FloorData.FloorNumber, ApartmentIndex);

                OutWarnings.Add(FString::Printf(
                    TEXT("Apartment at floor %d index %d has empty 'id'. Generated id: %s"),
                    FloorData.FloorNumber,
                    ApartmentIndex,
                    *ApartmentId
                ));
            }

            // Защита от дубликатов.
            FString UniqueApartmentId = ApartmentId;
            int32 Suffix = 1;

            while (UsedApartmentIds.Contains(UniqueApartmentId))
            {
                UniqueApartmentId = FString::Printf(TEXT("%s_%d"), *ApartmentId, ++Suffix);
            }

            if (UniqueApartmentId != ApartmentId)
            {
                OutWarnings.Add(FString::Printf(
                    TEXT("Duplicate apartment id '%s' was renamed to '%s'."),
                    *ApartmentId,
                    *UniqueApartmentId
                ));
            }

            UsedApartmentIds.Add(UniqueApartmentId);
            ApartmentData.Id = UniqueApartmentId;

            // -------------------------------------------------
            // Status
            // -------------------------------------------------

            FString StatusString;

            const bool bHasStatus =
                ApartmentObject->TryGetStringField(TEXT("status"), StatusString)
                || ApartmentObject->TryGetStringField(TEXT("Status"), StatusString);

            if (bHasStatus)
            {
                ApartmentData.Status = ParseStatus(StatusString);
            }
            else
            {
                ApartmentData.Status = EApartmentStatus::Free;

                OutWarnings.Add(FString::Printf(
                    TEXT("Apartment '%s' has no 'status' field. Default status 'Free' is used."),
                    *ApartmentData.Id
                ));
            }

            // -------------------------------------------------
            // Area
            // -------------------------------------------------

            double AreaDouble = 0.0;

            const bool bHasArea =
                TryGetNumberFlexible(ApartmentObject, TEXT("area"), AreaDouble)
                || TryGetNumberFlexible(ApartmentObject, TEXT("Area"), AreaDouble)
                || TryGetNumberFlexible(ApartmentObject, TEXT("area_m2"), AreaDouble);

            if (bHasArea)
            {
                ApartmentData.Area = FMath::Max(0.f, static_cast<float>(AreaDouble));
            }
            else
            {
                ApartmentData.Area = 0.f;

                OutWarnings.Add(FString::Printf(
                    TEXT("Apartment '%s' has no valid 'area' field. Default area 0 is used."),
                    *ApartmentData.Id
                ));
            }

            // -------------------------------------------------
            // Focus point
            // -------------------------------------------------

            FVector FocusPoint = FVector::ZeroVector;

            const bool bHasFocus =
                TryGetVector(ApartmentObject, TEXT("focus"), FocusPoint)
                || TryGetVector(ApartmentObject, TEXT("focus_point"), FocusPoint)
                || TryGetVector(ApartmentObject, TEXT("camera_focus"), FocusPoint);

            if (bHasFocus)
            {
                ApartmentData.FocusPoint = FocusPoint * SafeCoordinatesScale;
            }
            else
            {
                ApartmentData.FocusPoint = FVector::ZeroVector;

                OutWarnings.Add(FString::Printf(
                    TEXT("Apartment '%s' has no valid focus point. Default FocusPoint = (0,0,0)."),
                    *ApartmentData.Id
                ));
            }

            FloorData.Apartments.Add(ApartmentData);
        }

        OutConfig.Floors.Add(FloorData);
    }

    return true;
}

bool FApartmentParser::TryGetVector(
    const TSharedPtr<FJsonObject>& Object,
    const FString& FieldName,
    FVector& OutVector
)
{
    if (!Object.IsValid())
    {
        return false;
    }

    // Вариант 1:
    // "focus": [10.0, 20.0, 30.0]
    const TArray<TSharedPtr<FJsonValue>>* Array = nullptr;

    if (Object->TryGetArrayField(FieldName, Array)
        && Array != nullptr
        && Array->Num() >= 3
        && (*Array)[0].IsValid()
        && (*Array)[1].IsValid()
        && (*Array)[2].IsValid())
    {
        double X = 0.0;
        double Y = 0.0;
        double Z = 0.0;

        const bool bX = TryGetNumberFromValue(*(*Array)[0], X);
        const bool bY = TryGetNumberFromValue(*(*Array)[1], Y);
        const bool bZ = TryGetNumberFromValue(*(*Array)[2], Z);

        if (bX && bY && bZ)
        {
            OutVector = FVector(X, Y, Z);
            return true;
        }
    }

    // Вариант 2:
    // "focus": { "x": 10.0, "y": 20.0, "z": 30.0 }
    const TSharedPtr<FJsonObject>* VectorObjectPtr = nullptr;

    if (Object->TryGetObjectField(FieldName, VectorObjectPtr)
        && VectorObjectPtr != nullptr
        && (*VectorObjectPtr).IsValid())
    {
        const TSharedPtr<FJsonObject>& VectorObject = *VectorObjectPtr;

        double X = 0.0;
        double Y = 0.0;
        double Z = 0.0;

        const bool bHasX =
            TryGetNumberFlexible(VectorObject, TEXT("x"), X)
            || TryGetNumberFlexible(VectorObject, TEXT("X"), X);

        const bool bHasY =
            TryGetNumberFlexible(VectorObject, TEXT("y"), Y)
            || TryGetNumberFlexible(VectorObject, TEXT("Y"), Y);

        const bool bHasZ =
            TryGetNumberFlexible(VectorObject, TEXT("z"), Z)
            || TryGetNumberFlexible(VectorObject, TEXT("Z"), Z);

        if (bHasX || bHasY || bHasZ)
        {
            OutVector = FVector(X, Y, Z);
            return true;
        }
    }

    return false;
}

EApartmentStatus FApartmentParser::ParseStatus(const FString& StatusString)
{
    const FString Normalized = StatusString.TrimStartAndEnd().ToLower();

    // Поддерживаем русские и английские варианты.
    if (Normalized.Contains(TEXT("прод")) || Normalized.Equals(TEXT("sold")))
    {
        return EApartmentStatus::Sold;
    }

    return EApartmentStatus::Free;
}

FString FApartmentParser::MakeDefaultApartmentId(int32 FloorNumber, int32 ApartmentIndex)
{
    return FString::Printf(TEXT("Floor_%d_Apt_%d"), FloorNumber, ApartmentIndex);
}

bool FApartmentParser::TryGetNumberFlexible(
    const TSharedPtr<FJsonObject>& Object,
    const FString& FieldName,
    double& OutNumber
)
{
    if (!Object.IsValid())
    {
        return false;
    }

    // Сначала пробуем обычный number.
    if (Object->TryGetNumberField(FieldName, OutNumber))
    {
        return true;
    }

    // Если поле пришло строкой, пробуем преобразовать ее в число.
    FString StringValue;

    if (Object->TryGetStringField(FieldName, StringValue))
    {
        const FString CleanedValue = StringValue.TrimStartAndEnd();

        if (CleanedValue.IsNumeric())
        {
            OutNumber = FCString::Atod(*CleanedValue);
            return true;
        }
    }

    return false;
}

bool FApartmentParser::TryGetNumberFromValue(
    const FJsonValue& Value,
    double& OutNumber
)
{
    if (Value.TryGetNumber(OutNumber))
    {
        return true;
    }

    FString StringValue;

    if (Value.TryGetString(StringValue))
    {
        const FString CleanedValue = StringValue.TrimStartAndEnd();

        if (CleanedValue.IsNumeric())
        {
            OutNumber = FCString::Atod(*CleanedValue);
            return true;
        }
    }

    return false;
}