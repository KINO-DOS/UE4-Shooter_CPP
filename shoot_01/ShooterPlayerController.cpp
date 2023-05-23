// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayerController.h"
#include "Blueprint/UserWidget.h"








AShooterPlayerController::AShooterPlayerController():
PlayerScore(0),
WeaponCanUp(false)
{

}


void AShooterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (OldWeaponCanUp != WeaponCanUp)
	{
		if (WeaponCanUp)
		{
			StarTextAnim.Broadcast();
			UE_LOG(LogTemp, Warning, TEXT("Cast:Star"));
		}
		else
		{
			StopTextAnim.Broadcast();
			UE_LOG(LogTemp, Warning, TEXT("Cast:Stop"));
		}
		OldWeaponCanUp = WeaponCanUp;
	}
}

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	//Check out HUDOverlayClass TSubclassOf variable
	if (HUDOverlayClass)
	{
		HUDOverlay = CreateWidget<UUserWidget>(this,HUDOverlayClass);
		if (HUDOverlay)
		{
			HUDOverlay->AddToViewport();
			HUDOverlay->SetVisibility(ESlateVisibility::Visible);
		}
	}

}

void AShooterPlayerController::AddPlayerScore(int Num)
{
	if(Num > 0 && Num <= 100)
	PlayerScore += Num;
}

void AShooterPlayerController::SetWeaponCanUpTrue()
{
	OldWeaponCanUp = WeaponCanUp;
	WeaponCanUp = true;
}

void AShooterPlayerController::SetWeaponCanUpFalse()
{
	OldWeaponCanUp = WeaponCanUp;
	WeaponCanUp = false;
}
