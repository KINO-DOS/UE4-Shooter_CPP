// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "particles/ParticleSystemComponent.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "EnemyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "ShooterCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Explosive.h"
#include "ShooterPlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values
AEnemy::AEnemy() :
	Health(100.f),
	MaxHealth(100.f),
	HealthBarDisplayTime(4.f),
	bCanHitReact(true),
	HitReactTimeMin(.5f), 
	HitReactTimeMax(3.f),
	HitNumberDestroyTime(1.5f),
	bStunned(false),
	StunChance(0.5f),
	AttackLFast(TEXT("AttackLFast")),
	AttackRFast(TEXT("AttackRFast")),
	AttackL(TEXT("AttackL")),
	AttackR(TEXT("AttackR")),
	AttackDouble(TEXT("DoublePain")),
	BaseDamage(20.f),
	LeftWeaponSocket(TEXT("FX_Trail_L_01")),
	RightWeaponSocket(TEXT("FX_Trail_R_01")),
	AttackWaitTime(1.f),
	bDying(false),
	DeathTime(4.f),
	ExpImpulseRate(100000),
	IsGuxMolten(false),
	CanSetSpeed(true)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create the Agro Sphere
	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	AgroSphere->SetupAttachment(GetRootComponent());

	//Create the Combat Range Sphere
	CombatRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatRange"));
	CombatRangeSphere->SetupAttachment(GetRootComponent());

	// Create left and right weapon collision boxes
	LeftWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Left Weapon Box"));
	LeftWeaponCollision->SetupAttachment(GetMesh(),FName("LeftWeaponBone"));
	RightWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Right Weapon Box"));
	RightWeaponCollision->SetupAttachment(GetMesh(), FName("RightWeaponBone"));
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	AgroSphere->OnComponentBeginOverlap.AddDynamic(
		this,
		&AEnemy::AgroSphereOverlap);

	CombatRangeSphere->OnComponentBeginOverlap.AddDynamic(
		this,
		&AEnemy::CombatRangeOverlap);
	CombatRangeSphere->OnComponentEndOverlap.AddDynamic(
		this,
		&AEnemy::CombatRangeEndOverlap);
	// Bind functions to overlap events for weapon boxes
	LeftWeaponCollision->OnComponentBeginOverlap.AddDynamic(
		this,
		&AEnemy::OnLeftWeaponOverlap);
	RightWeaponCollision->OnComponentBeginOverlap.AddDynamic(
		this,
		&AEnemy::OnRightWeaponOverlap);
	// set collison presets for weapon boxes
	LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftWeaponCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	LeftWeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	LeftWeaponCollision->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightWeaponCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	RightWeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	RightWeaponCollision->SetCollisionResponseToChannel(
		ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	/** Collision Set */
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	//Ignnore the camera for Mesh and Capsule
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	//Get AI Controller
	EnemyController = Cast<AEnemyAIController>(GetController());

	if (EnemyController)
	{
		EnemyController->GetBlackBoardComponent()->SetValueAsBool(FName("CanAttack"), true);
	}


	const FVector WorldPatrolPoint = UKismetMathLibrary::TransformLocation(
		GetActorTransform(),
		PatrolPoint);

	const FVector WorldPatrolPoint2 = UKismetMathLibrary::TransformLocation(
		GetActorTransform(),
		PatrolPoint2);

	DrawDebugSphere(
		GetWorld(),
		WorldPatrolPoint,
		25.f,
		12,
		FColor::Red,
		true);

	DrawDebugSphere(
		GetWorld(),
		WorldPatrolPoint2,
		25.f,
		12,
		FColor::Green,
		true);

	if (EnemyController)
	{
		EnemyController->GetBlackBoardComponent()->SetValueAsVector(TEXT("PatrolPoint"), WorldPatrolPoint);

		EnemyController->GetBlackBoardComponent()->SetValueAsVector(TEXT("PatrolPoint2"), WorldPatrolPoint2);

		EnemyController->RunBehaviorTree(BehaviorTree);

		
	}
	
}


void AEnemy::ShowHealthBar_Implementation()
{
	GetWorldTimerManager().ClearTimer(HealthBarTimer);
	GetWorldTimerManager().SetTimer(HealthBarTimer,
		this,
		&AEnemy::HideHealthBar,
		HealthBarDisplayTime);
}

void AEnemy::Die(AController* Controler)
{
	if (bDying) return;
	bDying = true;

	// set collison presets for weapon boxes
	LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftWeaponCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	LeftWeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	//LeftWeaponCollision->SetCollisionResponseToChannel(
	//	ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightWeaponCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	RightWeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	//RightWeaponCollision->SetCollisionResponseToChannel(
	//	ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HideHealthBar();

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);
	}
	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("Dead"), true);
		EnemyController->StopMovement();
	}

	AShooterPlayerController* PlayerControler = Cast<AShooterPlayerController>(Controler);
	if (PlayerControler)
	{	
		PlayerControler->AddPlayerScore(GetDeadScore());
	}

}

void AEnemy::PlayHitMontage(FName Section, float PlayRate)
{
	if (bCanHitReact)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(HitMontage, PlayRate);
			AnimInstance->Montage_JumpToSection(Section, HitMontage);
		}
		bCanHitReact = false;
		const float HitReactTime{ FMath::FRandRange(HitReactTimeMin,HitReactTimeMax) };
		GetWorldTimerManager().SetTimer(
			HitReactTimer,
			this,
			&AEnemy::ResetHitReactTimer,
			HitReactTime);
	}

}



void AEnemy::StoreHitNumber(UUserWidget* HitNumber, FVector Location)
{
	HitNumbers.Add(HitNumber, Location);

	FTimerHandle HitNumberTimer;
	FTimerDelegate HitNumberDelegate;
	HitNumberDelegate.BindUFunction(this, FName("DestroyHitNumber"), HitNumber);
	GetWorld()->GetTimerManager().SetTimer(
		HitNumberTimer,
		HitNumberDelegate,
		HitNumberDestroyTime,
		false);
}

void AEnemy::DestroyHitNumber(UUserWidget* HitNumber)
{
	HitNumbers.Remove(HitNumber);
	HitNumber->RemoveFromParent();
}

void AEnemy::UpdateHitNumbers()
{
	for (auto& HitPair : HitNumbers)
	{
		UUserWidget* HitNumber{ HitPair.Key };
		const FVector Location{ HitPair.Value };
		FVector2D ScreenPosition;
		UGameplayStatics::ProjectWorldToScreen(
			GetWorld()->GetFirstPlayerController(),
			Location,
			ScreenPosition);
		HitNumber->SetPositionInViewport(ScreenPosition);
	}
}

void AEnemy::SetStunned(bool Stunned)
{
	bStunned = Stunned;
	if (EnemyController)
	{
		EnemyController->GetBlackBoardComponent()->SetValueAsBool(TEXT("Stunned"), Stunned);
	}
}

void AEnemy::AgroSphereOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,
								int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult)
{
	if (OtherActor == nullptr)return;
	auto Character = Cast<AShooterCharacter>(OtherActor);
	if (Character)
	{
		if (EnemyController)
		{
			if (EnemyController->GetBlackboardComponent())
			{
				//Set the value of the Target Blackboard Key
				EnemyController->GetBlackboardComponent()->SetValueAsObject(
					TEXT("Target"),
					Character);

				if (CanSetSpeed)
				{
					GetCharacterMovement()->MaxWalkSpeed *= 2;
					CanSetSpeed = false;
				}

			}
		}

	}
}



void AEnemy::CombatRangeOverlap(UPrimitiveComponent* OverlappedComponent,	AActor* OtherActor,UPrimitiveComponent* OtherComp,
								int32 OtherBodyIndex,bool bFromSweep,const FHitResult& SweepResult)
{
	if (OtherActor == nullptr) return;
	auto ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
	if (ShooterCharacter)
	{
		bInAttackRange = true;
		if (EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), true);
		}
	}


}


void AEnemy::ResetHitReactTimer()
{
	bCanHitReact = true;

}

void AEnemy::ResetCanAttack()
{
	bCanAttack = true;
	if (EnemyController)
	{
		EnemyController->GetBlackBoardComponent()->SetValueAsBool(FName("CanAttack"), true);
	}
}

void AEnemy::FinishDeath()
{
	GetMesh()->bPauseAnims = true;
	GetWorldTimerManager().SetTimer(DeathTimer, this, &AEnemy::DestroyEnemy, DeathTime);
}

void AEnemy::DestroyEnemy()
{
	Destroy();
}

void AEnemy::SetEndAttack(bool IsEnd)
{
	if (EnemyController)
	{
		EnemyController->GetBlackBoardComponent()->SetValueAsBool(FName("EndAttack"), IsEnd);
	}
}


void AEnemy::PlayAttackMontage(FName Section, float PlayRate)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		AnimInstance->Montage_Play(AttackMontage);
		AnimInstance->Montage_JumpToSection(Section, AttackMontage);
	}
	bCanAttack = false;
	GetWorldTimerManager().SetTimer(
		AttackWaitTimer, 
		this,
		&AEnemy::ResetCanAttack,
		AttackWaitTime
	);
	if (EnemyController)
	{
		EnemyController->GetBlackBoardComponent()->SetValueAsBool(FName("CanAttack"),false);
	}
}

FName AEnemy::GetAttackSectionName()
{
	FName SectionName;
	const int32 section{ FMath::RandRange(1,4) };
	switch (section)
	{
	case 1:
		SectionName = AttackLFast;
		break;
	case 2:
		SectionName = AttackRFast;
		break;
	case 3:
		if (IsGuxMolten)
		{
			SectionName = AttackDouble;
		}
		else
		{
			SectionName = AttackL;
		}	
		break;
	case 4:
		SectionName = AttackR;
		break;
	}
	return SectionName;
}


void AEnemy::DoDamage(AShooterCharacter* Victim)//(AActor* Victim)
{
	if (Victim == nullptr) return;
	if (Victim->GetMeleeImpactSound())
	{
		UGameplayStatics::PlaySoundAtLocation(this,
			Victim->GetMeleeImpactSound(),
			GetActorLocation());
	}

	UGameplayStatics::ApplyDamage(Victim, BaseDamage,
		EnemyController, this,
		UDamageType::StaticClass());

}

void AEnemy::SpawnBlood(AShooterCharacter* Victim, FName SocketName)
{
	const USkeletalMeshSocket* TipSocket{ GetMesh()->GetSocketByName(SocketName) };
	if (TipSocket)
	{
		const FTransform SocketTransform{ TipSocket->GetSocketTransform(GetMesh()) };
		if (Victim->GetBloodParticles())
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				Victim->GetBloodParticles(),
				SocketTransform);
		}
	}
}

void AEnemy::StunCharacter(AShooterCharacter* Victim)
{
	//return;
	if (Victim)
	{
		const float Stun{ FMath::FRandRange(0.f,1.f) };
		if (Stun <= Victim->GetStunChance())
		{
			Victim->Stun();
		}
	}
}


void AEnemy::OnLeftWeaponOverlap(UPrimitiveComponent* OverlappedComponent, 
								AActor* OtherActor, UPrimitiveComponent* OtherComp, 
								int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
	auto Character = Cast<AShooterCharacter>(OtherActor);
	if (Character)
	{
		DoDamage(Character);
		SpawnBlood(Character, LeftWeaponSocket);
		StunCharacter(Character);
	}
	
}

void AEnemy::OnRightWeaponOverlap(UPrimitiveComponent* OverlappedComponent,
								AActor* OtherActor, UPrimitiveComponent* OtherComp, 
								int32 OtherBodyIndex,bool bFromSweep, const FHitResult& SweepResult)
{
	
	auto Character = Cast<AShooterCharacter>(OtherActor);
	if (Character)
	{
		DoDamage(Character);
		SpawnBlood(Character, RightWeaponSocket);
		StunCharacter(Character);
	}

}


void AEnemy::ActivateLeftWeapon()
{
	LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}




void AEnemy::DeactivateLeftWeapon()
{
	LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
void AEnemy::ActivateRightWeapon()
{
	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}
void AEnemy::DeactivateRightWeapon()
{
	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


void AEnemy::CombatRangeEndOverlap(UPrimitiveComponent* OverlappedComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,
									int32 OtherBodyIndex)
{
	if (OtherActor == nullptr) return;
	auto ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
	if (ShooterCharacter)
	{
		bInAttackRange = false;
		if (EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), false);
		}
	}
}



// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateHitNumbers();
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::BulletHit_Implementation(FHitResult HitResult, AActor* Shooter, AController* ShooterController)
{
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, HitResult.Location, FRotator(0.f), true);
	}



	if (bDying) return;

	ShowHealthBar();
	//Determine whether bullet hit stuns
	const float Stunned = FMath::FRandRange(0.f, 1.f);
	if (Stunned <= StunChance)
	{
		// Stun the Enemy
		PlayHitMontage(FName("HitReactFront"));

		//bStunned = true;
		SetStunned(true);
	}
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{

	//Set the value of the Target Blackboard Key
	ACharacter* Character = EventInstigator->GetCharacter();
	AShooterCharacter* Shooter = Cast<AShooterCharacter>(Character);
	if (Shooter)
	{
		if(!Shooter->IsCombatState)
		{
			TArray<AActor*> Actors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemy::StaticClass(), Actors);
			for (AActor* PawnActor : Actors)
			{
				AEnemy* Enemy = Cast<AEnemy>(PawnActor);
				if (Enemy)
				{
					Enemy->EnemyController->GetBlackboardComponent()->SetValueAsObject(
						TEXT("Target"),
						Shooter);

					if (CanSetSpeed)
					{
						Enemy->GetCharacterMovement()->MaxWalkSpeed *= 2;
						CanSetSpeed = false;
					}					
				}
			}

			Shooter->IsCombatState = true;
		}	
	}


	//Set the Target Blackboard Key to agro the Character
	//if (Cast<AShooterCharacter>(DamageCauser))
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("IsShooter"));
	//	if (EnemyController)
	//	{
	//		EnemyController->GetBlackBoardComponent()->SetValueAsObject(FName("Target"), DamageCauser);
	//	}
	//	
	//}
	if(Cast<AExplosive>(DamageCauser) )
	{
		UE_LOG(LogTemp, Warning, TEXT("IsExplosive"));
		AExplosive* Explosive = Cast<AExplosive>(DamageCauser);
		FVector ImpactVector = GetActorLocation() - Explosive->GetActorLocation();

		FVector ExpDirection(0);
		float ExpDistance;
		ImpactVector.ToDirectionAndLength(ExpDirection, ExpDistance);
		ExpDirection.Z = 0.2;
		ExpDirection *= ExpImpulseRate;
		UE_LOG(LogTemp, Warning, TEXT("ExpDirectio.Y = %f , ExpDirectio.X = %f , ExpDirectio.Z = %f"), ExpDirection.Y, ExpDirection.X, ExpDirection.Z);

	
		if (!IsGuxMolten)
		{
			USkeletalMeshComponent* EnemyMesh = GetMesh();
			EnemyMesh->SetSimulatePhysics(true);
			EnemyMesh->SetEnableGravity(true);  //����	'
			//EnemyMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			EnemyMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

			GetMesh()->AddImpulse(ExpDirection, FName(TEXT("root")));
			EnemyMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			EnemyMesh->SetCollisionResponseToChannel(
				ECollisionChannel::ECC_WorldStatic,
				ECollisionResponse::ECR_Block);

			Health = 0.f;

			Die(EventInstigator);
		}
		else
		{
			if (Health - DamageAmount <= 0.f)
			{
				Health = 0.f;
				Die(EventInstigator);
			}
			else
			{
				PlayHitMontage(FName("HitReactFront"));
				SetStunned(true);
				Health -= DamageAmount;
				ShowHealthBar();
			}
		}


		return DamageAmount;
	}

	if(Health - DamageAmount <= 0.f)
	{
		Health = 0.f;
		Die(EventInstigator);
		////PlayHitMontage(FName("DeathA"));
	}
	else
	{
		//PlayHitMontage(FName("HitReactFront"));
		//SetStunned(true);
		Health -= DamageAmount;
	}

	//if (bDying) return 0;

	//ShowHealthBar();
	////Determine whether bullet hit stuns
	//const float Stunned = FMath::FRandRange(0.f, 1.f);
	//if (Stunned <= StunChance)
	//{
	//	// Stun the Enemy
	//	PlayHitMontage(FName("HitReactFront"));

	//	//bStunned = true;
	//	SetStunned(true);
	//}


	return DamageAmount;
}



