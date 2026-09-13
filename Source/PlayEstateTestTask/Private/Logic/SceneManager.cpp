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
	
	if (!Loader)
	{
		UE_LOG(LogSceneManager, Error, TEXT("Failed to create JsonLoader"));
		return;
	}
	
	Loader->OnCompleted.AddDynamic(this, &ASceneManager::HandleJsonLoaded);
	Loader->Activate();
}

void ASceneManager::HandleJsonLoaded(const FBuildingConfig& Config, const TArray<FString>& Errors)
{
	for (const FString& Error : Errors)
	{
		UE_LOG(LogSceneManager, Warning, TEXT("[JsonLoader] %s"), *Error);
	}

	if (Config.Floors.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("No floors loaded!"));
		return;
	}

	BuildingConfig = Config;

	UE_LOG(LogSceneManager, Warning, TEXT("Loaded %d floors"), BuildingConfig.Floors.Num());

	SpawnApartments();
	SetupUI();
}

void ASceneManager::SpawnApartments()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogSceneManager, Error, TEXT("World is null."));
    	return;	
    }
	
    if (!ApartmentActorClass)
    {
        UE_LOG(LogSceneManager, Error, TEXT("ApartmentActorClass is not set."));
        return;
    }

    SpawnedApartments.Empty();

	for (const FFloorData& Floor : BuildingConfig.Floors)
	{
		for (const FApartmentData& Apartment : Floor.Apartments)
		{
			FApartmentData ApartmentCopy = Apartment;
			
			const FVector SpawnLocation = Apartment.CameraFocus;
			FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);

			AApartmentActor* Actor = World->SpawnActorDeferred<AApartmentActor>(
				ApartmentActorClass,
				SpawnTransform,
				this,
				nullptr,
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn
			);

			if (!Actor)
			{
				UE_LOG(LogSceneManager, Error, TEXT("No apartment actor"));
				return;
			}

			Actor->Initialize(ApartmentCopy);
			Actor->FinishSpawning(SpawnTransform);
			Actor->OnApartmentClicked.AddDynamic(this, &ASceneManager::HandleApartmentClicked);
			SpawnedApartments.Add(Actor);
		}
	}

	UE_LOG(LogSceneManager, Warning, TEXT("Spawned %d apartments from JSON coordinates"), SpawnedApartments.Num());

	UpdateApartmentInteraction();
    UE_LOG(LogTemp, Warning, TEXT("Spawned %d apartments procedurally"), SpawnedApartments.Num());
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
	MainWidget->OnFilterChanged.AddDynamic(this, &ASceneManager::HandleFilterChanged);
}

void ASceneManager::UpdateApartmentInteraction()
{
	ACameraPawn* CameraPawn = GetCameraPawn();
	const ECameraMode Mode = CameraPawn ? CameraPawn->GetCameraMode() : ECameraMode::Genplan;
	const bool bInGenplan = (Mode == ECameraMode::Genplan);

	for (AApartmentActor* Apartment : SpawnedApartments)
	{
		if (!Apartment) continue;

		const FApartmentData Data = Apartment->GetData();
		const bool bIsSold = (Data.Status == EApartmentStatus::Sold);

		// Визуальное "скрытие" — всегда применяется, если фильтр включён и квартира продана
		Apartment->SetFilteredOut(bHideSoldFilterActive && bIsSold);

		// Интерактивность: выключена в Genplan И для отфильтрованных квартир
		const bool bShouldBeInteractive = !bInGenplan && !(bHideSoldFilterActive && bIsSold);
		Apartment->SetInteractionEnabled(bShouldBeInteractive);
	}
}

void ASceneManager::HandleFloorSelected(int32 FloorLevel)
{
	UE_LOG(LogSceneManager, Warning, TEXT("UI: Select floor %d"), FloorLevel);
	
	ClearAllApartmentsSelection();

	if (MainWidget)
		MainWidget->HideApartmentCard();

	if (ACameraPawn* Camera = GetCameraPawn())
	{
		const FVector FloorCenter = ComputeFloorTarget(FloorLevel);
		Camera->EnterFloorView(FloorCenter);
	}
	
	UpdateApartmentInteraction();
}

void ASceneManager::HandleBackRequested()
{
	UE_LOG(LogSceneManager, Warning, TEXT("UI: Back requested"));
	
	ClearAllApartmentsSelection();

	if (MainWidget)
		MainWidget->HideApartmentCard();

	if (ACameraPawn* Camera = GetCameraPawn())
		Camera->GoBack();
}

void ASceneManager::HandleApartmentClicked(FApartmentData Apartment, bool bIsSelected)
{	
	if (bIsSelected)
	{
		AApartmentActor* TargetActor = nullptr;

		for (AApartmentActor* ApartmentActor : SpawnedApartments)
		{
			if (ApartmentActor && ApartmentActor->GetData().ID == Apartment.ID)
			{
				TargetActor = ApartmentActor;
				break;
			}
		}

		if (!TargetActor)
		{
			return;
		}

		if (CurrentSelectedApartment.IsValid() && CurrentSelectedApartment.Get() != TargetActor)
		{
			CurrentSelectedApartment->SetSelected(false);
		}

		TargetActor->SetSelected(true);
		CurrentSelectedApartment = TargetActor;

		if (ACameraPawn* CameraPawn = GetCameraPawn())
		{
			// Позиция квартиры (куда смотрим)
			const FVector ApartmentLocation = TargetActor->GetActorLocation();

			// Камера встаёт слева сверху под углом 35° и смотрит на квартиру
			CameraPawn->EnterApartmentView(ApartmentLocation);
		}

		if (MainWidget)
		{
			MainWidget->ShowApartmentCard(Apartment);
		}
	}
	else
	{
		if (CurrentSelectedApartment.IsValid())
		{
			CurrentSelectedApartment->SetSelected(false);
			CurrentSelectedApartment = nullptr;
		}

		if (MainWidget)
		{
			MainWidget->HideApartmentCard();
		}
	}

	UpdateApartmentInteraction();
}

void ASceneManager::HandleFilterChanged(bool bHideSold)
{
	bHideSoldFilterActive = bHideSold;
	UpdateApartmentInteraction();
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

void ASceneManager::ClearAllApartmentsSelection()
{
	for (AApartmentActor* Apartment : SpawnedApartments)
	{
		if (!Apartment)
		{
			UE_LOG(LogSceneManager, Warning, TEXT("Apartment not found."));
			continue;
		}
		
		Apartment->SetSelected(false);
	}

	CurrentSelectedApartment = nullptr;
}