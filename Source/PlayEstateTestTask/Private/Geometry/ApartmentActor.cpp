//  Test task for Playestate.


#include "Geometry/ApartmentActor.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"


AApartmentActor::AApartmentActor()
{
    PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(Mesh);

    // Стандартный куб.
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));

    if (CubeMeshFinder.Succeeded())
    {
        Mesh->SetStaticMesh(CubeMeshFinder.Object);
    }

    // Дефолтный размер квартиры.
    Mesh->SetRelativeScale3D(FVector(4.f, 4.f, 2.5f));

    // Ховер и клики мыши.
    Mesh->SetGenerateOverlapEvents(true);
    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Mesh->SetCollisionResponseToAllChannels(ECR_Overlap);

    // События мыши.
    Mesh->OnClicked.AddDynamic(this, &AApartmentActor::HandleMeshClicked);
    Mesh->OnBeginCursorOver.AddDynamic(this, &AApartmentActor::HandleMeshBeginCursorOver);
    Mesh->OnEndCursorOver.AddDynamic(this, &AApartmentActor::HandleMeshEndCursorOver);
}

void AApartmentActor::BeginPlay()
{
    Super::BeginPlay();

    ApplyScale();
    CreateDynamicMaterial();
    RefreshVisual();
}

void AApartmentActor::Initialize(const FApartmentData& InData)
{
    Data = InData;

    if (bUseFocusPointAsActorLocation && !Data.CameraFocus.IsNearlyZero())
    {
        SetActorLocation(Data.CameraFocus);
    }

    ApplyScale();
    CreateDynamicMaterial();
    RefreshVisual();
}

FApartmentData AApartmentActor::GetApartmentData() const
{
    return Data;
}

void AApartmentActor::SetStatus(EApartmentStatus NewStatus)
{
    if (Data.Status == NewStatus)
    {
        return;
    }

    Data.Status = NewStatus;
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

    const bool bSold = Data.Status == EApartmentStatus::Sold;

    FLinearColor BaseColor = bSold ? SoldColor : FreeColor;

    FLinearColor EmissiveColor = FLinearColor::Black;
    float EmissiveStrength = 0.f;

    // Сначала hover, но если квартира выбрана, то selected.
    if (bHovered)
    {
        EmissiveColor = HoverEmissive;
        EmissiveStrength = HoverEmissiveStrength;
    }

    if (bSelected)
    {
        EmissiveColor = SelectedEmissive;
        EmissiveStrength = SelectedEmissiveStrength;
    }

    static const FName BaseColorName(TEXT("BaseColor"));
    static const FName EmissiveColorName(TEXT("EmissiveColor"));
    static const FName EmissiveStrengthName(TEXT("EmissiveStrength"));

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
    if (!Mesh || !bAutoScaleByArea)
    {
        return;
    }

    // Площадь в м².
    // Корень из площади дает примерную сторону квадрата в метрах.
    // Стандартный куб имеет размер 100 см = 1 м,
    // поэтому масштаб равен примерно стороне в метрах.
    float SideMeters = 3.f;

    if (Data.Area > 0.f)
    {
        SideMeters = FMath::Sqrt(Data.Area);
    }

    SideMeters = FMath::Max(SideMeters, 1.f);

    const float HeightScale = FMath::Max(ApartmentHeight / 100.f, 0.01f);

    Mesh->SetRelativeScale3D(
        FVector(
            SideMeters,
            SideMeters,
            HeightScale
        )
    );
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

    SetSelected(true);
    OnApartmentClicked.Broadcast(Data);
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