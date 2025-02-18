// All rights reserved Wise Labs ®

#include "Code/FNRAttachment.h"

#include "FrontlineShooterCore.h"
#include "Code/FNRAttachmentComponent.h"
#include "Code/FNRFireWeapon.h"
#include "Code/FNRWeaponComponent.h"
#include "Core/InteractableComponent.h"
#include "Core/InteractorComponent.h"
#include "Core/FNRInventoryComponent.h"
#include "Data/FNRAttachmentDataAsset.h"
#include "Logging/StructuredLog.h"

AFNRAttachment::AFNRAttachment()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);
	AttachmentMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attachment Mesh"));
	AttachmentMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SetRootComponent(AttachmentMesh);
	InteractableComponent=CreateDefaultSubobject<UInteractableComponent>(TEXT("Interactable Component"));
	InteractableComponent->SetupAttachment(AttachmentMesh);
	InteractableComponent->OnInteract.AddDynamic(this, &ThisClass::OnInteract);
}

void AFNRAttachment::BeginPlay()
{
	Super::BeginPlay();

	if (!IsValid(AttachmentDataAsset))
	{
		UKismetSystemLibrary::PrintString(this, 
			"Você esqueceu de colocar o Data Asset", 
			true, true, FLinearColor::Blue, 10.f);
		return;
	}

	UStaticMesh* LoadedMesh = AttachmentDataAsset->Mesh.LoadSynchronous();
	if (LoadedMesh)
	{
		AttachmentMesh->SetStaticMesh(LoadedMesh);
	}

	InteractableComponent->SetDisplayText(AttachmentDataAsset->Name.ToString());

	InteractableComponent->SetTooltipText(AttachmentDataAsset->Tooltip.ToString());

	AttachmentMesh->SetSimulatePhysics(true);
}

void AFNRAttachment::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AFNRAttachment::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AFNRAttachment::OnInteract(UInteractorComponent* Interactor, UInteractableComponent* Interactable)
{
	TryAddAttachment(Interactor->GetOwner());
}

void AFNRAttachment::OnDropItem_Implementation()
{
	IRbsPickupInterface::OnDropItem_Implementation();

	DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);
	InteractableComponent->SetInteractionActive(true);
	AttachmentMesh->SetHiddenInGame(false);
	AttachmentMesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	AttachmentMesh->SetSimulatePhysics(true);
}

void AFNRAttachment::Server_TryAddAttachment_Implementation(const AActor* Character)
{
	const UFNRWeaponComponent* WeaponComponent = Character->GetComponentByClass<UFNRWeaponComponent>();
	UFNRInventoryComponent* InventoryComponent = Character->GetComponentByClass<UFNRInventoryComponent>();
	if (WeaponComponent && WeaponComponent->EquippedFireWeapon)
	{
		const AFNRFireWeapon* FoundWeapon = WeaponComponent->EquippedFireWeapon;
		if (FoundWeapon->AttachmentComponent->AddAttachment(GetClass(), true))
		{
			Destroy(true);
			UE_LOGFMT(LogFSC, Display, "{0} added to {1} with success", GetClass()->GetName(), FoundWeapon->GetName());
		}
	}
	else if (InventoryComponent)
	{
		ItemAddResult = InventoryComponent->TryAddItemFromClass(AttachmentDataAsset->ItemClass, 1);
		if (ItemAddResult.Result != EItemAddResult::IAR_NoItemsAdded)
		{
			Destroy(true);
			UE_LOGFMT(LogFSC, Display, "{0} added to player inventory", GetClass()->GetName());
		}
		else
		{
			UE_LOGFMT(LogFSC, Warning, "Tried to add {0} to player inventory but failed", GetClass()->GetName());
		}
		UE_LOGFMT(LogFSC, Warning, "Tried to add {0} to player inventory but failed", GetClass()->GetName());
	}
}


bool AFNRAttachment::TryAddAttachment(const AActor* Character)
{
	if (!IsValid(AttachmentDataAsset) || !Character)
	{
		UE_LOGFMT(LogFSC, Warning, "Attachment Data Asset or Character is invalid");
		return false;
	}
	Server_TryAddAttachment(Character);
	return true;
}
