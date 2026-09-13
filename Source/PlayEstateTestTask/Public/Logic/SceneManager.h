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
	
	// Настройки для генерации 3D объектов
	UPROPERTY(EditDefaultsOnly)
	float FloorHeightSpacing = 500.f;
	
	UPROPERTY(EditDefaultsOnly)
	float ApartmentSpacing = 500.f;
	
	UPROPERTY(EditDefaultsOnly)
	float BaseZ = 100.f;
	
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
	void HandleApartmentClicked(FApartmentData Apartment, bool bIsSelected);
	
	UFUNCTION()
	void HandleFilterChanged(bool bHideSold);
	
	void SpawnApartments();
	void SetupUI();
	void UpdateApartmentInteraction();
	
	virtual void BeginPlay() override;

	
private:
	UPROPERTY()
	UMainWidget* MainWidget;
	
	UPROPERTY()
	TArray<AApartmentActor*> SpawnedApartments;
	
	bool bHideSoldFilterActive = false;
	TWeakObjectPtr<AApartmentActor> CurrentSelectedApartment;
	FBuildingConfig BuildingConfig;
	
	ACameraPawn* GetCameraPawn() const;
	FVector ComputeFloorTarget(int32 FloorLevel) const;	
	void ClearAllApartmentsSelection();
};