// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultAttributeSet.h"

#include "GameplayEffectExtension.h"

void UDefaultAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue,0.f,GetMaxHealth());
	}
}

void UDefaultAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		const float LocalDmg = GetDamage();
		SetDamage(0.f);
		const float NewHealth = FMath::Clamp(GetHealth() - LocalDmg,0.f,GetMaxHealth());
		SetHealth(NewHealth);
	}
}
