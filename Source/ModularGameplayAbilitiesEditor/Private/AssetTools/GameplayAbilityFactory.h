// Author: Tom Werner (MajorT), 2025

#pragma once

#include "Factories/BlueprintFactory.h"

#include "GameplayAbilityFactory.generated.h"

// Factory for UGameplayAbility
UCLASS(HideCategories = Object, Config = Editor)
class UGameplayAbilityFactory : public UBlueprintFactory
{
	GENERATED_BODY()

public:
	UGameplayAbilityFactory();

protected:
	//~ Begin UFactory Interface
	virtual FText GetDisplayName() const override;
	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	//~ End UFactory Interface

private:
	UPROPERTY(Config)
	TArray<TObjectPtr<UClass>> CommonAbilityClasses;
};

// Factory for UGameplayEffect
UCLASS(HideCategories = Object, Config = Editor)
class UGameplayEffectFactory : public UBlueprintFactory
{
	GENERATED_BODY()

public:
	UGameplayEffectFactory();

protected:
	//~ Begin UFactory Interface
	virtual FText GetDisplayName() const override;
	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	//~ End UFactory Interface

private:
	UPROPERTY(Config)
	TArray<TObjectPtr<UClass>> CommonEffectClasses;	
};