// Fill out your copyright notice in the Description page of Project Settings.

#include "Legend/Hero/Hero.h"
#include "Legend/Hero/Components/VaultComponent.h"
#include "Legend/Hero/AnimNotifies/ResetVaultNotify.h"

void UResetVaultNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) {
	AHero* Hero = Cast<AHero>(MeshComp->GetOwner());

	// Protects against running in the editor
	if (!Hero)
		return;

	UVaultComponent* VaultComp = Hero->FindComponentByClass<UVaultComponent>();

	if (!VaultComp)
		return;

	if (bIsVaultOver)
		VaultComp->StopVaultOver();
	else
		VaultComp->StopVaultOnto();

	return;
}