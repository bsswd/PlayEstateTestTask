// Test task for PlayEstate company.


#pragma once

#include "CoreMinimal.h"
#include "ApartmentConfigTypes.generated.h"

UENUM(BlueprintType)
enum class EApartmentStatus : uint8
{
	Free	UMETA(DisplayName = "Свободно"),
	Sold    UMETA(DisplayName = "Продано")
};

USTRUCT(BlueprintType)
struct FApartmentData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Apartment", meta = (ClampMin = 1, UIMin = 1))
	int32 ID = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Apartment")
	EApartmentStatus Status = EApartmentStatus::Free;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Apartment")
	float Area = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Apartment")
	FVector CameraFocus = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FFloorData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor", meta = (ClampMin = 1, ClampMax = 100, UIMin = 1, UIMax = 100))
	int32 FloorNumber = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor")
	TArray<FApartmentData> Apartments;
};

USTRUCT(BlueprintType)
struct FBuildingConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TArray<FFloorData> Floors;
};
