//  Test task for Playestate.


#include "UI/ActionButtonWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

DEFINE_LOG_CATEGORY_STATIC(LogActionButtonWidget, Log, All);

void UActionButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!ActionButton)
	{
		UE_LOG(LogActionButtonWidget, Error, TEXT("ActionButton is NULL"));
		return;
	}
	
	ActionButton->OnClicked.RemoveDynamic(this, &UActionButtonWidget::HandleButtonClicked);
	ActionButton->OnClicked.AddDynamic(this, &UActionButtonWidget::HandleButtonClicked);
}

void UActionButtonWidget::SetupButton(const FText& InLabel, int32 InContext)
{
	Context = InContext;

	if (!ActionLabel)
	{
		UE_LOG(LogActionButtonWidget, Error, TEXT("ActionLabel is NULL"));
		return;
	}
	
	ActionLabel->SetText(InLabel);
}

void UActionButtonWidget::SetButtonEnabled(bool bEnabled)
{
	if (!ActionButton)
	{
		UE_LOG(LogActionButtonWidget, Error, TEXT("ActionButton is NULL"));
		return;
	}
	
	ActionButton->SetIsEnabled(bEnabled);
}

void UActionButtonWidget::HandleButtonClicked()
{
	OnActionButtonClicked.Broadcast(Context);
}