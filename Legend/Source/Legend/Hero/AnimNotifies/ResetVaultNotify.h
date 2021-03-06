// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "ResetVaultNotify.generated.h"

/**
 * 
 */
UCLASS()
class LEGEND_API UResetVaultNotify : public UAnimNotify
{
	GENERATED_BODY()
	
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

public:
	// If true, vault type is "over". If false, vault type is "onto"
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vaulting")
		bool bIsVaultOver;
};
