// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "WeaponType.h"
#include "ShooterAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EOffsetState : uint8
{
	EOS_Aiming UMETA(DisplayName = "Aiming"),
	EOS_Hip UMETA(DisplayName = "Hip"),
	EOS_Reloading UMETA(DisplayName = "Reloading"),
	EOS_InAir UMETA(DisplayName = "InAir"),

	EOS_MAX UMETA(DisplayName = "DefaultMAX")
};

/**
 * 
 */
UCLASS()
class SHOOT_01_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:

	UShooterAnimInstance();

	//每一帧更新动画属性,在蓝图调用
	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float DeltaTime);

	//在动画实例初始化时调用，可以在此函数中执行初始化逻辑，如：获取设置变量
	virtual void NativeInitializeAnimation() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class AShooterCharacter* ShooterCharacter;

	/** The speed of the character*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;

	/** Whether or not the character os on the air */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;
	
	/** Whether or not the character is moving*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	/** Offset yaw used for strafing*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float MovementOffsetYaw;

	/** Offset yaw the frame before wu stopped moving*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float LastMovementOffsetYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	/** Yaw of the Character this frame ; 
	*	Only update when stand still and not in air
	*/
	float TIPCharacterYaw;

	/** Yaw of the Character the previous frame 
	*	Only update when stand still and not in air
	*/
	float TIPCharacterYawLastFrame;

	/** Yaw of the Character this frame ;*/
	// update every farme
	FRotator CharacterRotation;    //float CharacterYaw;

	/** Yaw of the Character the previous frame */
	// update every farme
	FRotator CharacterRotationLastFrame;       //float CharacterYawLastFrame;

	/** Yaw delta used for leaning in the runing blendspace */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Lean, meta = (AllowPrivateAccess = "true"))
	float YawDelta;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
	float RootYawOffset;

	/** Rotation curve value this frame*/
	float RotationCurve;
	/** Rotation curve value last frame */
	float RotationCurveLastFrame;

	/** The pitch of the aim rotation, used for Aim Offset*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
	float Pitch;

	/** True when reloading, used to prevent Aim Offset while reloading*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
	bool bReloading;

	/** Offset state; used to determine which Aim Offset to use */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Plase", meta = (AllowPrivateAccess = "true"))
		EOffsetState OffsetState;

	/** True when crouching*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Lean, meta = (AllowPrivateAccess = "true"))
		bool bCrouching;

	/** True when equipping */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, Meta = (AllowPrivateAccess = "true"))
		bool bEquipping;

	/** Chage the recoil weight based on turning in plase and aiming*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
		float RecoilWeight;
	/** True when turning in place */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
		bool bTurningInPlace;

	/** Weapon type for the currently equipped weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
		EWeaponType EquippedWeaponType;

	/** True when not reloading or equipping*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
		bool bShouldUseFABRIK;

protected:
	/** Handle turning in place variables */
	void TurnInPlace();

	/** Handle calculations for leaning while running */
	void Lean(float DeltaTime);
};
