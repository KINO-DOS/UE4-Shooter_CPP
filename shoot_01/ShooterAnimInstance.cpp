// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon.h"


UShooterAnimInstance::UShooterAnimInstance() :
	Speed(0.f),
	bIsInAir(false),
	bIsAccelerating(false),
	MovementOffsetYaw(0.f),
	LastMovementOffsetYaw(0.f),
	bAiming(false),
	CharacterRotation(FRotator(0.f)),
	CharacterRotationLastFrame(FRotator(0.f)),
	TIPCharacterYaw(0.f),
	TIPCharacterYawLastFrame(0.f),
	YawDelta(0.f),
	RootYawOffset(0.f),
	RotationCurveLastFrame(0.f),
	RotationCurve(0.f),
	Pitch(0.f),
	bReloading(false),
	OffsetState(EOffsetState::EOS_Hip),
	bCrouching(false),
	RecoilWeight(1.0f),
	bTurningInPlace(false),
	EquippedWeaponType(EWeaponType::EWT_MAX),
	bShouldUseFABRIK(false)
{

}

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (ShooterCharacter == nullptr)
	{
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	}
	if (ShooterCharacter)
	{
		bCrouching = ShooterCharacter->GetCrouching();
		bReloading = ShooterCharacter->GetCombatState() == ECombatState::ECS_Reloading;
		bEquipping = ShooterCharacter->GetCombatState() == ECombatState::ECS_Equipping;
		bShouldUseFABRIK = ShooterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied ||
			ShooterCharacter->GetCombatState() == ECombatState::ECS_FireTimerInProgress;

		//Get the speed of the character from velocity
		FVector Velocity{ ShooterCharacter->GetVelocity()};

		//排除竖直方向Z轴的大小
		Velocity.Z = 0; 
		//获取向量长度，只需要速度，不需要方向。
		Speed = Velocity.Size();  

		//Is the Character in the air? //获取CharacteerMovement来判断是否在（下落）空中
		bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();

		//Is the character accelerating?   //获取加速度大小
		if (ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f)
		{
			bIsAccelerating = true;
		}
		else
		{
			bIsAccelerating = false;
		}

		FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());
		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;

		if(ShooterCharacter->GetVelocity().Size() > 0.f)LastMovementOffsetYaw = MovementOffsetYaw;


		FString RotationMessage = FString::Printf(TEXT("Base Aim Rotation: %f"), AimRotation.Yaw);
		FString MovementRotationMessage = FString::Printf(TEXT("Movement Rotation: %f"), MovementRotation.Yaw);
		FString MovementOffsetMessage = FString::Printf(TEXT("MovementOffset: %f"), MovementOffsetYaw);
		if (GEngine)
		{
			//GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::Red, RotationMessage);
			//GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::Yellow, MovementRotationMessage);
			GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::Cyan, MovementOffsetMessage);
		}

		bAiming = ShooterCharacter->GetAiming();

		if (bReloading)
		{
			OffsetState = EOffsetState::EOS_Reloading;
		}
		else if (bIsInAir)
		{
			OffsetState = EOffsetState::EOS_InAir;
		}
		else if (ShooterCharacter->GetAiming())
		{
			OffsetState = EOffsetState::EOS_Aiming;
		}
		else
		{
			OffsetState = EOffsetState::EOS_Hip;
		}
		//Chek if ShooterCharacter has a Valid EquippedWeapon
		if (ShooterCharacter->GetEquippedWeapon())
		{
			EquippedWeaponType = ShooterCharacter->GetEquippedWeapon()->GetWeaponType();
		}
	}

	TurnInPlace();
	Lean(DeltaTime);
}


void UShooterAnimInstance::NativeInitializeAnimation()
{
	//尝试获取该动画实例的拥有者，也就是当前动画实例绑定的Pawn
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());

}

void UShooterAnimInstance::TurnInPlace()
{
	if (ShooterCharacter == nullptr) return;

	Pitch = ShooterCharacter->GetBaseAimRotation().Pitch;

	if (Speed > 0 || bIsInAir)
	{
		//Dot't want to turn in place; Character is moving ， or isInAir
		RootYawOffset = 0.f;
		TIPCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		TIPCharacterYawLastFrame = TIPCharacterYaw;
		RotationCurveLastFrame = 0.f;
		RotationCurve = 0.f;
	}
	else
	{
		//Yaw 向右转是正，向左负
		TIPCharacterYawLastFrame = TIPCharacterYaw;
		TIPCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		const float TIPYawDelta{ TIPCharacterYaw - TIPCharacterYawLastFrame };

		//Root Yaw Offset, update and clamped to [-180,180]  
		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - TIPYawDelta);

		// 1.0 if turning, 0.0 if not
		const float Turning{ GetCurveValue(TEXT("Turning")) };
		if (Turning > 0)
		{
			bTurningInPlace = true;
			RotationCurveLastFrame = RotationCurve;
			RotationCurve = GetCurveValue(TEXT("Rotation"));
			const float DeltaRotation{ RotationCurve - RotationCurveLastFrame };

			//RootYawOffest > 0,-> Turning Left.RootYawOffset < 0. -> Turning Right
			RootYawOffset > 0 ? RootYawOffset -= DeltaRotation : RootYawOffset += DeltaRotation;
		
			//Abs() 绝对值
			const float ABSRootYawOffset{ FMath::Abs(RootYawOffset) };
			if (ABSRootYawOffset > 90.f)
			{
				const float YawExcess{ ABSRootYawOffset - 90.f };
				RootYawOffset > 0 ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
			}
			GEngine->AddOnScreenDebugMessage(-1, -1, FColor::Red, FString::Printf(TEXT("TurnInPlace_RootYawOffset: %f"), RootYawOffset));
		}
		else 
		{
			bTurningInPlace = false;
		}
		GEngine->AddOnScreenDebugMessage(5, -1, FColor::Yellow, FString::Printf(TEXT("RootYawOffset: %f"), RootYawOffset));


	}

	//Set the Recoil Weight
	if (bTurningInPlace)
	{
		if (bReloading || bEquipping)
		{
			RecoilWeight = 1.f;
		}
		else
		{
			RecoilWeight = 0.f;
		}

	}
	else //not turning in place
	{
		if (bCrouching)
		{
			if (bReloading || bEquipping)
			{
				RecoilWeight = 1.f;
			}
			else
			{
				RecoilWeight = 0.1f;
			}
		}
		else
		{
			if (bAiming || bEquipping || bReloading)
			{
				RecoilWeight = 1.f;
			}
			else
			{
				RecoilWeight = 0.5f;
			}
		}
	}

}

void UShooterAnimInstance::Lean(float DeltaTime)
{
	if (ShooterCharacter == nullptr) return;
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = ShooterCharacter->GetActorRotation();

	//这个函数会先把Delta 转换成(-360,360) 再 转换成（0，360），然后在 转回（-180，180）；
	//这个函数可以用于计算两个角度之间的最短距离，从而使转换更加平滑和自然。
	const FRotator Delta{ UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation,CharacterRotationLastFrame) };

	const float Target{ Delta.Yaw / DeltaTime };
	const float Interp{ FMath::FInterpTo(YawDelta,Target,DeltaTime,6.f) }; // 如果60帧/s ,10帧到Target
	YawDelta = FMath::Clamp(Interp, -90.f, 90.f);
	if (GEngine)
	{
		//GEngine->AddOnScreenDebugMessage(5, -1, FColor::Cyan, FString::Printf(TEXT("Delta.Yaw: %f"), Delta.Yaw));
		//GEngine->AddOnScreenDebugMessage(6, -1, FColor::Cyan, FString::Printf(TEXT("CharacterRotation.Yaw: %f"), CharacterRotation.Yaw));
		//GEngine->AddOnScreenDebugMessage(5, -1, FColor::Cyan, FString::Printf(TEXT("Delta.Yaw: %f"), Delta.Yaw));
		////GEngine->AddOnScreenDebugMessage(4, -1, FColor::Cyan, FString::Printf(TEXT("Target: %f"), Target));
		////GEngine->AddOnScreenDebugMessage(3, -1, FColor::Cyan, FString::Printf(TEXT("Interp: %f"), Interp));
		//GEngine->AddOnScreenDebugMessage(2, -1, FColor::Cyan, FString::Printf(TEXT("YawDelta: %f"), YawDelta));
	}
}

