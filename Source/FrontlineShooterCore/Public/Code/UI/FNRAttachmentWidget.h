// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FNRAttachmentWidget.generated.h"

UCLASS()
class FRONTLINESHOOTERCORE_API UFNRAttachmentWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Info")
	TObjectPtr<class AFNRAttachment> Interactable;
};
