// Test task for PlayEstate company.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ApartmentConfigTypes.h"
#include "JsonLoader.generated.h"

/**
 * 
 */
UCLASS()
class PLAYESTATETESTTASK_API UJsonLoader : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Config")
	static bool LoadBuildingConfig(const FString& FileName, FApartmentConfig& OutConfig);
};


