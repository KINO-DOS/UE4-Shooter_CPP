// Fill out your copyright notice in the Description page of Project Settings.


#include "Explosive.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "particles/ParticleSystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "ShooterCharacter.h"

// Sets default values
AExplosive::AExplosive() :
	Damage(100.f),
	CanCallBulletHit(true)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ExplosiveMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExplosiveMesh"));
	SetRootComponent(ExplosiveMesh);

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AExplosive::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AExplosive::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AExplosive::BulletHit_Implementation(FHitResult HitResult, AActor* Shooter, AController* ShooterController)
{
	CanCallBulletHit = false;
	if (ExplodeSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());
	}
	if (ExplodeParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplodeParticles,GetActorLocation(), FRotator(0.f), true);//HitResult.Location
	}

	//  Apply Explosive damage
	TArray<AActor*>OverlappingActors;
	GetOverlappingActors(OverlappingActors, AActor::StaticClass());
	for(auto Actor: OverlappingActors)
	{
		UE_LOG(LogTemp, Warning, TEXT("Actor damaged by explosive: %s"), *Actor->GetName());

		//UGameplayStatics::ApplyDamage(
		//	Actor,
		//	Damage,
		//	ShooterController,
		//	Shooter,
		//	UDamageType::StaticClass()
		//);

		if(Cast<ACharacter>(Actor))
		{
			UGameplayStatics::ApplyDamage(
				Actor,
				Damage,
				ShooterController,
				this,
				UDamageType::StaticClass()
			);
		}
		else
		{
			AExplosive* Explosive = Cast<AExplosive>(Actor);
			if (Explosive)
			{
				UE_LOG(LogTemp, Warning, TEXT("GET:explosive: %s"), *Explosive->GetName());
				if (Explosive->CanCallBulletHit)
				{
					Explosive->BulletHit_Implementation(HitResult, Shooter, ShooterController);
				}

			}
		}



	}


	Destroy();
}

