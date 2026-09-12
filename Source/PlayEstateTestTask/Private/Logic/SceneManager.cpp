//  Test task for Playestate.

#include "Logic/SceneManager.h"
#include "Blueprint/UserWidget.h"
#include "CameraSystem/CameraPawn.h"
#include "Geometry/ApartmentActor.h"
#include "JsonHandler/ApartmentConfigTypes.h"
#include "JsonHandler/JsonLoader.h"
#include "UI/MainWidget.h"

DEFINE_LOG_CATEGORY_STATIC(LogSceneManager, Log, All);

ASceneManager::ASceneManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASceneManager::BeginPlay()
{
	Super::BeginPlay();
	
	UJsonLoader* Loader = UJsonLoader::LoadBuildingConfigAsync(this, JsonFilePath);
	
	if (Loader)
	{
		Loader->OnCompleted.AddDynamic(this, &ASceneManager::HandleJsonLoaded);
		Loader->Activate();
	}
	else
	{
		UE_LOG(LogSceneManager, Error, TEXT("Failed to create JsonLoader"));
	}
}

void ASceneManager::HandleJsonLoaded(const FBuildingConfig& Config, const TArray<FString>& Errors)
{
	for (const FString& Error : Errors)
	{
		UE_LOG(LogSceneManager, Warning, TEXT("[JsonLoader] %s"), *Error);
	}

	if (Config.Floors.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("No floors loaded! Abort."));
		return;
	}

	BuildingConfig = Config;

	UE_LOG(LogSceneManager, Warning, TEXT("Loaded %d floors"), BuildingConfig.Floors.Num());

	SpawnApartments();
	SetupUI();
}

void ASceneManager::SpawnApartments()
{
    if (!ApartmentActorClass)
    {
        UE_LOG(LogSceneManager, Error, TEXT("ApartmentActorClass is not set"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    SpawnedApartments.Empty();

    // Настройки размещения
    const float FloorHeightSpacing = 400.f;  // Высота между этажами (4 метра)
    const float ApartmentSpacing = 600.f;     // Расстояние между квартирами (6 метров)
    const float BaseFloorZ = 200.f;           // Высота первого этажа

    for (int32 FloorIndex = 0; FloorIndex < BuildingConfig.Floors.Num(); ++FloorIndex)
    {
        const FFloorData& Floor = BuildingConfig.Floors[FloorIndex];
        const float FloorZ = BaseFloorZ + FloorIndex * FloorHeightSpacing;

        // Размещаем квартиры сеткой 2x2 (или в ряд, если больше)
        const float NumApartments = Floor.Apartments.Num();
        const int32 GridSize = FMath::CeilToInt(FMath::Sqrt(NumApartments));

        for (int32 AptIndex = 0; AptIndex < NumApartments; ++AptIndex)
        {
            const FApartmentData& Apartment = Floor.Apartments[AptIndex];

            // Вычисляем позицию в сетке
            const int32 Row = AptIndex / GridSize;
            const int32 Col = AptIndex % GridSize;

            // Центрируем сетку
            const float OffsetX = (Col - (GridSize - 1) * 0.5f) * ApartmentSpacing;
            const float OffsetY = (Row - (GridSize - 1) * 0.5f) * ApartmentSpacing;

            FVector SpawnLocation(OffsetX, OffsetY, FloorZ);
            FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);

            AApartmentActor* Actor = World->SpawnActorDeferred<AApartmentActor>(
                ApartmentActorClass,
                SpawnTransform,
                this,
                nullptr,
                ESpawnActorCollisionHandlingMethod::AlwaysSpawn
            );

            if (Actor)
            {
                Actor->Initialize(Apartment);
                Actor->FinishSpawning(SpawnTransform);
                Actor->OnApartmentClicked.AddDynamic(this, &ASceneManager::HandleApartmentClickedIn3D);
                SpawnedApartments.Add(Actor);
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Spawned %d apartments procedurally"), SpawnedApartments.Num());

    // Центрируем камеру на здании
    if (SpawnedApartments.Num() > 0)
    {
        FVector BuildingCenter = FVector::ZeroVector;
        for (AApartmentActor* Apt : SpawnedApartments)
        {
            BuildingCenter += Apt->GetActorLocation();
        }
        BuildingCenter /= SpawnedApartments.Num();

        if (ACameraPawn* Camera = GetCameraPawn())
        {
            const float BuildingRadius = 4000.f;
            Camera->SetBuildingView(BuildingCenter, BuildingRadius);
        }
    }
}

void ASceneManager::SetupUI()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !MainWidgetClass)
	{
		UE_LOG(LogSceneManager, Error, TEXT("Cannot create MainWidget"));
		return;
	}

	UUserWidget* RawWidget = CreateWidget<UUserWidget>(PC, MainWidgetClass);
	MainWidget = Cast<UMainWidget>(RawWidget);

	if (!MainWidget)
	{
		UE_LOG(LogSceneManager, Error, TEXT("MainWidget is not UMainWidget"));
		return;
	}

	MainWidget->AddToViewport();
	MainWidget->Setup(BuildingConfig);

	MainWidget->OnFloorSelected.AddDynamic(this, &ASceneManager::HandleFloorSelected);
	MainWidget->OnBackRequested.AddDynamic(this, &ASceneManager::HandleBackRequested);
}



void ASceneManager::HandleFloorSelected(int32 FloorLevel)
{
	UE_LOG(LogSceneManager, Warning, TEXT("UI: Select floor %d"), FloorLevel);

	if (MainWidget)
	{
		MainWidget->HideApartmentCard();
	}

	if (ACameraPawn* Camera = GetCameraPawn())
	{
		const FVector FloorTarget = ComputeFloorTarget(FloorLevel);
		Camera->EnterFloor(FloorTarget, 3000.f, -30.f);
	}
}

void ASceneManager::HandleBackRequested()
{
	UE_LOG(LogSceneManager, Warning, TEXT("UI: Back requested"));

	if (MainWidget)
	{
		MainWidget->HideApartmentCard();
	}

	if (ACameraPawn* Camera = GetCameraPawn())
	{
		Camera->GoBack();
	}
}

void ASceneManager::HandleApartmentClickedIn3D(FApartmentData Apartment, bool bIsSelected)
{
	if (bIsSelected)
	{
		AApartmentActor* TargetActor = nullptr;
		for (AApartmentActor* Actor : SpawnedApartments)
		{
			if (Actor && Actor->GetData().ID == Apartment.ID)
			{
				TargetActor = Actor;
				break;
			}
		}

		if (ACameraPawn* CameraPawn = Cast<ACameraPawn>(GetCameraPawn()); TargetActor && CameraPawn)
		{
			const FVector FocusPoint = TargetActor->GetActorLocation();
			CameraPawn->EnterApartment(FocusPoint, 1200.f, -20.f);
		}

		if (MainWidget)
		{
			MainWidget->ShowApartmentCard(Apartment);
		}
	}
	else
	{
		if (MainWidget)
		{
			MainWidget->HideApartmentCard();
		}
	}
}

ACameraPawn* ASceneManager::GetCameraPawn() const
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		return nullptr;
	}

	return Cast<ACameraPawn>(PC->GetPawn());
}

FVector ASceneManager::ComputeFloorTarget(int32 FloorLevel) const
{
	FVector Target = FVector::ZeroVector;
	int32 Count = 0;

	for (const FFloorData& Floor : BuildingConfig.Floors)
	{
		if (Floor.FloorLevel != FloorLevel)
		{
			continue;
		}
		
		for (const FApartmentData& Apartment : Floor.Apartments)
		{
			Target += Apartment.CameraFocus;
			Count++;
		}
		break;
	}

	if (Count > 0)
	{
		Target /= Count;
	}

	return Target;
}