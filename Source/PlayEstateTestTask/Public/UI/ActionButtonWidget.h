//  Test task for Playestate.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ActionButtonWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionButtonClicked, int32, Context);

/**
 * Универсальный класс кнопки для использования в UI
 */
UCLASS()
class PLAYESTATETESTTASK_API UActionButtonWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// Настроить текст и контекст кнопки.
	// Для этажей контекст = номер этажа.
	// Для "Назад"/"Забронировать" контекст не важен (0).
	UFUNCTION(BlueprintCallable)
	void SetupButton(const FText& InLabel, int32 InContext = 0);

	UFUNCTION(BlueprintCallable)
	void SetButtonEnabled(bool bEnabled);

	UPROPERTY(BlueprintAssignable)
	FOnActionButtonClicked OnActionButtonClicked;

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleButtonClicked();

	UPROPERTY(meta = (BindWidget))
	UButton* ActionButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ActionLabel;

private:
	int32 Context = 0;
};
