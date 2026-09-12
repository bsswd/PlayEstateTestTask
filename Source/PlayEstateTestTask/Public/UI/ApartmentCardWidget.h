//  Test task for Playestate.

#pragma once


#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "JsonHandler/ApartmentConfigTypes.h"
#include "ApartmentCardWidget.generated.h"

class UActionButtonWidget;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBookRequested, FApartmentData, Apartment);

/**
 * Карточка квартиры
 */
UCLASS()
class PLAYESTATETESTTASK_API UApartmentCardWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FOnBookRequested OnBookRequested;
	
	
	UFUNCTION(BlueprintCallable)
	void SetupCard(const FApartmentData& InApartmentData);


protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ApartmentIdText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ApartmentAreaText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ApartmentStatusText;

	UPROPERTY(meta = (BindWidget))
	UActionButtonWidget* BookButton;
	
	
	UFUNCTION()
	void HandleBookClicked(int32 Context);
	
	virtual void NativeConstruct() override;

private:
	FApartmentData CurrentApartmentData;
};
