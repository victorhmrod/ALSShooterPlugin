// All rights reserved Wise Labs ®

#include "Code/FNRAttachment.h"

#include "FrontlineShooterCore.h"
#include "Code/FNRAttachmentWeaponComponent.h"
#include "Code/FNRFireWeapon.h"
#include "Code/FNRWeaponComponent.h"
#include "Core/InteractableComponent.h"
#include "Core/InteractorComponent.h"
#include "Core/FNRInventoryComponent.h"
#include "Data/FNRAttachmentData.h"
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

	if (!IsValid(Attachment))
	{
		UKismetSystemLibrary::PrintString(this, 
			"Você esqueceu de colocar o Data Asset", 
			true, true, FLinearColor::Blue, 10.f);
		return;
	}

	UStaticMesh* LoadedMesh = Attachment->Mesh.LoadSynchronous();
	if (IsValid(LoadedMesh))
	{
		AttachmentMesh->SetStaticMesh(LoadedMesh);
	}

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
	UFNRWeaponComponent* WeaponComponent = Character->GetComponentByClass<UFNRWeaponComponent>();
	if (WeaponComponent && WeaponComponent->EquippedFireWeapon)
	{
		AFNRFireWeapon* FoundWeapon = WeaponComponent->EquippedFireWeapon;
		if (FoundWeapon->AttachmentComponent->AddAttachment(GetClass(), true))
		{
			Destroy(true);
			UE_LOGFMT(LogFSC, Display, "{0} added to {1} with success", GetClass()->GetName(), FoundWeapon->GetName());
		}
	}
	UFNRInventoryComponent* InventoryComponent = Character->GetComponentByClass<UFNRInventoryComponent>();
	if (InventoryComponent)
	{
		ItemAddResult = InventoryComponent->TryAddItemFromClass(Attachment->ItemClass, 1);
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
	if (!IsValid(Attachment) || !Character)
	{
		UE_LOGFMT(LogFSC, Warning, "Attachment Data Asset or Character is invalid");
		return false;
	}
	Server_TryAddAttachment(Character);
	return true;
}
