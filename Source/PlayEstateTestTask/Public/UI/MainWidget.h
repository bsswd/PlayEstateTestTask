//  Test task for Playestate.

#pragma once


#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "JsonHandler/ApartmentConfigTypes.h"
#include "MainWidget.generated.h"

class UVerticalBox;
class UActionButtonWidget;
class UApartmentCardWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFloorSelected, int32, FloorLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBackRequested);

/**
 * Главный виджет, который изменяется в зависимости от режима просмотра.
 */

UCLASS()
class PLAYESTATETESTTASK_API UMainWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnFloorSelected OnFloorSelected;

	UPROPERTY(BlueprintAssignable)
	FOnBackRequested OnBackRequested;

	// Класс универсальной кнопки (назначается в Blueprint).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UActionButtonWidget> ActionButtonClass;
	
	
	UFUNCTION(BlueprintCallable)
	void Setup(const FBuildingConfig& InConfig);

	UFUNCTION(BlueprintCallable)
	void ShowApartmentCard(const FApartmentData& InApartment);

	UFUNCTION(BlueprintCallable)
	void HideApartmentCard();
	

protected:
	// Панель кнопок выбора этажа.
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* FloorPanel;

	// Кнопка "Назад".
	UPROPERTY(meta = (BindWidget))
	UActionButtonWidget* BackButton;

	// Карточка квартиры.
	UPROPERTY(meta = (BindWidget))
	UApartmentCardWidget* ApartmentCard;
	
	
	UFUNCTION()
	void HandleFloorButtonClicked(int32 FloorNumber);

	UFUNCTION()
	void HandleBackClicked(int32 Context);

	
	virtual void NativeConstruct() override;

	
private:
	FBuildingConfig CurrentConfig;

	void RebuildFloorsPanel();
};