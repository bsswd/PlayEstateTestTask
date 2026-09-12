//  Test task for Playestate.


#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JsonHandler/ApartmentConfigTypes.h"
#include "InputCoreTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ApartmentActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnApartmentClicked, FApartmentData, ApartmentData);

UCLASS()
class AApartmentActor : public AActor
{
    GENERATED_BODY()

public:
    AApartmentActor();

    // Инициализация.
    UFUNCTION(BlueprintCallable)
    void Initialize(const FApartmentData& InData);

    // Для Blueprint.
    UFUNCTION(BlueprintCallable)
    FApartmentData GetApartmentData() const;

    // Для C++.
    const FApartmentData& GetData() const
    {
        return Data;
    }

    UFUNCTION(BlueprintCallable)
    void SetStatus(EApartmentStatus NewStatus);

    UFUNCTION(BlueprintCallable)
    void SetSelected(bool bInSelected);

    UFUNCTION(BlueprintCallable)
    void SetHovered(bool bInHovered);

    UFUNCTION(BlueprintCallable)
    void SetInteractionEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable)
    void RefreshVisual();
    
    // Событие для контроллера, камеры, UI и.т.д...
    UPROPERTY(BlueprintAssignable, Category = "Apartment")
    FOnApartmentClicked OnApartmentClicked;

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void HandleMeshClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

    UFUNCTION()
    void HandleMeshBeginCursorOver(UPrimitiveComponent* TouchedComponent);

    UFUNCTION()
    void HandleMeshEndCursorOver(UPrimitiveComponent* TouchedComponent);

    void CreateDynamicMaterial();
    void ApplyScale();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UStaticMeshComponent> Mesh;

    // Данные квартиры.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Data")
    FApartmentData Data;

    // Если включено, актер поставит себя в FocusPoint из JSON.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Data")
    bool bUseFocusPointAsActorLocation = true;

    // Если включено, размер куба будет примерно зависеть от площади.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Data")
    bool bAutoScaleByArea = true;

    // Высота куба квартиры в сантиметрах.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Data")
    float ApartmentHeight = 250.f;

    // Материал с параметрами: BaseColor, EmissiveColor, EmissiveStrength.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Appearance")
    TObjectPtr<UMaterialInterface> BaseMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Appearance")
    FLinearColor FreeColor = FLinearColor(0.75f, 0.8f, 0.85f, 1.f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Appearance")
    FLinearColor SoldColor = FLinearColor(0.25f, 0.25f, 0.27f, 1.f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Appearance")
    FLinearColor HoverEmissive = FLinearColor(0.f, 0.6f, 1.f, 1.f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Appearance")
    FLinearColor SelectedEmissive = FLinearColor(1.f, 0.5f, 0.f, 1.f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Appearance")
    float HoverEmissiveStrength = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Appearance")
    float SelectedEmissiveStrength = 2.5f;

    // Разрешено ли наводить/клики по квартире.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Interaction")
    bool bInteractionEnabled = true;

private:
    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

    bool bSelected = false;
    bool bHovered = false;
};