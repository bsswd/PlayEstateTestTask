//  Test task for Playestate.


#include "UI/ApartmentCardWidget.h"
#include "UI/ActionButtonWidget.h"
#include "Components/TextBlock.h"

DEFINE_LOG_CATEGORY_STATIC(LogApartmentCardWidget, Log, All);

void UApartmentCardWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!BookButton)
	{
		UE_LOG(LogApartmentCardWidget, Error, TEXT("BookButton is NULL"));
		return;
	}
	
	BookButton->SetupButton(INVTEXT("Забронировать"), 0);
	BookButton->OnActionButtonClicked.RemoveDynamic(this, &UApartmentCardWidget::HandleBookClicked);
	BookButton->OnActionButtonClicked.AddDynamic(this, &UApartmentCardWidget::HandleBookClicked);
}

void UApartmentCardWidget::SetupCard(const FApartmentData& InApartmentData)
{
	CurrentApartmentData = InApartmentData;

	if (!ApartmentIdText)
	{
		UE_LOG(LogApartmentCardWidget, Error, TEXT("ApartmentIdText is NULL"));
		return;
	}
	
	ApartmentIdText->SetText(FText::AsNumber(InApartmentData.ID, &FNumberFormattingOptions::DefaultNoGrouping()));

	if (!ApartmentAreaText)
	{
		UE_LOG(LogApartmentCardWidget, Error, TEXT("ApartmentAreaText is NULL"));
		return;
	}
	
	ApartmentAreaText->SetText(FText::FromString(FString::Printf(TEXT("%.2f м²"), InApartmentData.Area)));

	if (ApartmentStatusText)
	{
		UE_LOG(LogApartmentCardWidget, Error, TEXT("ApartmentStatusText is NULL"));
		return;
	}
	const FString StatusStr = InApartmentData.Status == EApartmentStatus::Sold
		? TEXT("Продано")
		: TEXT("Свободно");
	ApartmentStatusText->SetText(FText::FromString(StatusStr));
}

void UApartmentCardWidget::HandleBookClicked(int32 Context)
{
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("BOOKING REQUEST: Apartment Id=%d, Area=%.2f, Status=%s"),
		CurrentApartmentData.ID,
		CurrentApartmentData.Area,
		CurrentApartmentData.Status == EApartmentStatus::Sold ? TEXT("Sold") : TEXT("Free")
	);

	OnBookRequested.Broadcast(CurrentApartmentData);
}