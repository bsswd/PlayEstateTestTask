// Test task for Playestate.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "ApartmentConfigTypes.h"
#include "JsonLoader.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FJsonLoaderDelegate, const FBuildingConfig&, Config,
																	const TArray<FString>&, ErrorMessages);


UCLASS()
class PLAYESTATETESTTASK_API UJsonLoader : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FJsonLoaderDelegate OnCompleted;
	 
	
	UFUNCTION(BlueprintCallable, meta=(BlueprintInternalUseOnly="true", WorldContext="WorldContextObject"), Category="JsonLoader")
	static UJsonLoader* LoadBuildingConfigAsync(UObject* WorldContextObject, FString FileName);
 
	virtual void Activate() override;
	
private:
	FString TargetFile;
	void ExecuteLoading();
	
	void FinishLoading(const FBuildingConfig& Config, const TArray<FString>& Errors);
};