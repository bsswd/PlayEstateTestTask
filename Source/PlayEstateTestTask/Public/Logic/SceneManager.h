//  Test task for Playestate.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JsonHandler/ApartmentConfigTypes.h"
#include "SceneManager.generated.h"

class UMainWidget;
class UUserWidget;
class ACameraPawn;
class AApartmentActor;

UCLASS()
class PLAYESTATETESTTASK_API ASceneManager : public AActor
{
	GENERATED_BODY()

public:
	ASceneManager();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AApartmentActor> ApartmentActorClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UUserWidget> MainWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString JsonFilePath;

	
protected:
	UFUNCTION()
	void HandleJsonLoaded(const FBuildingConfig& Config, const TArray<FString>& Errors);
	
	UFUNCTION()
	void HandleFloorSelected(int32 FloorLevel);

	UFUNCTION()
	void HandleBackRequested();

	UFUNCTION()
	void HandleApartmentClickedIn3D(FApartmentData Apartment, bool bIsSelected);
	
	void SpawnApartments();
	void SetupUI();
	
	virtual void BeginPlay() override;

	
private:
	UPROPERTY()
	UMainWidget* MainWidget;
	
	UPROPERTY()
	TArray<AApartmentActor*> SpawnedApartments;
	
	ACameraPawn* GetCameraPawn() const;
	FVector ComputeFloorTarget(int32 FloorLevel) const;	
	
	FBuildingConfig BuildingConfig;
};