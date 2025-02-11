// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "FNRAnimNotify_ThrowGrenade.generated.h"

/**
 * 
 */
UCLASS()
class FRONTLINESHOOTERCORE_API UFNRAnimNotify_ThrowGrenade : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Info")
	bool bStartThrowing = false;
	
protected:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
