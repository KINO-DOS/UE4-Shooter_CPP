// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStarTextAnim);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStopTextAnim);

/**
 * 
 */
UCLASS()
class SHOOT_01_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AShooterPlayerController();


	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

private:
	/** Reference to the Overall HUD Overlay Blueprint Clss */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<class UUserWidget> HUDOverlayClass;
	
	/** Variable to hold the HUD Overlay Widget after creating it */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Widgets, meta = (AllowPrivateAccess = "true"))
		UUserWidget * HUDOverlay;

	//玩家得分
	int PlayerScore;

	UPROPERTY(visibleAnywhere, BlueprintReadWrite, Category = Widgets, meta = (AllowPrivateAccess = "true"))
	bool WeaponCanUp;

	UPROPERTY(visibleAnywhere, BlueprintReadWrite, Category = Widgets, meta = (AllowPrivateAccess = "true"))
	bool OldWeaponCanUp;

	UPROPERTY(BlueprintAssignable, Category = Delegates, meta = (AllowPrivateAccess = "true"))
	FStarTextAnim StarTextAnim;

	UPROPERTY(BlueprintAssignable, Category = Delegates, meta = (AllowPrivateAccess = "true"))
	FStopTextAnim StopTextAnim;

public:

	//增加玩家得分
	void AddPlayerScore(int Num);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE int GetPlayerScore() const { return PlayerScore; }

	UFUNCTION(BlueprintCallable)
	void SetWeaponCanUpTrue();

	UFUNCTION(BlueprintCallable)
	void SetWeaponCanUpFalse();
	
	
};
