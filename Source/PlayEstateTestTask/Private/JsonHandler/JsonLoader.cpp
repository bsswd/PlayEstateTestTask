// Test task for PlayEstate company.


#include "PlayEstateTestTask/Public/JsonHandler/JsonLoader.h"
#include "JsonObjectConverter.h"

bool UJsonLoader::LoadBuildingConfig(const FString& FileName, FApartmentConfig& OutConfig)
{
	// Путь к json файлу в Content/Data/
	FString FullPath = FPaths::ProjectContentDir() + "Data/" + FileName;
 
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FullPath))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load file: %s"), *FullPath);
		return false;
	}
 
	// Десериализация строки в структуру
	if (!FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &OutConfig, 0, 0))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON. Errors in data mapping."));
		return false;
	}
 
	return true;
}