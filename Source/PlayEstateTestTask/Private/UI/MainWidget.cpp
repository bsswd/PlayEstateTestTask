//  Test task for Playestate.

#include "UI/MainWidget.h"
#include "UI/ActionButtonWidget.h"
#include "UI/ApartmentCardWidget.h"
#include "Components/VerticalBox.h"

DEFINE_LOG_CATEGORY_STATIC(LogMainWidget, Log, All);

void UMainWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (!BackButton)
    {
        UE_LOG(LogMainWidget, Error, TEXT("BackButton is not set"));
        return;
    }
    
    BackButton->OnActionButtonClicked.RemoveDynamic(this, &UMainWidget::HandleBackClicked);
    BackButton->OnActionButtonClicked.AddDynamic(this, &UMainWidget::HandleBackClicked);
    BackButton->SetupButton(INVTEXT("Назад"), 0);

    HideApartmentCard();
}

void UMainWidget::Setup(const FBuildingConfig& InConfig)
{
    CurrentConfig = InConfig;
    RebuildFloorsPanel();
}

void UMainWidget::RebuildFloorsPanel()
{
    UE_LOG(LogMainWidget, Warning, TEXT("Start rebuild floor panels"));
    
    
    if (!FloorPanel)
    {
        UE_LOG(LogMainWidget, Error, TEXT("FloorPanel is not set"));
        return;
    }

    FloorPanel->ClearChildren();

    if (!ActionButtonClass)
    {
        UE_LOG(LogMainWidget, Error, TEXT("ActionButtonClass is not set"));
        return;
    }

    for (const FFloorData& Floor : CurrentConfig.Floors)
    {
        UActionButtonWidget* FloorBtn = CreateWidget<UActionButtonWidget>(this, ActionButtonClass);
        if (!FloorBtn)
        {
            continue;
        }

        FloorBtn->SetupButton(
            FText::Format(INVTEXT("Этаж {0}"), Floor.FloorLevel),
            Floor.FloorLevel
        );

        FloorBtn->OnActionButtonClicked.AddDynamic(this, &UMainWidget::HandleFloorButtonClicked);
        FloorPanel->AddChild(FloorBtn);
    }
}

void UMainWidget::HandleFloorButtonClicked(int32 FloorNumber)
{
    OnFloorSelected.Broadcast(FloorNumber);
}

void UMainWidget::HandleBackClicked(int32 Context)
{
    OnBackRequested.Broadcast();
}

void UMainWidget::ShowApartmentCard(const FApartmentData& InApartment)
{
    if (!ApartmentCard)
    {
        UE_LOG(LogMainWidget, Error, TEXT("ApartmentCard is not set"));
        return;
    }
    
    ApartmentCard->SetupCard(InApartment);
    ApartmentCard->SetVisibility(ESlateVisibility::Visible);
}

void UMainWidget::HideApartmentCard()
{
    if (!ApartmentCard)
    {
        UE_LOG(LogMainWidget, Error, TEXT("ApartmentCard is not set"));
        return;
    }
    
    ApartmentCard->SetVisibility(ESlateVisibility::Collapsed);
}