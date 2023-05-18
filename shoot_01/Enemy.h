// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BulletHitInterface.h"

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"

UCLASS()
class SHOOT_01_API AEnemy : public ACharacter, public IBulletHitInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent)
	void ShowHealthBar();
	//ShowHealthBar_Implementation(),是ShowHealthBar();对应在C++中的实现。
	void ShowHealthBar_Implementation();

	UFUNCTION(BlueprintImplementableEvent)
	void HideHealthBar();

	void Die();

	void PlayHitMontage(FName Section, float PlayRate = 1.0f);

	void ResetHitReactTimer();

	UFUNCTION(BlueprintCallable)
	void StoreHitNumber(UUserWidget* HitNumber, FVector Location);

	UFUNCTION()
	void DestroyHitNumber(UUserWidget* HitNumber);

	void UpdateHitNumbers();


	/** Called when somthing overlaps with the agro sphere */
	UFUNCTION()
	void AgroSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	//UFUNCTION(BlueprintCallable)
	UFUNCTION(BlueprintCallable)
	void SetStunned(bool Stunned);

	/** Called when somthing overlaps with the CombatRange sphere */
	UFUNCTION()
		void CombatRangeOverlap(
			UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex,
			bool bFromSweep,
			const FHitResult& SweepResult);
	/** Called when End Overlapping AreaSphere*/
	UFUNCTION()
		void CombatRangeEndOverlap(
			UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	void PlayAttackMontage(FName Section, float PlayRate);

	UFUNCTION(BlueprintPure)
	FName GetAttackSectionName();

	UFUNCTION()
	void OnLeftWeaponOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,
							int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult);
	UFUNCTION()
	void OnRightWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
							int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Activate/deactivate collision for weapon boxs
	UFUNCTION(BlueprintCallable)
	void ActivateLeftWeapon();
	UFUNCTION(BlueprintCallable)
	void DeactivateLeftWeapon();
	UFUNCTION(BlueprintCallable)
	void ActivateRightWeapon();
	UFUNCTION(BlueprintCallable)
	void DeactivateRightWeapon();
	
	void DoDamage(class AShooterCharacter* Victim);
	void SpawnBlood(AShooterCharacter* Victim,FName SocketName);

	// Attempt to stun character
	void StunCharacter(AShooterCharacter* Victim);

	void ResetCanAttack();

	UFUNCTION(BlueprintCallable)
	void FinishDeath();

	void DestroyEnemy();

private:

	FTimerHandle DeathTimer;
	/** Time after death until Destroy  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float DeathTime;
	bool bDying;

	/** Death anim montage for the enemy */
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* DeathMontage;

	/** Minimum wait time between attacks */
	UPROPERTY(VisibleAnywhere, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bCanAttack;

	FTimerHandle AttackWaitTimer;

	/** Minimum wait time between attacks */
	UPROPERTY(EditAnywhere, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float AttackWaitTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FName LeftWeaponSocket;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FName RightWeaponSocket;

	/** Base damage for enemy */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float BaseDamage;

	/** Collision volume for the left weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* LeftWeaponCollision;

	/** Collision volume for the Right weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	 UBoxComponent* RightWeaponCollision;

	/** Montage containing different attacks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* AttackMontage;
	/** The four attack montage section names */
	FName AttackLFast;
	FName AttackRFast;
	FName AttackL;
	FName AttackR;

	/** True when in attack range; time to attack!*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bInAttackRange;
	/** Sphere for attack range */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* CombatRangeSphere;

	/** Overlap sphere for when the enemy becomes hostile(敌对的) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* AgroSphere;

	/** True when palying the get hit animation */
	UPROPERTY(visibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bStunned;

	/** Chance of being stunned. 0:no stun chance, 1:100% stun chance  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float StunChance;   //Stun机率

	/** Point for the enemy to move to*/
	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
	FVector PatrolPoint;

	/** Second Point for the enemy to move to*/
	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
	FVector PatrolPoint2;

	class AEnemyAIController* EnemyController;

	/** Behavior tree for the AI Character */
	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true"))
	class UBehaviorTree* BehaviorTree;

	/** Map to store HitNumber widgets and their hit locations*/
	UPROPERTY(VisibleAnywhere, Category = Combat, meta = (AllowPrivateAccess = "true"))
		TMap<UUserWidget*, FVector> HitNumbers;

	/** Time before a HitNumber is removed form the screen */
	UPROPERTY(EditAnywhere, Category = Combat, meta = (AllowPrivateAccess = "true"))
		float HitNumberDestroyTime;

	/** Montage containing Hit and Death animations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HitMontage;

	FTimerHandle HitReactTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HitReactTimeMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HitReactTimeMax;

	bool bCanHitReact;

	/** Particles to spawn when hit by bullets */
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* ImpactParticles;

	/** Sound to spawn when hit by bullets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* ImpactSound;

	/** Current health of the enemy*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
		float Health;
	/** Maximum health of the enemy*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
		float MaxHealth;

	/** Name of the head bone */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
		FString HeadBone;

	/** Time t display health bar once shot*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
		float HealthBarDisplayTime;
	FTimerHandle HealthBarTimer;

public:	

	FORCEINLINE UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void  BulletHit_Implementation(FHitResult HitResult, AActor* Shooter, AController* ShooterController) override;

	//继承自AActor类
	virtual float TakeDamage(float DamageAmount,
		struct FDamageEvent const & DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser) override;

	FORCEINLINE FString GetHeadBone() const { return HeadBone; }

	UFUNCTION(BlueprintImplementableEvent) //在蓝图实现
	void ShowHitNumber(int32 Damage,FVector HitLLocation, bool bHeadShot);


};
