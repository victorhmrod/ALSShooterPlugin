// All rights reserved Wise Labs ®

#pragma once

#include "CoreMinimal.h"
#include "Utils/RbsPickupInterface.h"
#include "Utils/RbsTypes.h"

#include "FNRAttachment.generated.h"

UCLASS()
class FRONTLINESHOOTERCORE_API AFNRAttachment : public AActor, public IRbsPickupInterface
{
	GENERATED_BODY()

#pragma region Components
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> AttachmentMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<class UInteractableComponent> InteractableComponent;
	
#pragma endregion
	
#pragma region Variables
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	TObjectPtr<class UFNRAttachmentData> Attachment;
	
	//Variable that handles and stores the result of the operation to add item to the inventory.
	UPROPERTY(BlueprintReadOnly, Category="Info|Inventory")
	FItemAddResult ItemAddResult;

#pragma endregion
	
#pragma region Unreal Defaults
public:
	// Sets default values for this actor's properties
	AFNRAttachment();

	virtual void BeginPlay() override;

	virtual void PostInitializeComponents() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

#pragma endregion

#pragma region CPP Only
private:
	UFUNCTION()
	void OnInteract(class UInteractorComponent* Interactor, UInteractableComponent* Interactable);
public:	
	virtual void OnDropItem_Implementation() override;
	
#pragma endregion

#pragma region Blueprint Exposed
	UFUNCTION(BlueprintCallable, Category = "Attachment")
	bool TryAddAttachment(const AActor* Character);

	UFUNCTION(Server, Reliable)
	void Server_TryAddAttachment(const AActor* Character);
	
#pragma endregion
};
