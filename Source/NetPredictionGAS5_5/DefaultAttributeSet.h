// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"

#include "DefaultAttributeSet.generated.h"

/**
 * No replication or On Rep are needed.
 */
UCLASS()
class NETPREDICTIONGAS5_5_API UDefaultAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

protected:
	/** Sample "Health" Attribute */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayAttributeData Health;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayAttributeData MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayAttributeData MoveSpeed;

	// Meta Attribute Helps with calculations.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayAttributeData Damage;
	//~ ... Other Gameplay Attributes here ...

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
 
public:
	//~ Helper functions for "Health" attributes
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UDefaultAttributeSet, Health);
	GAMEPLAYATTRIBUTE_VALUE_GETTER(Health);
	GAMEPLAYATTRIBUTE_VALUE_SETTER(Health);
	GAMEPLAYATTRIBUTE_VALUE_INITTER(Health);

	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UDefaultAttributeSet, MaxHealth);
	GAMEPLAYATTRIBUTE_VALUE_GETTER(MaxHealth);
	GAMEPLAYATTRIBUTE_VALUE_SETTER(MaxHealth);
	GAMEPLAYATTRIBUTE_VALUE_INITTER(MaxHealth);

	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UDefaultAttributeSet, MoveSpeed);
	GAMEPLAYATTRIBUTE_VALUE_GETTER(MoveSpeed);
	GAMEPLAYATTRIBUTE_VALUE_SETTER(MoveSpeed);
	GAMEPLAYATTRIBUTE_VALUE_INITTER(MoveSpeed);

	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(UDefaultAttributeSet, Damage);
	GAMEPLAYATTRIBUTE_VALUE_GETTER(Damage);
	GAMEPLAYATTRIBUTE_VALUE_SETTER(Damage);
	GAMEPLAYATTRIBUTE_VALUE_INITTER(Damage);
};
