//  Test task for Playestate.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloorActor.generated.h"

UCLASS()
class PLAYESTATETESTTASK_API AFloorActor : public AActor
{
	GENERATED_BODY()

public:
	AFloorActor();

	// Инициализация этажа: номер, размеры, высота потолка, позиция.
	UFUNCTION(BlueprintCallable)
	void InitializeFloor(int32 InFloorNumber,
							const FVector2D& Size,
								float Height,
									const FVector& Location);

	UFUNCTION(BlueprintCallable)
	int32 GetFloorNumber() const;

	// Точка обзора для камеры.
	UFUNCTION(BlueprintCallable)
	FVector GetCameraTarget() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 FloorNumber = 1;

	// Небольшой офсет точки камеры над площадкой.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CameraTargetZOffset = 100.f;
};
