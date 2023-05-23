// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayerController.h"
#include "Blueprint/UserWidget.h"








AShooterPlayerController::AShooterPlayerController():
PlayerScore(0)
{

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
