// All rights reserved Wise Labs �

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/FSCTypes.h"
#include "FNRAttachmentSwitch.generated.h"

/**
 * 
 */
UCLASS()
class FRONTLINESHOOTERCORE_API UFNRAttachmentSwitch : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateInfo(class UFNRWeaponComponent* WeaponComponent, const EAttachmentType& AttachmentType);
};
