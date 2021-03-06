// Fill out your copyright notice in the Description page of Project Settings.

#include "Legend/Hero/Hero.h"
#include "Legend/Weapons/Weapon.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Legend/Hero/Components/CombatComponent.h"

#pragma region General
UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	Hero = Cast<AHero>(GetOwner());
	MovementComp = Hero->FindComponentByClass<UCharacterMovementComponent>();

	Weapon = GetWorld()->SpawnActor<AWeapon>(WeaponClass, FTransform::Identity);
	if (Weapon) {
		Weapon->SetOwner(Hero);
		Weapon->AttachToComponent(
			Hero->GetMesh(), 
			FAttachmentTransformRules::SnapToTargetNotIncludingScale, 
			"WeaponUnequipped"
		);
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
#pragma endregion


bool UCombatComponent::TryAttack() {

	if (!bWeaponEquipped) {
		EquipWeapon();
		return false;
	}

	return false;
}


#pragma region Equip / Unequip
void UCombatComponent::ToggleEquipped() {
	if (bWeaponEquipped)
		UnequipWeapon();
	else
		EquipWeapon();
}

void UCombatComponent::EquipWeapon() {

	if (MovementComp->IsFalling())
		return;

	bWeaponEquipped = true;
	Hero->UpdateMaxWalkSpeed();

	MovementComp->bOrientRotationToMovement = false;
	Hero->bUseControllerRotationYaw = true;

	Hero->PlayAnimMontage(EquipMontage);
}

void UCombatComponent::UnequipWeapon() {

	if (MovementComp->IsFalling())
		return;

	bWeaponEquipped = false;
	Hero->UpdateMaxWalkSpeed();

	MovementComp->bOrientRotationToMovement = true;
	Hero->bUseControllerRotationYaw = false;

	Hero->PlayAnimMontage(UnequipMontage);
}

void UCombatComponent::EquipWeaponCallback(USkeletalMeshComponent* Mesh) {
	Weapon->AttachToComponent(
		Hero->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		"WeaponEquipped"
	);
}

void UCombatComponent::UnequipWeaponCallback(USkeletalMeshComponent* Mesh) {
	Weapon->AttachToComponent(
		Hero->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		"WeaponUnequipped"
	);
}
#pragma endregion

