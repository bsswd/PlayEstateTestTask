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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnApartmentClicked, FApartmentData, ApartmentData, bool, bIsSelected);

UCLASS()
class AApartmentActor : public AActor
{
    GENERATED_BODY()

public:
    AApartmentActor();
    
    UPROPERTY(BlueprintAssignable, Category = "Apartment")
    FOnApartmentClicked OnApartmentClicked;
    
    UFUNCTION(BlueprintCallable)
    void Initialize(const FApartmentData& InData);

    UFUNCTION(BlueprintCallable)
    FApartmentData GetApartmentData() const {return ApartmentData;}

    UFUNCTION(BlueprintCallable)
    void SetStatus(EApartmentStatus NewStatus);

    UFUNCTION(BlueprintCallable)
    void SetSelected(bool bInSelected);

    UFUNCTION(BlueprintCallable)
    void SetHovered(bool bInHovered);

    UFUNCTION(BlueprintCallable)
    void SetInteractionEnabled(bool bEnabled);
    
    UFUNCTION(BlueprintCallable)
    void SetFilteredOut(bool bFiltered);

    UFUNCTION(BlueprintCallable)
    void RefreshVisual();
    
    
    const FApartmentData& GetData() const  {return ApartmentData;}

    
protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UStaticMeshComponent> Mesh;

    // Данные квартиры.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Data")
    FApartmentData ApartmentData;

    // Материал с параметрами: BaseColor, EmissiveColor, EmissiveStrength.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Appearance")
    TObjectPtr<UMaterialInterface> BaseMaterial;

    // Цвета отвечающие за визуальное представление в 3D мире.
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

    // Разрешен ли клик по квартире.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|Interaction")
    bool bInteractionEnabled = true;

    // Обработчики клика и ховера.
    UFUNCTION()
    void HandleMeshClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

    UFUNCTION()
    void HandleMeshBeginCursorOver(UPrimitiveComponent* TouchedComponent);

    UFUNCTION()
    void HandleMeshEndCursorOver(UPrimitiveComponent* TouchedComponent);

    
    void CreateDynamicMaterial();
    void ApplyScale();

    virtual void BeginPlay() override;

    
private:
    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

    bool bSelected = false;
    bool bHovered = false;
    bool bFilteredOut = false;
};