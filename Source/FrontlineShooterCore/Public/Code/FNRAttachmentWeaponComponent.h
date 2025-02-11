//  Wise Labs: Gameworks (c) 2020-2024

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FNRAttachmentWeaponComponent.generated.h"

enum class EAttachmentType : uint8;
class AFNRAttachment;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FRONTLINESHOOTERCORE_API UFNRAttachmentWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFNRAttachmentWeaponComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

#pragma region Variables
protected:
	FTimerHandle ReassignAddAttachmentHandle;
	
	UPROPERTY(BlueprintReadOnly, Category = "References", Replicated)
	class AFNRFireWeapon* Weapon = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachment System")
	TArray<TSubclassOf<AFNRAttachment>> InitialAttachments;
	
	UPROPERTY(BlueprintReadOnly, Category = "Attachment System", Replicated)
	TArray<AFNRAttachment*> CurrentAttachments;

public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attachment System")
	TArray<EAttachmentType> CompatibleAttachments;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attachment System")
	TMap<EAttachmentType, FName> WidgetAttachmentsLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachment System|Ammo")
	TMap<UStaticMesh*, FTransform> UpgradeAmmoMeshes;

#pragma endregion Variables

#pragma region Functions
	
#pragma region Blueprint Callable
public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attachement System")
	AFNRAttachment* GetAttachmentByType(const EAttachmentType& AttachmentType) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attachement System")
	TArray<AFNRAttachment*> GetAttachments() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attachement System")
	bool WeaponContainsAttachment(AFNRAttachment* Attachment) const;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attachement System")
	bool WeaponContainsAttachmentByClass(const TSubclassOf<AFNRAttachment> Attachment) const;
	
	UFUNCTION(BlueprintCallable, Category = "Attachment System")
	bool AddAttachment(const TSubclassOf<AFNRAttachment> AttachmentToAdd, const bool bRemoveFromInventory = true);

	UFUNCTION(BlueprintCallable, Category = "Attachment System")
	bool RemoveAttachment(AFNRAttachment* AttachmentToRemove, const bool bDropOnRemove = false);

	UFUNCTION(BlueprintCallable, Category = "Attachment System")
	bool RemoveAttachmentByClass(const TSubclassOf<AFNRAttachment> AttachmentToRemove, const bool bDropOnRemove = false);

	UFUNCTION(BlueprintCallable, Category = "Attachment System")
	void UpdateAttachment(const TSubclassOf<AFNRAttachment> AttachmentClass, const bool bForceRemoveAttachment = false, const bool bDropInsteadRemove = false);
#pragma endregion

#pragma region Replicated Functions
private:
	UFUNCTION(Server, Reliable)
	void AddAttachment_Server(TSubclassOf<AFNRAttachment> AttachmentToAdd, bool bRemoveFromInventory);

	UFUNCTION(Server, Reliable)
	void RemoveAttachment_Server(AFNRAttachment* AttachmentToRemove, bool bDropOnRemove);

	UFUNCTION(Server, Reliable)
	void DropAttachment(TSubclassOf<AFNRAttachment> AttachmentToRemove, const FTransform& Transform);
	
#pragma endregion

#pragma region CPP Only Functions
private:
	void Internal_AddAttachment(AFNRAttachment* Attachment, const TSubclassOf<AFNRAttachment>& AttachmentToAdd, const bool bRemoveFromInventory);
#pragma endregion
	
#pragma endregion Functions
};
