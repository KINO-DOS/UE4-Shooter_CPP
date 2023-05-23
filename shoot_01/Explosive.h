// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BulletHitInterface.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Explosive.generated.h"

UCLASS()
class SHOOT_01_API AExplosive : public AActor,public IBulletHitInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AExplosive();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Particles to spawn when hit by bullets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
		class UParticleSystem* ExplodeParticles;

	/** Sound to spawn when hit by bullets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
		class USoundCue* ExplodeSound;

	/** Mesh for hte explosive*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* ExplosiveMesh;

	/** Used to determine what Actors overlap during explosion */
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* OverlapSphere;

	/** Damage Amount for explosive*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float Damage;

	
public:	
	bool CanCallBulletHit;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void BulletHit_Implementation(FHitResult HitResult, AActor* Shooter, AController* ShooterController) override;


};
