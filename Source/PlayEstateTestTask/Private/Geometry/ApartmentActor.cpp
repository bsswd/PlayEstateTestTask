//  Test task for Playestate.

#include "Geometry/ApartmentActor.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"

AApartmentActor::AApartmentActor()
{
    PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(Mesh);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));

    if (CubeMeshFinder.Succeeded())
    {
        Mesh->SetStaticMesh(CubeMeshFinder.Object);
    }

    Mesh->SetRelativeScale3D(FVector(4.f, 4.f, 2.5f));

    Mesh->SetGenerateOverlapEvents(true);
    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Mesh->SetCollisionResponseToAllChannels(ECR_Overlap);

    Mesh->OnClicked.AddDynamic(this, &AApartmentActor::HandleMeshClicked);
    Mesh->OnBeginCursorOver.AddDynamic(this, &AApartmentActor::HandleMeshBeginCursorOver);
    Mesh->OnEndCursorOver.AddDynamic(this, &AApartmentActor::HandleMeshEndCursorOver);
}

void AApartmentActor::BeginPlay()
{
    Super::BeginPlay();

    CreateDynamicMaterial();
    RefreshVisual();
}

void AApartmentActor::Initialize(const FApartmentData& InData)
{
    ApartmentData = InData;

    CreateDynamicMaterial();
    RefreshVisual();
}

void AApartmentActor::SetStatus(EApartmentStatus NewStatus)
{
    if (ApartmentData.Status == NewStatus)
    {
        return;
    }

    ApartmentData.Status = NewStatus;
    RefreshVisual();
}

void AApartmentActor::SetSelected(bool bInSelected)
{
    if (bSelected == bInSelected)
    {
        return;
    }

    bSelected = bInSelected;
    RefreshVisual();
}

void AApartmentActor::SetHovered(bool bInHovered)
{
    if (bHovered == bInHovered)
    {
        return;
    }

    bHovered = bInHovered;
    RefreshVisual();
}

void AApartmentActor::SetInteractionEnabled(bool bEnabled)
{
    if (bInteractionEnabled == bEnabled)
    {
        return;
    }

    bInteractionEnabled = bEnabled;

    if (!bInteractionEnabled)
    {
        SetHovered(false);
    }
}

void AApartmentActor::SetFilteredOut(bool bFiltered)
{
    if (bFilteredOut == bFiltered)
    {
        return;
    }

    bFilteredOut = bFiltered;
    RefreshVisual();
}

void AApartmentActor::RefreshVisual()
{
    if (!DynamicMaterial)
    {
        CreateDynamicMaterial();
    }

    if (!DynamicMaterial)
    {
        return;
    }

    static const FName BaseColorName(TEXT("BaseColor"));
    static const FName EmissiveColorName(TEXT("EmissiveColor"));
    static const FName EmissiveStrengthName(TEXT("EmissiveStrength"));

    // Базовый цвет: ВСЕ квартиры зелёные по умолчанию
    FLinearColor BaseColor = FLinearColor(0.2f, 0.8f, 0.2f);  // зелёный
    FLinearColor EmissiveColor = FLinearColor::Black;
    float EmissiveStrength = 0.f;

    // Если квартира отфильтрована (продана и фильтр включён) — затемняем
    if (bFilteredOut)
    {
        BaseColor = FLinearColor(0.1f, 0.1f, 0.1f);  // тёмно-серый
        EmissiveColor = FLinearColor::Black;
        EmissiveStrength = 0.f;

        DynamicMaterial->SetVectorParameterValue(BaseColorName, BaseColor);
        DynamicMaterial->SetVectorParameterValue(EmissiveColorName, EmissiveColor);
        DynamicMaterial->SetScalarParameterValue(EmissiveStrengthName, EmissiveStrength);
        return;
    }

    // Подсветка ховера
    if (bHovered)
    {
        EmissiveColor = HoverEmissive;
        EmissiveStrength = HoverEmissiveStrength;
    }

    // Подсветка выбора (приоритет выше ховера)
    if (bSelected)
    {
        EmissiveColor = SelectedEmissive;
        EmissiveStrength = SelectedEmissiveStrength;
    }

    DynamicMaterial->SetVectorParameterValue(BaseColorName, BaseColor);
    DynamicMaterial->SetVectorParameterValue(EmissiveColorName, EmissiveColor);
    DynamicMaterial->SetScalarParameterValue(EmissiveStrengthName, EmissiveStrength);
}

void AApartmentActor::CreateDynamicMaterial()
{
    if (DynamicMaterial || !Mesh)
    {
        return;
    }

    UMaterialInterface* SourceMaterial;
    if (BaseMaterial)
    {
        SourceMaterial = BaseMaterial;
    }
    else
    {
        SourceMaterial = Mesh->GetMaterial(0);
    }

    if (SourceMaterial)
    {
        DynamicMaterial = Mesh->CreateDynamicMaterialInstance(0, SourceMaterial);
    }
    else
    {
        DynamicMaterial = Mesh->CreateDynamicMaterialInstance(0);
    }
}

void AApartmentActor::ApplyScale()
{
    
}

void AApartmentActor::HandleMeshClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
    if (!bInteractionEnabled)
    {
        return;
    }

    if (ButtonPressed != EKeys::LeftMouseButton)
    {
        return;
    }

    SetSelected(!bSelected);
    OnApartmentClicked.Broadcast(ApartmentData, bSelected);
}

void AApartmentActor::HandleMeshBeginCursorOver(UPrimitiveComponent* TouchedComponent)
{
    if (!bInteractionEnabled)
    {
        return;
    }

    SetHovered(true);
}

void AApartmentActor::HandleMeshEndCursorOver(UPrimitiveComponent* TouchedComponent)
{
    SetHovered(false);
}