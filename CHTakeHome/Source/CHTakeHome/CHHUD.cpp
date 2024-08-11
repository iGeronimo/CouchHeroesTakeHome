#include "CHHUD.h"
#include "Components/ProgressBar.h"

void UCHHUD::SetHealth(float CurrentHealth, float MaxHealth)
{
	if (HealthBar)
	{
		HealthBar->SetPercent(CurrentHealth / MaxHealth);
	}
}

void UCHHUD::SetStamina(float CurrentStamina, float MaxStamina)
{
	if (StaminaBar)
	{
		StaminaBar->SetPercent(CurrentStamina / MaxStamina);
	}
}

//void UCHHUD::PostLoad()
//{
//	loaded = true;
//}
