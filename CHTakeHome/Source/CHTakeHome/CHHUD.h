#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CHHUD.generated.h"

UCLASS(Abstract)
class UCHHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetHealth(float CurrentHealth, float MaxHealth);

	void SetStamina(float CurrentStamina, float MaxStamina);

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UProgressBar* HealthBar;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	class UProgressBar* StaminaBar;

	UPROPERTY(EditAnywhere)
	bool loaded;
};