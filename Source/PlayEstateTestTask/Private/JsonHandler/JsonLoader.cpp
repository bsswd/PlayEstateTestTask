// Test task for PlayEstate company.


#include "PlayEstateTestTask/Public/JsonHandler/JsonLoader.h"


UJsonLoader* UJsonLoader::LoadBuildingConfigAsync(UObject* WorldContextObject, FString FileName)
{
	UJsonLoader* Action = NewObject<UJsonLoader>();
	Action->TargetFile = FileName;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UJsonLoader::Activate()
{
	Super::Activate();
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]() { ExecuteLoading(); });
}

void UJsonLoader::ExecuteLoading()
{
	FBuildingConfig Result;
    TArray<FString> DiagnosticLogs;
    FString FullPath = FPaths::ProjectContentDir() + "Data/" + TargetFile;
    FString JsonRaw;
 
    // Попытка чтения файла
    if (!FFileHelper::LoadFileToString(JsonRaw, *FullPath))
    {
        DiagnosticLogs.Add(FString::Printf(TEXT("Критическая ошибка: Файл не найден [%s]"), *FullPath));
        FinishLoading(Result, DiagnosticLogs);
        return;
    }
 
    // Инициализация парсера
    TSharedPtr<FJsonObject> RootObj;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonRaw);
 
    if (!FJsonSerializer::Deserialize(Reader, RootObj) || !RootObj.IsValid())
    {
        DiagnosticLogs.Add(TEXT("Критическая ошибка: Некорректный синтаксис JSON (проверьте скобки и запятые)."));
        FinishLoading(Result, DiagnosticLogs);
        return;
    }
 
    // Парсинг этажей
    const TArray<TSharedPtr<FJsonValue>>* FloorsArray;
    if (RootObj->TryGetArrayField(TEXT("Floors"), FloorsArray))
    {
        for (int32 f = 0; f < FloorsArray->Num(); ++f)
        {
            TSharedPtr<FJsonObject> FObj = (*FloorsArray)[f]->AsObject();
            if (!FObj.IsValid())
            {
                DiagnosticLogs.Add(FString::Printf(TEXT("Этаж в индексе [%d]: Объект поврежден."), f));
                continue;
            }
 
            FFloorData Floor;
            if (!FObj->TryGetNumberField(TEXT("FloorNumber"), Floor.FloorNumber))
            {
                DiagnosticLogs.Add(FString::Printf(TEXT("Этаж [%d]: Отсутствует номер FloorLevel."), f));
            }
 
            // Парсинг квартир
            const TArray<TSharedPtr<FJsonValue>>* ApartmentsArray;
            if (FObj->TryGetArrayField(TEXT("Apartments"), ApartmentsArray))
            {
                for (int32 Iterator = 0; Iterator < ApartmentsArray->Num(); ++Iterator)
                {
                    TSharedPtr<FJsonObject> ApartmentObj = (*ApartmentsArray)[Iterator]->AsObject();
                    if (!ApartmentObj.IsValid()) continue;
 
                    FApartmentData ApartmentData;
                    // Валидация ID
                    if (!ApartmentObj->TryGetNumberField(TEXT("ID"), ApartmentData.ID))
                    {
                        DiagnosticLogs.Add(FString::Printf(TEXT("Этаж %d: Пропущена квартира в индексе [%d] (отсутствует ID)."), Floor.FloorNumber, Iterator));
                        continue;
                    }
 
                    // Валидация площади
                    if (!ApartmentObj->TryGetNumberField(TEXT("Area"), ApartmentData.Area))
                    {
                        DiagnosticLogs.Add(FString::Printf(TEXT("Квартира %d: Area не найдена, установлено 0.0."), ApartmentData.ID));
                    }
                    
                    // Валидация статуса
                    FString StatusString;
                    if (!ApartmentObj->TryGetStringField(TEXT("Status"), StatusString))
                    {
                        // Ошибка: Ключ "Status" вообще не найден в JSON-объекте
                        DiagnosticLogs.Add(FString::Printf(TEXT("Квартира %d: Ошибка - ключ 'Status' не найден. Установлено 'Free'."), ApartmentData.ID));
                        ApartmentData.Status = EApartmentStatus::Free;
                    }
                    else
                    {
                        StatusString = StatusString.TrimStartAndEnd();
 
                        if (StatusString.IsEmpty())
                        {
                            // Ошибка: Поле есть, но строка пустая ("Status": "")
                            DiagnosticLogs.Add(FString::Printf(TEXT("Квартира %d: Ошибка - статус пуст."), ApartmentData.ID));
                            ApartmentData.Status = EApartmentStatus::Free;
                        }
                        else if (StatusString.Equals(TEXT("Sold"), ESearchCase::IgnoreCase))
                        {
                            // Успех: Значение распознано как "Продано"
                            ApartmentData.Status = EApartmentStatus::Sold;
                        }
                        else if (StatusString.Equals(TEXT("Free"), ESearchCase::IgnoreCase))
                        {
                            // Успех: Значение распознано как "Свободно"
                            ApartmentData.Status = EApartmentStatus::Free;
                        }
                        else
                        {
                            // Ошибка: Значение в JSON не совпадает с допустимыми
                            DiagnosticLogs.Add(FString::Printf(TEXT("Квартира %d: Ошибка - статус '%s' некорректен. Установлено 'Free'."), ApartmentData.ID, *StatusString));
                            ApartmentData.Status = EApartmentStatus::Free;
                        }
                    }
 
                    // Валидация фокуса камеры
                    const TSharedPtr<FJsonObject>* FocusObj;
                    if (ApartmentObj->TryGetObjectField(TEXT("CameraFocus"), FocusObj))
                    {
                        double X=0, Y=0, Z=0;
                        bool bCoords = true;
                        bCoords &= (*FocusObj)->TryGetNumberField(TEXT("x"), X);
                        bCoords &= (*FocusObj)->TryGetNumberField(TEXT("y"), Y);
                        bCoords &= (*FocusObj)->TryGetNumberField(TEXT("z"), Z);
                        
                        if (!bCoords) DiagnosticLogs.Add(FString::Printf(TEXT("Квартира %d: Неполные координаты CameraFocus."), ApartmentData.ID));
                        ApartmentData.CameraFocus = FVector(X, Y, Z);
                    }
                    else
                    {
                        DiagnosticLogs.Add(FString::Printf(TEXT("Квартира %d: CameraFocus не найден."), ApartmentData.ID));
                    }
 
                    Floor.Apartments.Add(ApartmentData);
                }
            }
            Result.Floors.Add(Floor);
        }
    }
    else
    {
        DiagnosticLogs.Add(TEXT("Ошибка: Корневой массив 'Floors' не найден."));
    }
 
    FinishLoading(Result, DiagnosticLogs);
}

void UJsonLoader::FinishLoading(const FBuildingConfig& Config, const TArray<FString>& Errors)
{
    // Передаем результат обратно в основной поток
    AsyncTask(ENamedThreads::GameThread, [this, Config, Errors]()
    {
        OnCompleted.Broadcast(Config, Errors);
        SetReadyToDestroy();
    });
}