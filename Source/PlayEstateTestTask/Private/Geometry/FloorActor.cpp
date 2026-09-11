//  Test task for Playestate.

#include "Geometry/FloorActor.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"

AFloorActor::AFloorActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	// Используем стандартный куб.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(
		TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'")
	);

	if (CubeMeshFinder.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMeshFinder.Object);
	}

	// Дефолтные размеры для этажа:
	Mesh->SetRelativeScale3D(FVector(20.f, 20.f, 0.2f));

	// Чтобы этаж не мешал кликать по квартирам, отключаем коллизию.
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFloorActor::InitializeFloor(int32 InFloorNumber,
									const FVector2D& Size,
										float Height,
											const FVector& Location)
{
	FloorNumber = InFloorNumber;

	SetActorLocation(Location);

	// Size и Thickness ожидаем в сантиметрах.
	// Стандартный куб имеет размер 100 см, поэтому делим на 100.
	const FVector Scale(
		FMath::Max(Size.X / 100.f, 0.01f),
		FMath::Max(Size.Y / 100.f, 0.01f),
		FMath::Max(Height / 100.f, 0.01f)
	);

	Mesh->SetRelativeScale3D(Scale);
}

int32 AFloorActor::GetFloorNumber() const
{
	return FloorNumber;
}

FVector AFloorActor::GetCameraTarget() const
{
	return GetActorLocation() + FVector(0.f, 0.f, CameraTargetZOffset);
}

