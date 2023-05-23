// Fill out your copyright notice in the Description page of Project Settings.



#include "ShooterCharacter.h"
#include "GameFramework\SpringArmComponent.h"
#include "Camera\CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "Item.h"
#include "Weapon.h"
#include "Components/WidgetComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Ammo.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "shoot_01.h"
//#include "BulletHitInterface.h"
#include "Enemy.h"
#include "EnemyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "WeaponType.h"

// Sets default values 
AShooterCharacter::AShooterCharacter() :
	//base rates for turning
	BaseTurnRate(45.f),
	BaseLookUpRate(45.f),
	//Tren fates for aiming/not aiming
	HipTurnRate(90.f),
	HipLookUpTate(90.f),
	AimingTurnRate(20.f),
	AimingLookUpRate(20.f),
	// Mouse look sensitivity scale factors
	MouseHipTurnRate(1.0f),
	MouseHipLookUpRate(1.0f),
	MouseAimingTurnRate(0.2f),
	MouseAimingLookUpRate(0.2f),
	// true when aiming the weapon
	bAiming(false),
	// Camera field of view values
	CameraDefaultFOV(0.f), // set in BeginPlay
	CameraZoomedFOV(35.f),
	CameraCurrentFOV(0.f),
	ZoomInterpSpeed(20.f),
	//CrosshairSpread
	DefaultSpreadsScale(0.5f),
	//Crosshair spread factors
	CrosshairSpreadMultiplier(0.f),
	CrosshairVelocityFactor(0.f),
	CrosshairInAirFactor(0.f),
	CrosshairAimFactor(0.f),
	CrosshairShootingFactor(0.f),
	// Bullet fire timer variables
	ShootTimeDuration(0.05f),
	bFiringBullet(false),
	bShouldFire(true),
	bFireButtonPressed(false),
	// Item trace variable
	bShouldTraceForItems(false),
	//Camera interp location
	//CameraInterpDistance(250.f),
	CameraInterpDistance(170.f),
	CameraInterpElevation(65.f),
	//Starting ammo amounts
	Starting9mmAmmo(85),
	StartingARAmmo(120),
	// Combat variables
	CombatState(ECombatState::ECS_Unoccupied),
	bCrouching(false),
	//Pickup sound timer peoperties
	bShouldPlayPickupSound(true),
	bShouldPlayEquipSound(true),
	PickupSoundResetTime(0.2f),
	EquipSoundResetTime(0.2f),

	BaseMovementSpeed(650),
	CrouchMovementSpeed(300),

	//CurrentCapsuleHalfHeight(0.0),
	StandingCapsuleHalfHeight(88.f),
	CrouchingCapsuleHalfHeight(44.f),

	BaseGroudFriction(2.f),
	CrouchingGroudFriction(100.f),

	//Icon animation property
	HighlightedSlot(-1),

	Health(100.f),
	MaxHealth(100.f),
	StunChance(0.25f),
	IsCombatState(false)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create a cameraboom (pulls in towards the character if there is a collosion
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 200.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.f, 50.f, 70.f); //(0.f, 50.f, 70.f)
	//CameraBoom->SocketOffset = FVector(0.f, 50.f, 45.f);

	//Creat a follow Camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom,USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; //camera does not rotate relative to arm

	//Don't rotation when the controller rorate. Let the controller only affect the camera.
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	//Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; //Character moves in the direction of inout...
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); //...at this roration rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2;

	// Create Hand Scene Component
	HandSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HandSceneComp"));

	//Create Interpolation Components
	WeaponInterpComp = CreateDefaultSubobject<USceneComponent>(TEXT("Weapon Interpolation Componet"));
	WeaponInterpComp->SetupAttachment(GetFollowCamera());
	
	InterpComp1 = CreateDefaultSubobject<USceneComponent>(TEXT("Weapon Interpolation Componet1"));
	InterpComp1->SetupAttachment(GetFollowCamera());

	InterpComp2 = CreateDefaultSubobject<USceneComponent>(TEXT("Weapon Interpolation Componet2"));
	InterpComp2->SetupAttachment(GetFollowCamera());
	 
	InterpComp3 = CreateDefaultSubobject<USceneComponent>(TEXT("Weapon Interpolation Componet3"));
	InterpComp3->SetupAttachment(GetFollowCamera());

	InterpComp4 = CreateDefaultSubobject<USceneComponent>(TEXT("Weapon Interpolation Componet4"));
	InterpComp4->SetupAttachment(GetFollowCamera());

	InterpComp5 = CreateDefaultSubobject<USceneComponent>(TEXT("Weapon Interpolation Componet5"));
	InterpComp5->SetupAttachment(GetFollowCamera());

	InterpComp6 = CreateDefaultSubobject<USceneComponent>(TEXT("Weapon Interpolation Componet6"));
	InterpComp6->SetupAttachment(GetFollowCamera());
	
}

	float AShooterCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
										AController* EventInstigator, AActor* DamageCauser)
	{
		if (Health <= 0) return 0;

		if (Health - DamageAmount <= 0.f)
		{
			
			GetMesh()->GetAnimInstance()->StopAllMontages(0.0f);
			Health = 0.f;
			Die();

			auto EnemyController = Cast<AEnemyAIController>(EventInstigator);
			if (EnemyController)
			{
				EnemyController->GetBlackBoardComponent()->SetValueAsBool(FName("CharacterDead"), true);
			}
		}
		else
		{
			Health -= DamageAmount;
		}
		return DamageAmount;
	}

	// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (FollowCamera)
	{
		CameraDefaultFOV = GetFollowCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}


	// Spawn the default weapon and and equip it
	SpawnDefaultWeapon();
	//EquipWeapon(SpawnDefaultWeapon());
	//Inventory.Add(EquippedWeapon);
	//EquippedWeapon->SetSlotIndex(0);
	////游戏开始时装备的武器关闭发光材质和边框
	//EquippedWeapon->DisableCustomDepth();
	//EquippedWeapon->DisableGlowMaterial();
	//EquippedWeapon->SetCharacter(this);


	InitializeAmmoMap();

	// Create FinterpLocation structs for each interp location. Add to array
	InitializeInterpLocations();

	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Handle interpolation for zoom when aiming
	CameraInterpZoom(DeltaTime);

	//Change look sensitivity based on aiming
	SetLookRates();

	//Calculate crosshair spread multiplier
	CalculateCrosshairSpread(DeltaTime);

	//Check OverlappedItemCount, then trace for items
	TraceForItems();

	//Interpolate the capsule half height based  on crouching/standing
	InterpCapsuleHalfHeight(DeltaTime);
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);

	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpAtRate);

	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp);
	
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShooterCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	//this->GetVelocity()
	//PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireWeapon);
	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AShooterCharacter::FireButtonReleased);

	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);

	PlayerInputComponent->BindAction("Select", IE_Pressed, this, &AShooterCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction("Select", IE_Released, this, &AShooterCharacter::SelectButtonReleased);

	PlayerInputComponent->BindAction("ReloadButton", IE_Pressed, this, &AShooterCharacter::ReloadButtonPressed);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShooterCharacter::CrouchButtonPressed);

	PlayerInputComponent->BindAction("Fkey", IE_Pressed, this, &AShooterCharacter::FkeyPressed);
	PlayerInputComponent->BindAction("1key", IE_Pressed, this, &AShooterCharacter::OnekeyPressed);
	PlayerInputComponent->BindAction("2key", IE_Pressed, this, &AShooterCharacter::TwokeyPressed);
	PlayerInputComponent->BindAction("3key", IE_Pressed, this, &AShooterCharacter::ThreekeyPressed);
	PlayerInputComponent->BindAction("4key", IE_Pressed, this, &AShooterCharacter::FourkeyPressed);
	PlayerInputComponent->BindAction("5key", IE_Pressed, this, &AShooterCharacter::FivekeyPressed);
}

void AShooterCharacter::Jump()
{
	if (bCrouching)
	{
		bCrouching = false;
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}
	else
	{
		ACharacter::Jump();
	}
}

void AShooterCharacter::InterpCapsuleHalfHeight(float DeltaTime)
{
	float TargetCapsuleHalfHeight{};
	if (bCrouching)
	{
		TargetCapsuleHalfHeight = CrouchingCapsuleHalfHeight;
	}
	else
	{
		TargetCapsuleHalfHeight = StandingCapsuleHalfHeight;
	}
	const float InterpHalfHeight{ FMath::FInterpTo(GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),
		TargetCapsuleHalfHeight,DeltaTime,20.f) };

	//Negative value if crouching; Positive value if stasnding
	const float DeltaCapsuleHalfHeight{ InterpHalfHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight() };
	FVector MeshOffset{ 0.f,0.f,-DeltaCapsuleHalfHeight };
	GetMesh()->AddLocalOffset(MeshOffset);

	GetCapsuleComponent()->SetCapsuleHalfHeight(InterpHalfHeight);
}

void AShooterCharacter::Aim()
{
	bAiming = true;

	//瞄准时移动速度，和Crouch时是相同的，所以使用CrouchMovementSpeed
	GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
}

void AShooterCharacter::StopAiming()
{
	bAiming = false;
	if (!bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}	
}

void AShooterCharacter::ResetPickupSoundTimer()
{
	bShouldPlayPickupSound = true;
}

void AShooterCharacter::ResetEquipSoundTimer()
{
	bShouldPlayEquipSound = true;
}

void AShooterCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		//find out which way forward
		const FRotator Rotation{Controller->GetControlRotation()};
		const FRotator YawRotation{0,Rotation.Yaw,0};

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) };
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::MoveRight(float Value)
{
	//find out which way right
	const FRotator Rotation{ Controller->GetControlRotation() };
	const FRotator YawRotation{ 0,Rotation.Yaw,0 };

	const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) };
	AddMovementInput(Direction, Value);
}

void AShooterCharacter::TurnAtRate(float Rate)
{
	//calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds()); // deg/sec *  sec/frame

	float TurnValue = Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds();
	FString MyMessage = FString::Printf(TEXT("DeltaSecond: %f"), GetWorld()->GetDeltaSeconds());
	FString MyTurn = FString::Printf(TEXT("TurnValue: %f"), TurnValue);
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, MyMessage);
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, MyTurn);


}

void AShooterCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());// deg/sec * sec/frame
}

void AShooterCharacter::Turn(float Value)
{
	float TurnScaleFactor{};
	if (bAiming)
	{
		TurnScaleFactor = MouseAimingTurnRate;
	}
	else
	{
		TurnScaleFactor = MouseHipTurnRate;
	}
	AddControllerYawInput(Value * TurnScaleFactor);
}

void AShooterCharacter::LookUp(float Value)
{
	float LookUpScaleFactor{};
	if (bAiming)
	{
		LookUpScaleFactor = MouseAimingLookUpRate;
	}
	else
	{
		LookUpScaleFactor = MouseHipLookUpRate;
	}
	AddControllerPitchInput(Value * LookUpScaleFactor);
}

void AShooterCharacter::FireWeapon()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Fire!!!"));

	if (EquippedWeapon == nullptr) return;

	if (CombatState != ECombatState::ECS_Unoccupied) return;


	if (WeaponHasAmmo())
	{
		PlayFireSound();
		SendBullet();
		PlayGunfireMontage();

		//Start bullet fire timer for crosshairs
		StartCrosshairBulletFire();
		//Subtract 1 from the Weapon's Ammo
		EquippedWeapon->DecrementAmmo();

		StartFireTimer();

		if (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol)
		{
			//Start moving slide timer
			EquippedWeapon->StartSlideTimer();
		}
	}
}

bool AShooterCharacter::GetBeamEndLoacation(const FVector& MuzzleSocketLocation, FHitResult& OutHitResult)//FVector& OutBeamLocation)
{
	FVector OutBeamLocation;
	//Check for crosshair trace hit
	FHitResult CrosshairHitResult;
	FVector End;
	bool bCrosshairHit = TraceUnderCrosshairs(CrosshairHitResult,OutBeamLocation);

	//Perform asecond trace ,this time from the gun barrel
	//FHitResult WeaponTraceHit;
	const FVector WeaponTraceStart{ MuzzleSocketLocation };
	const FVector WeaponTraceEnd{ OutBeamLocation };
	GetWorld()->LineTraceSingleByChannel(
		//WeaponTraceHit,
		OutHitResult,
		WeaponTraceStart,
		WeaponTraceEnd,
		ECollisionChannel::ECC_Visibility);

	//Tentatice beam location - still need to trace from fgun
	//if (WeaponTraceHit.bBlockingHit)//WeaponTraceHit.bBlockingHit)
	//{
	//	OutBeamLocation = WeaponTraceHit.Location;
	//	return true;
	//}
	/*return false;*/

	if (!OutHitResult.bBlockingHit)
	{
		OutHitResult.Location = OutBeamLocation;
		return false;
	}

	return true;
}

void AShooterCharacter::AimingButtonPressed()
{
	bAimingButtonPressed = true;
	if (CombatState != ECombatState::ECS_Reloading && 
		CombatState != ECombatState::ECS_Equipping &&
		CombatState != ECombatState::ECS_Stunned)
	{
		Aim();
	}
}

void AShooterCharacter::AimingButtonReleased()
{
	bAimingButtonPressed = false;
	StopAiming();
}

void AShooterCharacter::CameraInterpZoom(float DeltaTime)
{
	//Set current camera field of view
	if (bAiming)
	{
		//Interpolate to zoomed FOV
		CameraCurrentFOV = FMath::FInterpTo(
			CameraCurrentFOV,
			CameraZoomedFOV,
			DeltaTime,
			ZoomInterpSpeed);
	}
	else
	{
		//Interpolate to default FOV
		CameraCurrentFOV = FMath::FInterpTo(
			CameraCurrentFOV,
			CameraDefaultFOV,
			DeltaTime,
			ZoomInterpSpeed);
	}
	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);

}

void AShooterCharacter::SetLookRates()
{
	if (bAiming)
	{
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else
	{
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpTate;
	}
}

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	FVector2D WalkSpeedRange{ 0.f,600.f };
	FVector2D VelocityMultiplierRange{ 0.f,1.f };
	FVector Velocity{ GetVelocity() };
	Velocity.Z = 0.f;

	//Calculate crosshair Velocity factor
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(
		WalkSpeedRange,
		VelocityMultiplierRange,
		Velocity.Size());

	// Calculate crosshair in air factor
	if (GetCharacterMovement()->IsFalling()) //is in air?
	{
		//  Sperad the Crosshairs slowly while in air
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25);
	}
	else // Character is on the ground
	{
		// Shrink the crosshairs rapidly while on the ground
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
	}

	//Calculate crosshair aim factor
	if (bAiming) //Are we aiming?
	{
		// Shrink crosshairs a small amount very quickly
		CrosshairAimFactor = FMath::FInterpTo(
			CrosshairAimFactor,
			0.6f,
			DeltaTime,
			30.f);
	}
	else // Not aiming
	{
		// Spread crosshairs back to normal very quickly
		CrosshairAimFactor = FMath::FInterpTo(
			CrosshairAimFactor,
			0.f,
			DeltaTime,
			30.f);
	}

	// true 0.05 second after firing
	if (bFiringBullet)
	{
		CrosshairShootingFactor = FMath::FInterpTo(
			CrosshairShootingFactor,
			0.3f,
			DeltaTime,
			60.f);
	}
	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(
			CrosshairShootingFactor,
			0.f,
			DeltaTime,
			60.f);
	}

	CrosshairSpreadMultiplier =
		DefaultSpreadsScale + //0.5
		CrosshairVelocityFactor +
		CrosshairInAirFactor -
		CrosshairAimFactor +
		CrosshairShootingFactor
		;
}

void AShooterCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true;

	GetWorldTimerManager().SetTimer(
		CrosshairShootTimer,
		this,
		&AShooterCharacter::FinishCrosshairBulletFire,
		ShootTimeDuration);
}

void AShooterCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false;
}

void AShooterCharacter::FireButtonPressed()
{
	bFireButtonPressed = true;
	FireWeapon();
	//if (WeaponHasAmmo())
	//{
	//	bFireButtonPressed = true;
	//	StartFireTimer();
	//}
}

void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void AShooterCharacter::StartFireTimer()
{
	if(EquippedWeapon)
	{
		CombatState = ECombatState::ECS_FireTimerInProgress;

		GetWorldTimerManager().SetTimer(
			AutoFireTimer,
			this,
			&AShooterCharacter::AutoFireReset,
			EquippedWeapon->GetAutoFireRate());
	}


	//if (bShouldFire)
	//{
	//	FireWeapon();
	//	bShouldFire = false;
	//	GetWorldTimerManager().SetTimer(
	//		AutoFireTimer,
	//		this,
	//		&AShooterCharacter::AutoFireReset,
	//		AutomaticFireRate);
	//}
}


void AShooterCharacter::AutoFireReset()
{
	if (CombatState == ECombatState::ECS_Stunned) return;

	CombatState = ECombatState::ECS_Unoccupied;
	if (EquippedWeapon == nullptr) return;

	if (WeaponHasAmmo())
	{
		bShouldFire = true;
		if (bFireButtonPressed && EquippedWeapon->GetAutomatic())
		{
			FireWeapon();
		}
	}
	else
	{
		//Reload Weapon
		ReloadWeapon();
	}

	//if (WeaponHasAmmo())
	//{
	//	bShouldFire = true;
	//	if (bFireButtonPressed)
	//	{
	//		StartFireTimer();
	//	}
	//}

}

bool AShooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult,FVector& OutHitLocation)
{
	// Get Viewport Size
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	//Get screen space location of crosshairs
	FVector2D CrosshairLocation(ViewportSize.X / 2., ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	//Get world postion and direction of crosshairs
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);
	if (bScreenToWorld)
	{
		//Trace from Crosshair world location outward
		const FVector Start{ CrosshairWorldPosition };
		const FVector End{ Start + CrosshairWorldDirection * 50'000.f };
		OutHitLocation = End;
		if(GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End , ECollisionChannel::ECC_Visibility))
		{
			OutHitLocation = OutHitResult.Location;
			return true;
		}
		//or
		//if (OutHitResult.bBlockingHit)
		//{
		// OutHitLocation = OutHitResult.Location;
		//	return true;
		//}
	}
	
	return false;
}

void AShooterCharacter::TraceForItems()
{
	if (bShouldTraceForItems)
	{
		FHitResult ItemTraceRestul;
		FVector HitLocation;
		if (TraceUnderCrosshairs(ItemTraceRestul, HitLocation))
		{
			TraceHitItem = Cast<AItem>(ItemTraceRestul.Actor);
			const auto TraceHitWeapon = Cast<AWeapon>(TraceHitItem);
			if(TraceHitWeapon)
			{
				if (TraceHitWeapon->GetItemState() == EItemState::EIS_Pickup)
				{
					PrePickWeapon = TraceHitWeapon;
				}
				
				if (HighlightedSlot == -1)
				{
					//Not currently highlighting a slot; hightlight one
					HighlightInventorySlot();
				}
			}
			else
			{
				// is a slot being highlight
				if (HighlightedSlot != -1)
				{
					//UnHightlight the slot
					UnHighlightInventorySlot();
				}
			}

			if (TraceHitItem && TraceHitItem->GetItemState() == EItemState::EIS_EquipInterping)
			{
				TraceHitItem = nullptr;
			}
			if (TraceHitItem && TraceHitItem->GetPickupWidget())
			{
				//Show Item's Pickup Widget
				TraceHitItem->GetPickupWidget()->SetVisibility(true);
				TraceHitItem->EnableCustomDepth();

				if (Inventory.Num() >= INVENTORY_CAPACITY)
				{
					// Inventory is full
					TraceHitItem->SetCharacterInventoryFull(true);
				}
				else
				{
					// Inventory has room
					TraceHitItem->SetCharacterInventoryFull(false);
				}
			}

			// We hit an Aitem last frame
			if (TraceHitItemLastFrame)
			{
				if (TraceHitItem != TraceHitItemLastFrame)
				{
					// We are hitting a different Aitem this frame from last frame
					//Or AItem is null.
					TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);

					TraceHitItemLastFrame->DisableCustomDepth();
				}
			}
			// Store a reference to HitItem for next frame
			TraceHitItemLastFrame = TraceHitItem;
		}
		else if (TraceHitItemLastFrame)
		{
			//Not getting a blocking hit this frame
			// but we hit an Aitem last frame.
			TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
			TraceHitItemLastFrame->DisableCustomDepth();
		}

	}
	else if (TraceHitItemLastFrame)
	{
		//Not Longer overlapping any items,
		// Item last frame should not show widget.
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
		TraceHitItemLastFrame->DisableCustomDepth();
	}

}

AWeapon* AShooterCharacter::SpawnDefaultWeapon()
{
	// Check the TSubclassOf variable
	if (DefaultWeaponClass)
	{
		// Spawn the Weapon
		AWeapon* Weapon1 = GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
		Weapon1->SetBaseProperty(EItemRarity::EIR_Damaged, EWeaponType::EWT_SubmachineGun);
		EquipWeapon(Weapon1);
		Inventory.Add(Weapon1);
		EquippedWeapon->SetSlotIndex(0);

		AWeapon* Weapon2 = GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
		if (Weapon2)
		{
			Weapon2->SetBaseProperty(EItemRarity::EIR_Damaged,EWeaponType::EWT_AssaultRifle);
			Inventory.Add(Weapon2);
			Inventory[1]->SetItemState(EItemState::EIS_PickedUp);
			Inventory[1]->SetSlotIndex(1);
			Inventory[1]->DisableCustomDepth();
			Inventory[1]->DisableGlowMaterial();
			Inventory[1]->SetCharacter(this);
		}

		AWeapon* Weapon3 = GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
		if (Weapon3)
		{
			Weapon3->SetBaseProperty(EItemRarity::EIR_Damaged, EWeaponType::EWT_Pistol);
			Inventory.Add(Weapon3);
			Inventory[2]->SetItemState(EItemState::EIS_PickedUp);
			Inventory[2]->SetSlotIndex(2);
			Inventory[2]->DisableCustomDepth();
			Inventory[2]->DisableGlowMaterial();
			Inventory[2]->SetCharacter(this);
		}

	}
	//游戏开始时装备的武器关闭发光材质和边框
	EquippedWeapon->DisableCustomDepth();
	EquippedWeapon->DisableGlowMaterial();
	EquippedWeapon->SetCharacter(this);

	//if (DefaultWeaponClass)
	//{
	//	// Spawn the Weapon
	//	return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
	//}
	return nullptr;
	//// Spawn the default weapon and and equip it
	//EquipWeapon(SpawnDefaultWeapon());
	//Inventory.Add(EquippedWeapon);
	//EquippedWeapon->SetSlotIndex(0);
	////游戏开始时装备的武器关闭发光材质和边框
	//EquippedWeapon->DisableCustomDepth();
	//EquippedWeapon->DisableGlowMaterial();
	//EquippedWeapon->SetCharacter(this);

}

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip, bool bSwaping)
{
	if (WeaponToEquip)
	{

		// Get the Hand Socket
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			// Attach wht Weapon to the hand socket RightHandSocket
			HandSocket->AttachActor(WeaponToEquip, GetMesh());
		}

		if (EquippedWeapon == nullptr)
		{
			// -1 == no EquippedWeapon yet. No need to reverse the icon animation
			EquipItemDelegate.Broadcast(-1, WeaponToEquip->GetSlotIndex());
		}
		else if(!bSwaping)
		{
			EquipItemDelegate.Broadcast(EquippedWeapon->GetSlotIndex(), WeaponToEquip->GetSlotIndex());
		}

		// Set EquippedWeapon to the newly spanwed Weapon
		EquippedWeapon = WeaponToEquip;
		EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
	}

}

void AShooterCharacter::DropWeapon(int index)
{
	if (Inventory[index])
	{
		if (Inventory[index] == EquippedWeapon)
		{
			FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);

			Inventory[index]->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);
		}

		Inventory[index]->SetItemState(EItemState::EIS_Falling);

		AWeapon* Weapon = Cast<AWeapon>(Inventory[index]);
		if (Weapon)
		{
			Weapon->ThrowWeapon();
		}	
	}
}

void AShooterCharacter::SelectButtonPressed()
{
	if (TraceHitItem)
	{
		if (CombatState != ECombatState::ECS_Unoccupied) return;
		//auto TraceHitWeapon = Cast<AWeapon>(TraceHitItem);
		//SwapWeapon(TraceHitWeapon);
		TraceHitItem->StartItemCurve(this, true);
		TraceHitItem = nullptr;
		//UGameplayStatics::PlaySound2D(this, TraceHitItem->GetPickupSound());
	}
}

void AShooterCharacter::SelectButtonReleased()
{
}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
	int SlotIndex(0);
	switch (WeaponToSwap->GetWeaponType())
	{
	case EWeaponType::EWT_SubmachineGun:
		SlotIndex = 0;
		break;
	case EWeaponType::EWT_AssaultRifle:
		SlotIndex = 1;
		break;
	case EWeaponType::EWT_Pistol:
		SlotIndex = 2;
		break;
	}
	UE_LOG(LogTemp, Warning, TEXT("WeaponToSwap: % d"),SlotIndex);


	//if (Inventory.Num() - 1 >= EquippedWeapon->GetSlotIndex())
	//{
	//	Inventory[EquippedWeapon->GetSlotIndex()] = WeaponToSwap;
	//	WeaponToSwap->SetSlotIndex(EquippedWeapon->GetSlotIndex());
	//}
	DropWeapon(SlotIndex);
	if (WeaponToSwap->GetWeaponType() == EquippedWeapon->GetWeaponType())
	{
		EquipWeapon(WeaponToSwap, true);
	}
	else
	{
		WeaponToSwap->SetItemState(EItemState::EIS_PickedUp);			
	}

	Inventory[SlotIndex] = WeaponToSwap;
	WeaponToSwap->SetSlotIndex(SlotIndex);

	TraceHitItem = nullptr;
	TraceHitItemLastFrame = nullptr;
}

void AShooterCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);
}

bool AShooterCharacter::WeaponHasAmmo()
{
	if (EquippedWeapon == nullptr) return false;
	return EquippedWeapon->GetAmmo() > 0;
}

void AShooterCharacter::PlayFireSound()
{
	// Play fire sound
	if (EquippedWeapon->GteFireSound())
	{
		UGameplayStatics::PlaySound2D(this, EquippedWeapon->GteFireSound());
	}
}

void AShooterCharacter::SendBullet()
{
	// Send bullet
	const USkeletalMeshSocket* BarrelSocket =
		EquippedWeapon->GetItemMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());

		if (EquippedWeapon->GetMuzzleFlash())
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EquippedWeapon->GetMuzzleFlash(), SocketTransform);
		}

		FHitResult BeamHitResult;
		if (GetBeamEndLoacation(SocketTransform.GetLocation(), BeamHitResult))
		{
			//Does hit Actor implement BulletHitInterface?
			if (BeamHitResult.Actor.IsValid())
			{
				IBulletHitInterface* BulletHitInterface = Cast<IBulletHitInterface>(BeamHitResult.Actor.Get());
				if (BulletHitInterface)
				{
					BulletHitInterface->BulletHit_Implementation(BeamHitResult,this,GetController());
				}
				else
				{
					if (ImpactParticles)
					{
						UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamHitResult.Location);//BeamEndPoint);
					}
				}

				AEnemy* HitEnemy = Cast<AEnemy>(BeamHitResult.Actor.Get());
				if (HitEnemy)
				{
					int32 Damage{};
					if (BeamHitResult.BoneName.ToString() == HitEnemy->GetHeadBone())
					{
						//Head Shot
						Damage = EquippedWeapon->GetHeadShotDamage();
						UGameplayStatics::ApplyDamage(BeamHitResult.Actor.Get(),
							Damage,
							GetController(),
							this,
							UDamageType::StaticClass());
						HitEnemy->ShowHitNumber(Damage, BeamHitResult.Location,true);
					}
					else
					{
						//Body Shot
						Damage = EquippedWeapon->GetDamage();
						UGameplayStatics::ApplyDamage(BeamHitResult.Actor.Get(),
							Damage,
							GetController(),
							this,
							UDamageType::StaticClass());
						HitEnemy->ShowHitNumber(Damage, BeamHitResult.Location,false);
					}	
					//HitEnemy->ShowHitNumber(Damage, BeamHitResult.Location);
					//UE_LOG(LogTemp, Warning, TEXT("Hit Component:%s"), *BeamHitResult.BoneName.ToString());
				}
			}

		}
		if (BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamHitResult.Location);// BeamEndPoint);
			}


		}

	}
}

void AShooterCharacter::PlayGunfireMontage()
{
	if (CombatState == ECombatState::ECS_Stunned || CombatState == ECombatState::ECS_Dead) return;
	// Play Hip Fire Montage
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}

}

void AShooterCharacter::ReloadButtonPressed()
{
	ReloadWeapon();
}

void AShooterCharacter::ReloadWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	if (EquippedWeapon == nullptr) return;
	
	//Do we have ammo of the correct type?
	// TODO: Caeate bool CarryingAmmo()
	if (CarryingAmmo()) // replace with CarryingAmmo()
	{
		if (bAiming)
		{
			StopAiming();
		}
		// TODO: Create an enum for Weapon Type
		// TODO: switch on EquippedWeapon->WeaponType

		CombatState = ECombatState::ECS_Reloading;

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if(AnimInstance && ReloadMontage)
		{
			AnimInstance->Montage_Play(ReloadMontage);
			AnimInstance->Montage_JumpToSection(EquippedWeapon->GetReloadMontageSection());			
		}
	}
}

void AShooterCharacter::FinishReloading()
{
	if (CombatState == ECombatState::ECS_Stunned) return;
	//Update the Combat State
	CombatState = ECombatState::ECS_Unoccupied;

	if (bAimingButtonPressed)
	{
		Aim();
	}

	const auto AmmoType{ EquippedWeapon->GetAmmoType() };
	//TODO: Update AmmoMap
	if (AmmoMap.Contains(AmmoType))
	{
		// Amount of ammo the Character is carrying of the EquippedWeapon type
		int32 CarriedAmmo = AmmoMap[AmmoType];

		 // Space left in the magazine of EquippedWeapon
		const int32 MagEmptySpace = EquippedWeapon->GetMagazineCapacity() - EquippedWeapon->GetAmmo();

		if (MagEmptySpace > CarriedAmmo)
		{
			// Reload the magazine with all the ammo we are carrying
			EquippedWeapon->ReloadAmmo(CarriedAmmo);
			CarriedAmmo = 0;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
		else
		{
			// fill the magazine
			EquippedWeapon->ReloadAmmo(MagEmptySpace);
			CarriedAmmo -= MagEmptySpace;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
	}
}

void AShooterCharacter::FinishEquipping()
{
	if (CombatState == ECombatState::ECS_Stunned) return;
	CombatState = ECombatState::ECS_Unoccupied;
	if(bAimingButtonPressed)
	{
		Aim();
	}
}

bool AShooterCharacter::CarryingAmmo()
{
	if (EquippedWeapon == nullptr) return false;

	auto AmmoType = EquippedWeapon->GetAmmoType();

	if (AmmoMap.Contains(AmmoType))
	{
		return AmmoMap[AmmoType] > 0;
	}

	return false;
}

void AShooterCharacter::GrabClip()
{
	if (EquippedWeapon == nullptr) return;
	if (HandSceneComponent == nullptr) return;

	// Index for the clip bone on the Equipped Weapon
	int32 ClipBoneIndex{ EquippedWeapon->GetItemMesh()->GetBoneIndex(EquippedWeapon->GetClipBoneName()) };
	// Store the  transform of the clip
	ClipTransform = EquippedWeapon->GetItemMesh()->GetBoneTransform(ClipBoneIndex);

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative,true);
	HandSceneComponent->AttachToComponent(GetMesh(), AttachmentRules, FName(TEXT("Hand_L")));
	HandSceneComponent->SetWorldTransform(ClipTransform);

	EquippedWeapon->SetMovingClip(true);

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("GeabClip!!!"));


}	

void AShooterCharacter::ReleaseClip()
{
	EquippedWeapon->SetMovingClip(false);
}

void AShooterCharacter::CrouchButtonPressed()
{
	if(!GetCharacterMovement()->IsFalling())
	{
		bCrouching = !bCrouching;

		if(bCrouching)
		{
			GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
			GetCharacterMovement()->GroundFriction = CrouchingGroudFriction;
		}
		else
		{
			GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
			GetCharacterMovement()->GroundFriction = BaseGroudFriction;
		}
		
	}
}

void AShooterCharacter::PickupAmmo(AAmmo* Ammo)
{
	//check to see if AmmoMap contains Ammo's AmmoType
	if (AmmoMap.Find(Ammo->GetAmmoType( )))
	{
		//Get Amount of ammo in our AmmoMap for Ammo's type
		int32 AmmoCount{ AmmoMap[Ammo->GetAmmoType()] };
		AmmoCount += Ammo->GetItemCount();
		//Set the amount of ammo in the Map for this type
		AmmoMap[Ammo->GetAmmoType()] = AmmoCount;
	}

	if (EquippedWeapon->GetAmmoType() == Ammo->GetAmmoType())
	{
		//Check to see if the gun is empty
		if (EquippedWeapon->GetAmmo() == 0)
		{
			ReloadWeapon();
		}
	}

	Ammo->Destroy();
}

void AShooterCharacter::InitializeInterpLocations()
{
	FInterpLocation WeaponLocation{ WeaponInterpComp,0 };
	InterpLocations.Add(WeaponLocation);

	FInterpLocation InterpLoc1{InterpComp1,0 };
	InterpLocations.Add(InterpLoc1);
	FInterpLocation InterpLoc2{ InterpComp2,0 };
	InterpLocations.Add(InterpLoc2);
	FInterpLocation InterpLoc3{ InterpComp3,0 };
	InterpLocations.Add(InterpLoc3);
	FInterpLocation InterpLoc4{ InterpComp4,0 };
	InterpLocations.Add(InterpLoc4);
	FInterpLocation InterpLoc5{ InterpComp5,0 };
	InterpLocations.Add(InterpLoc5);
	FInterpLocation InterpLoc6{ InterpComp6,0 };
	InterpLocations.Add(InterpLoc6);
}

void AShooterCharacter::FkeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 0) return;
	
	ExChangeInventoryItems(EquippedWeapon->GetSlotIndex(), 0);
}

void AShooterCharacter::OnekeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 1) return;

	ExChangeInventoryItems(EquippedWeapon->GetSlotIndex(), 1);
}

void AShooterCharacter::TwokeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 2) return;

	ExChangeInventoryItems(EquippedWeapon->GetSlotIndex(), 2);
}

void AShooterCharacter::ThreekeyPressed()
{
	//if (EquippedWeapon->GetSlotIndex() == 3) return;

	//ExChangeInventoryItems(EquippedWeapon->GetSlotIndex(), 3);
}

void AShooterCharacter::FourkeyPressed()
{
	//if (EquippedWeapon->GetSlotIndex() == 4) return;

	//ExChangeInventoryItems(EquippedWeapon->GetSlotIndex(), 4);
}

void AShooterCharacter::FivekeyPressed()
{
	//if (EquippedWeapon->GetSlotIndex() == 5) return;

	//ExChangeInventoryItems(EquippedWeapon->GetSlotIndex(), 5);
}

void AShooterCharacter::ExChangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex)
{

	const bool bCanExchangeItems = (CurrentItemIndex != NewItemIndex) && (NewItemIndex < Inventory.Num()) &&
		((CombatState == ECombatState::ECS_Unoccupied) || (CombatState == ECombatState::ECS_Equipping));

	if (bCanExchangeItems)
	{
		if(bAiming)
		{
			StopAiming();
		}
		auto OldEquippedWeapon = EquippedWeapon;
		auto NewWeapon = Cast<AWeapon>(Inventory[NewItemIndex]);
		EquipWeapon(NewWeapon);

		OldEquippedWeapon->SetItemState(EItemState::EIS_PickedUp);
		NewWeapon->SetItemState(EItemState::EIS_Equipped);

		CombatState = ECombatState::ECS_Equipping;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && EquipMontage)
		{
			AnimInstance->Montage_Play(EquipMontage, 1.0f);
			AnimInstance->Montage_JumpToSection(FName("Equip"));
		}
		NewWeapon->PlayEquipSound(true);
	}

}

int32 AShooterCharacter::GetEmptyInventorySlot(AWeapon* SwapWeapon)
{
	if (SwapWeapon)
	{
		switch (SwapWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_SubmachineGun:
			return 0;
			break;
		case EWeaponType::EWT_AssaultRifle:
			return 1;
			break;
		case EWeaponType::EWT_Pistol:
			return 2;
			break;
		}
	}


	for (int32 i = 0; i < Inventory.Num(); i++)
	{
		if (Inventory[i] == nullptr)
		{
			return i;
		}
	}
	if (Inventory.Num() < INVENTORY_CAPACITY)
	{
		return Inventory.Num();
	}
	return -1; //Inventory if full!
}

void AShooterCharacter::HighlightInventorySlot()
{
	if (PrePickWeapon)
	{
		const int32 EmptySlot{ GetEmptyInventorySlot(PrePickWeapon) };
		HighlightIconDelegate.Broadcast(EmptySlot, true);
		HighlightedSlot = EmptySlot;
	}

}

EPhysicalSurface AShooterCharacter::GetSurfaceType()  //void AShooterCharacter::Footstep()
{
	FHitResult HitResult;
	const FVector Start{ GetActorLocation() };
	const FVector End{ Start + FVector(0.f,0.f,-400.f) };
	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;

	GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECollisionChannel::ECC_Visibility,
		QueryParams);

	return UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());
}

void AShooterCharacter::EndStun()
{
	CombatState = ECombatState::ECS_Unoccupied;

	if (bAimingButtonPressed)
	{
		Aim();
	}
}

void AShooterCharacter::Die()
{
	CombatState = ECombatState::ECS_Dead;
	bFireButtonPressed = false;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		//AnimInstance->StopAllMontages(0.0f);
		AnimInstance->Montage_Play(DeathMontage);

		APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
		if (PC)
		{
			DisableInput(PC);
			
		}

	}
}

void AShooterCharacter::FinishDeath()
{
	GetMesh()->bPauseAnims = true;
	CharacterDead.Broadcast();
}

void AShooterCharacter::UnHighlightInventorySlot()
{
	HighlightIconDelegate.Broadcast(HighlightedSlot, false);
	HighlightedSlot = -1;
}

void AShooterCharacter::Stun()
{
	if (Health <= 0.f) return;
	CombatState = ECombatState::ECS_Stunned;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		UE_LOG(LogTemp, Warning, TEXT("Stun()"));
	}
}

int32 AShooterCharacter::GetInterpLocationIndex()
{
	int32 LowestIndex = 1;
	int32 LowestCount = INT_MAX;
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, TEXT("GEtINterFunc"));
	for (int32 i = 1; i < InterpLocations.Num(); i++)
	{
		if (InterpLocations[i].ItemCount < LowestCount)
		{
			LowestIndex = i;
			LowestCount = InterpLocations[i].ItemCount;

			//FString InterIndexStr = FString::Printf(TEXT("GetIndex: %d"), LowestIndex);
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, InterIndexStr);
		}
	}
	return LowestIndex;
}

void AShooterCharacter::IncrementInterpLocItemCount(int32 Index, int32 Amount)
{
	if (Amount < -1 || Amount > 1) return;

	if (InterpLocations.Num() >= Index)
	{
		InterpLocations[Index].ItemCount += Amount;
	}
}

void AShooterCharacter::StartPickupSoundTimer()
{
	bShouldPlayPickupSound = false;
	GetWorldTimerManager().SetTimer(
		PickupSoundTimer, 
		this, 
		&AShooterCharacter::ResetPickupSoundTimer, 
		PickupSoundResetTime);

}

void AShooterCharacter::StartEquipSoundTimer()
{
	bShouldPlayEquipSound = false;
	GetWorldTimerManager().SetTimer(
		EquipSoundTimer,
		this,
		&AShooterCharacter::ResetEquipSoundTimer,
		EquipSoundResetTime);
}


float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}

void AShooterCharacter::IncrementOverlappedItemCount(int8 Amount)
{
	if (OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;
	}
}

/* No longer needed;AItem has GetInterpLocatin
FVector AShooterCharacter::GetCameraInterpLocation()
{
	const FVector CameraWorldLocation{ FollowCamera->GetComponentLocation() };
	const FVector CameraForward{ FollowCamera->GetForwardVector() };
	// Desired = CameraWorldLocation + Forward * A + Up * B

	return CameraWorldLocation + CameraForward * CameraInterpDistance
		+ FVector(0.f, 0.f, CameraInterpElevation);
}*/

void AShooterCharacter::GetPickupItem(AItem* Item)
{
	Item->PlayEquipSound(false);
	//if (Item->GetEquipSound())
	//{
	//	UGameplayStatics::PlaySound2D(this, Item->GetEquipSound());
	//}
	auto Weapon = Cast<AWeapon>(Item);
	if (Weapon)
	{
		if(Inventory.Num() < INVENTORY_CAPACITY)
		{

			Weapon->SetSlotIndex(Inventory.Num());
			Inventory.Add(Weapon);
			Weapon->SetItemState(EItemState::EIS_PickedUp);
		}
		else // Inventory is full! Swap with EquippedWeapon
		{
			SwapWeapon(Weapon);
			//GetEmptyInventorySlot(Weapon);
		}
	}
	auto Ammo = Cast<AAmmo>(Item);
	if (Ammo)
	{
		PickupAmmo(Ammo);
	}
}

FInterpLocation AShooterCharacter::GetInterplocation(int32 Index)
{
	if(Index <= InterpLocations.Num())
	{
		return InterpLocations[Index];
	}
	return FInterpLocation();
}

