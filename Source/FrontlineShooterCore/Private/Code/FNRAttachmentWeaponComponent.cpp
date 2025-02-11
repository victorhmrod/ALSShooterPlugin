//  Wise Labs: Gameworks (c) 2020-2024

#include "Code/FNRAttachmentWeaponComponent.h"

#include "FrontlineShooterCore.h"
#include "Code/FNRAttachment.h"
#include "Code/FNRFireWeapon.h"
#include "Core/FNRInventoryComponent.h"
#include "Core/InteractableComponent.h"
#include "Data/FNRAttachmentData.h"
#include "Data/FSCTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"

UFNRAttachmentWeaponComponent::UFNRAttachmentWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	WidgetAttachmentsLocation.Add(EAttachmentType::Aim, "AimWidget");
	WidgetAttachmentsLocation.Add(EAttachmentType::Ammo, "AmmoWidget");
	WidgetAttachmentsLocation.Add(EAttachmentType::Barrel, "BarrelWidget");
	WidgetAttachmentsLocation.Add(EAttachmentType::Stock, "StockWidget");
	WidgetAttachmentsLocation.Add(EAttachmentType::Other, "OtherWidget");
	CompatibleAttachments.Add(EAttachmentType::Aim);
	CompatibleAttachments.Add(EAttachmentType::Ammo);
	CompatibleAttachments.Add(EAttachmentType::Barrel);
	CompatibleAttachments.Add(EAttachmentType::Stock);
}

void UFNRAttachmentWeaponComponent::BeginPlay()
{
	Super::BeginPlay();
	if (InitialAttachments.Num() > 0)
	{
		for (const auto& i : InitialAttachments)
		{
			AddAttachment(i);
		}
	}
	Weapon = Cast<AFNRFireWeapon>(GetOwner());
}

void UFNRAttachmentWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UFNRAttachmentWeaponComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentAttachments);
	DOREPLIFETIME(ThisClass, Weapon);
}

AFNRAttachment* UFNRAttachmentWeaponComponent::GetAttachmentByType(const EAttachmentType& AttachmentType) const
{
	if (CurrentAttachments.Num() > 0)
	{
		for (const auto& a : CurrentAttachments)
		{
			if (a && a->Attachment->AttachmentType == AttachmentType)
			{
				return a;
			}
		}
	}
	return nullptr;
}

TArray<AFNRAttachment*> UFNRAttachmentWeaponComponent::GetAttachments() const
{
	return CurrentAttachments;
}

bool UFNRAttachmentWeaponComponent::WeaponContainsAttachment(AFNRAttachment* Attachment) const
{
	return Attachment ? CurrentAttachments.Contains(Attachment) : false;
}

bool UFNRAttachmentWeaponComponent::WeaponContainsAttachmentByClass(const TSubclassOf<AFNRAttachment> Attachment) const
{
	if (Attachment)
	{
		for (const auto& a : CurrentAttachments)
		{
			if (a->GetClass() == Attachment)
			{
				return true;
			}
		}
	}
	return false;
}

bool UFNRAttachmentWeaponComponent::AddAttachment(const TSubclassOf<AFNRAttachment> AttachmentToAdd,
	const bool bRemoveFromInventory)
{
	if (!IsValid(Weapon->CharacterOwner))
	{
		//UE_LOGFMT(LogFSC, Error, "`{Name}`: 'AttachmentToAdd' don't be added because Weapon->CharacterOwner Owner is invalid, retryig now", UKismetSystemLibrary::GetDisplayName(this));
		const UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().SetTimer(ReassignAddAttachmentHandle, [&, AttachmentToAdd, bRemoveFromInventory]
			{
				AddAttachment(AttachmentToAdd, bRemoveFromInventory);
			}, 1.0f, false);
		}
		return false;
	}
	UE_LOGFMT(LogFSC, Warning, "'AttachmentToAdd' added because Weapon->CharacterOwner Owner is now valid");
	AFNRAttachment* Attachment = AttachmentToAdd.GetDefaultObject();
	if (!IsValid(Attachment))
	{
		UE_LOGFMT(LogFSC, Error, "'AttachmentToAdd' is an invalid class");
		return false;
	}
	if (!CompatibleAttachments.Contains(Attachment->Attachment->AttachmentType))
	{
		UE_LOGFMT(LogFSC, Warning, "'AttachmentToAdd' isn't compatible with weapon type");
		return false;
	}
	switch (Attachment->Attachment->CompatibilityMode)
	{
	case EC_ByWeaponName:
		{
			if (Attachment->Attachment->CompatibleWeaponName == Weapon->GetGeneralData().WeaponName)
			{
				Internal_AddAttachment(Attachment, AttachmentToAdd, bRemoveFromInventory);
				return true;
			}
			UE_LOGFMT(LogFSC, Error, "'AttachmentToAdd' error case ByName");
			break;
		}
	default: case EC_ByAmmoMode:
		{
			if (Attachment->Attachment->CompatibleWeaponAmmoModes.Contains(Weapon->Internal_FireWeaponData.AmmoMode))
			{
				Internal_AddAttachment(Attachment, AttachmentToAdd, bRemoveFromInventory);
				return true;
			}
			UE_LOGFMT(LogFSC, Error, "'AttachmentToAdd' error case ByType");
			break;
		}
	}
	if (const AFNRAttachment* AttachmentSpawned = GetAttachmentByType(Attachment->Attachment->AttachmentType))
	{
		UE_LOGFMT(LogFSC, Warning, "{0} is{1} simulating physics", AttachmentSpawned->GetName(), AttachmentSpawned->AttachmentMesh->IsAnySimulatingPhysics() ? "" : "n't");
	}
	return false;
}

void UFNRAttachmentWeaponComponent::Internal_AddAttachment(AFNRAttachment* Attachment, const TSubclassOf<AFNRAttachment>& AttachmentToAdd, const bool bRemoveFromInventory)
{
	AddAttachment_Server(AttachmentToAdd, bRemoveFromInventory);
	Attachment->AttachmentMesh->SetSimulatePhysics(false);
	Attachment->AttachmentMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Attachment->InteractableComponent->SetInteractionActive(false);
	//Attachment->AttachToComponent(MeshComponent, FAttachmentTransformRules::SnapToTargetIncludingScale, Attachment->Attachment->WeaponSocketToAttach);
	if (IsValid(Weapon->CharacterOwner) && Weapon->CharacterOwner->IsLocallyControlled())
	{
		UGameplayStatics::PlaySound2D(this, AttachmentToAdd.GetDefaultObject()->Attachment->EquipSound.LoadSynchronous());
	}
}

bool UFNRAttachmentWeaponComponent::RemoveAttachment(AFNRAttachment* AttachmentToRemove, const bool bDropOnRemove)
{
	if (AttachmentToRemove && CurrentAttachments.Contains(AttachmentToRemove))
	{
		RemoveAttachment_Server(AttachmentToRemove, bDropOnRemove);
		UE_LOGFMT(LogFSC, Display, "Weapon contains attachment");
		return true;
	}
	UE_LOGFMT(LogFSC, Warning, "Attachment is invalid or weapon don't contains him");
	return false;
}

bool UFNRAttachmentWeaponComponent::RemoveAttachmentByClass(const TSubclassOf<AFNRAttachment> AttachmentToRemove,
	const bool bDropOnRemove)
{
	if (IsValid(AttachmentToRemove))
	{
		bool bSuccess = false;
		for (const auto& a : CurrentAttachments)
		{
			if (IsValid(a) && a->GetClass() == AttachmentToRemove)
			{
				bSuccess = RemoveAttachment(a, bDropOnRemove);
				break;
			}
		}
		if (bSuccess)
		{
			UE_LOGFMT(LogFSC, Display, "Attachment class founded in Weapon Attachments");
		}
		else
		{
			UE_LOGFMT(LogFSC, Error, "Don't found {0} in Weapon Attachments", AttachmentToRemove->GetName());
		}		
		return bSuccess;
	}
	UE_LOGFMT(LogFSC, Error, "Attachment class is invalid");
	return false;
}

void UFNRAttachmentWeaponComponent::UpdateAttachment(const TSubclassOf<AFNRAttachment> AttachmentClass,
	const bool bForceRemoveAttachment, const bool bDropInsteadRemove)
{
	if (IsValid(AttachmentClass))
	{
		if (bForceRemoveAttachment)
		{
			if (!WeaponContainsAttachmentByClass(AttachmentClass))
			{
				if (bDropInsteadRemove)
				{
					DropAttachment(AttachmentClass, Weapon->CharacterOwner->GetActorTransform());
				}
			}
			else
			{
				RemoveAttachmentByClass(AttachmentClass, bDropInsteadRemove);
			}
		}
		else
		{
			if (WeaponContainsAttachmentByClass(AttachmentClass))
			{
				RemoveAttachmentByClass(AttachmentClass, bDropInsteadRemove);
			}
			else
			{
				AddAttachment(AttachmentClass, true);
			}
		}
		return;
	}
	UE_LOGFMT(LogFSC, Error, "Attachment class is invalid");
}

void UFNRAttachmentWeaponComponent::DropAttachment_Implementation(TSubclassOf<AFNRAttachment> AttachmentToRemove,
                                                                  const FTransform& Transform)
{
	UFNRInventoryComponent* Inventory = Weapon->CharacterOwner->GetComponentByClass<UFNRInventoryComponent>();
	if (Inventory && !Inventory->FindItemsByClass(AttachmentToRemove->GetDefaultObject<AFNRAttachment>()->Attachment->ItemClass).IsEmpty())
	{
		UFNRInventoryItem* ItemToRemove = Inventory->FindItemsByClass(AttachmentToRemove->GetDefaultObject<AFNRAttachment>()->Attachment->ItemClass)[0];
		if (ItemToRemove)
		{
			Inventory->RemoveItem(ItemToRemove);
		}
	}
	AFNRAttachment* AttachmentSpawned = GetWorld()->SpawnActorDeferred<AFNRAttachment>(AttachmentToRemove, Transform);
	if (AttachmentSpawned)
	{
		AttachmentSpawned->AttachmentMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AttachmentSpawned->AttachmentMesh->SetSimulatePhysics(true);
		UGameplayStatics::FinishSpawningActor(AttachmentSpawned, Transform);
		AttachmentSpawned->AttachmentMesh->AddRadialImpulse(Weapon->MeshComponent->GetSocketLocation(AttachmentSpawned->Attachment->WeaponSocketToAttach), 32.0f, 50.0f, RIF_Linear);
	}
}

void UFNRAttachmentWeaponComponent::RemoveAttachment_Server_Implementation(AFNRAttachment* AttachmentToRemove,
                                                                           bool bDropOnRemove)
{
	CurrentAttachments.Remove(AttachmentToRemove);
	if (bDropOnRemove)
	{
		if (Weapon->CharacterOwner->IsLocallyControlled())
		{
			UGameplayStatics::PlaySound2D(this, AttachmentToRemove->Attachment->UnequipSound.LoadSynchronous());
		}
		DropAttachment(AttachmentToRemove->GetClass(), AttachmentToRemove->GetActorTransform());
		AttachmentToRemove->Destroy();

		UE_LOGFMT(LogFSC, Display, "Attachment dropped with success");
	}
	else if (UFNRInventoryComponent* Inventory = Weapon->CharacterOwner->GetComponentByClass<UFNRInventoryComponent>())
	{
		if (Weapon->CharacterOwner->IsLocallyControlled())
		{
			UGameplayStatics::PlaySound2D(this, AttachmentToRemove->Attachment->UnequipSound.LoadSynchronous());
		}
		UE_LOGFMT(LogFSC, Display, "Attachment added to player inventory with success");
		Inventory->TryAddItemFromClass(AttachmentToRemove->Attachment->ItemClass);
		AttachmentToRemove->Destroy();
	}
	else
	{
		UE_LOGFMT(LogFSC, Error, "Inventory component is invalid and is marked to don't drop attachment");
	}
}

void UFNRAttachmentWeaponComponent::AddAttachment_Server_Implementation(TSubclassOf<AFNRAttachment> AttachmentToAdd,
                                                                        bool bRemoveFromInventory)
{
	if (!IsValid(Weapon->CharacterOwner))
	{
		UE_LOGFMT(LogFSC, Error, "'AttachmentToAdd' don't be added because Weapon->CharacterOwner Owner is invalid");
		return;
	}
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = Weapon;
	AFNRAttachment* AttachmentSpawned = GetWorld()->SpawnActor <AFNRAttachment>(AttachmentToAdd, Weapon->GetActorTransform(), SpawnParameters);
	if (!IsValid(AttachmentSpawned))
	{
		UE_LOGFMT(LogFSC, Error, "AttachmentToAdd is not valid");
		return;
	}
	if (AFNRAttachment* ExistentAttachment = GetAttachmentByType(AttachmentSpawned->Attachment->AttachmentType))
	{
		if (UFNRInventoryComponent* Inventory = Weapon->CharacterOwner->GetComponentByClass<UFNRInventoryComponent>())
		{
			if (Inventory->TryAddItemFromClass(ExistentAttachment->Attachment->ItemClass).Result != EItemAddResult::IAR_NoItemsAdded)
			{
				UE_LOGFMT(LogFSC, Display, "{0} added to inventory with success", ExistentAttachment->Attachment->ItemClass->GetName());
			}
			else
			{
				UE_LOGFMT(LogFSC, Warning, "Can't add {0} to inventory", ExistentAttachment->Attachment->ItemClass->GetName());
			}
		}
		CurrentAttachments.Remove(ExistentAttachment);
		ExistentAttachment->Destroy();
	}
	CurrentAttachments.AddUnique(AttachmentSpawned);
	UE_LOGFMT(LogFSC, Display, "'AttachmentToAdd' was added to Current Attachments array");
	UFNRInventoryComponent* Inventory = Weapon->CharacterOwner->GetComponentByClass<UFNRInventoryComponent>();
	if (Inventory && bRemoveFromInventory && !Inventory->FindItemsByClass(AttachmentSpawned->Attachment->ItemClass).IsEmpty())
	{
		UFNRInventoryItem* ItemToRemove = Inventory->FindItemsByClass(AttachmentSpawned->Attachment->ItemClass)[0];
		if (IsValid(ItemToRemove))
		{
			Inventory->RemoveItem(ItemToRemove);
		}
	}
	AttachmentSpawned->AttachmentMesh->SetSimulatePhysics(false);
	AttachmentSpawned->AttachmentMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachmentSpawned->InteractableComponent->SetInteractionActive(false);
	AttachmentSpawned->AttachToComponent(Weapon->MeshComponent, FAttachmentTransformRules::SnapToTargetIncludingScale, AttachmentSpawned->Attachment->WeaponSocketToAttach);
	if (!AttachmentSpawned->Attachment->AttachToWeapon)
	{
		AttachmentSpawned->SetActorHiddenInGame(true);
	}
}