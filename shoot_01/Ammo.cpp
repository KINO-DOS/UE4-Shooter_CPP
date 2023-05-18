// Fill out your copyright notice in the Description page of Project Settings.


#include "Ammo.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "ShooterCharacter.h"


AAmmo::AAmmo():
	AmmoType(EAmmoType::EAT_9mm)
{
	 //Construct the AmmoMesh component and set it as the root
	AmmoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AmmoMesh"));
	SetRootComponent(AmmoMesh);

	GetCollisionBox()->SetupAttachment(GetRootComponent());
	GetPickupWidget()->SetupAttachment(GetRootComponent());
	GetAreaSphere()->SetupAttachment(GetRootComponent());

	AmmoCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AmmoCollisionSphere"));
	AmmoCollisionSphere->SetupAttachment(GetRootComponent());
	AmmoCollisionSphere->SetSphereRadius(50.f);
}

void AAmmo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAmmo::BeginPlay()
{
	Super::BeginPlay();

	AmmoCollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AAmmo::AmmoSphereOverlap);
}

void AAmmo::SetItemProperties(EItemState State)
{
	Super::SetItemProperties(State);

	switch (State)
	{
	case EItemState::EIS_Pickup:

		//Set mesh properties
		AmmoMesh->SetSimulatePhysics(false);
		AmmoMesh->SetVisibility(true);
		AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;

	case EItemState::EIS_Equipped:

		//Set mesh peoperties
		AmmoMesh->SetSimulatePhysics(false);
		AmmoMesh->SetVisibility(true);
		AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;

	case EItemState::EIS_Falling:
		//Set mesh peoperties
		AmmoMesh->SetSimulatePhysics(true);
		AmmoMesh->SetEnableGravity(true);  //重力
		//AmmoMesh->SetVisibility(true);
		AmmoMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AmmoMesh->SetCollisionResponseToChannel(
			ECollisionChannel::ECC_WorldStatic,
			ECollisionResponse::ECR_Block);

		break;

	case EItemState::EIS_EquipInterping:

		//Set mesh peoperties
		AmmoMesh->SetSimulatePhysics(false);
		AmmoMesh->SetEnableGravity(false);
		AmmoMesh->SetVisibility(true);
		AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		//拾取时关闭AmmoCollisionSohere的碰撞，否则会触发AmmoSphereOverlap(）；
		AmmoCollisionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AmmoCollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		break;

	case EItemState::EIS_PickedUp:
		break;
	}
}

void AAmmo::AmmoSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(OtherActor)
	{
		auto OverlappedCharacter = Cast<AShooterCharacter>(OtherActor);
		if (OverlappedCharacter)
		{
			StartItemCurve(OverlappedCharacter);
			AmmoCollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void AAmmo::EnableCustomDepth()
{
	AmmoMesh->SetRenderCustomDepth(true);
}

void AAmmo::DisableCustomDepth()
{
	AmmoMesh->SetRenderCustomDepth(false);
}
