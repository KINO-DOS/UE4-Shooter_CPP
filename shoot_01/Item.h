// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Item.generated.h"

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	//武器稀有度
	EIR_Damaged UMETA(DisplauName = "Damaged"),
	EIR_Common UMETA(DisplayName = "Common"),
	EIR_Uncommon UMETA(DisplayName = "UNCommon"),
	EIR_Rare UMETA(DisplayName = "Rare"),
	EIR_Legendary UMETA(DisplayName = "Legendary"),

	EIR_MAX UMETA(DispalyName = "DefaultMAX")

};

UENUM(BlueprintType)
enum class EItemState : uint8
{
	//武器状态
	EIS_Pickup UMETA(DisplauName = "Pickup"),
	EIS_EquipInterping UMETA(DisplayName = "EquipInterping"),
	EIS_PickedUp UMETA(DisplayName = "PickedUp"),
	EIS_Equipped UMETA(DisplayName = "Equipped"),
	EIS_Falling UMETA(DisplayName = "Falling"),

	EIS_MAX UMETA(DispalyName = "DefaultMAX")

};
	/**
		* 将 EIR_MAX 添加到枚举中的目的是为了确保枚举中的值都是有效的。
	在定义枚举时，我们通常会将最后一个值设置为枚举的最大值，
	以确保其他值都在有效范围内。
	*/

UENUM(BlueprintType)
enum class EItemType : uint8
{
	EIT_Ammo UMETA(DisplayName = "Ammo"),
	EIT_Weapon UMETA(DisplayName = "Weapon"),

	EIT_MAX UMETA(DisplayName = "MAX")
};

USTRUCT(BlueprintType)
struct FItemRarityTable :public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor GlowColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor LightColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor DarkColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumberOfStars;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* IconBackgroud;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CustomDepthStencil;

};



UCLASS()
class SHOOT_01_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	/** Skeletal Mesh for the item*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ItemMesh;

	/** Line trace collodes with box to show HUD widgets*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* CollisionBox;

	/** Popup widget for when the player looks at the item*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties",meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* PickupWidget;

	/** Enables item tracing when overlapped */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* AreaSphere; 

	/** The name which Appears on the Pickup widget*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	FString ItemName;

	/** Item count (ammo,etc.*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 ItemCount;

	/** Item rarity -determines number of stars in Pickup Widget */
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	//EItemRarity ItemRarity;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	EItemRarity ItemRarity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TArray<bool> ActiveStars;

	/** State of the Item */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	EItemState ItemState;

	/*** The curve asset to use for the item's Z location when interping */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class UCurveFloat* ItemZCurve;

	/** Starting location when interping begins */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
		FVector ItemInterpStartLocation;
	/** Target interp location in front of the camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
		FVector CameraTargetLocation;
	/** ture when interping */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
		bool bInterping;

	/** Palys when start interping*/
	FTimerHandle ItemInterpTimer;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
		float ZCurveTime;

	/** Pointer to the character*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
		class AShooterCharacter* Character;

	/** X and Y for the Item while interping*/
	float ItemInterpX;
	float ItemInterpY;

	/** Initial Yaw offset between the camera and the interping item*/
	float InterpInitialYawOffset;

	/** Curve used to scale the item when intering*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
		UCurveFloat* ItemScaleCurve;

	/** Sound palyed when Item is picked up */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
		class USoundCue* PickupSound;

	/** Sound played when the Item is equipped*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
		USoundCue* EquipSound;

	/** Enum for the type of item this Item is*/
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	EItemType ItemType;

	/** Index of the interp location this item is interping to*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
		int32 InterpLocIndex;

	/** Index for the material we'd like to change at runtime */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 MaterialIndex;

	/** Dynamic instance that we can change at runtime */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UMaterialInstanceDynamic* DynamicMaterialInstance;

	/** Material Instance used with the Dynamic material Instance */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UMaterialInstance* MaterialInstance;

	/** 控制边框的开启关闭*/
	bool bCanChangeCustomDepth;

	/** Curve to drive the dynamic material parameters */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
		class UCurveVector* PulseCurve;

	/** Curve to drive the dynamic material parameters */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
		class UCurveVector* InterpPulseCurve;

	FTimerHandle PulseTimer;

	/** Time for the PulseTimer */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
		float PulseCurveTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
		float GlowAmount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
		float FresnelExponent;;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
		float FresnelReflectFraction;

	///** Backgroud for this item im the incentory*/
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	//	UTexture2D* IconBackground;

	/** Icon for this item im the incentory*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
		UTexture2D* IconItem;

	/** Ammo for this item im the incentory*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
		UTexture2D* AmmoIcon;

	/** Slot int the Inventory array */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
		int32 SlotIndex;

	/** True when the Character's inventory is full */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
		bool bCharacterInventoryFull;

	/** Item Rarity data table */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataTable, meta = (AllowPrivateAccess = "true"))
	UDataTable* ItemRarityDataTable;

	/** Color in the glow material */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	FLinearColor GlowColor; 

	/** Light Color in the Picup widget */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	FLinearColor LightColor;

	/** Dark Color in the Picup widget */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	FLinearColor DarkColor;

	/** Number of stars in the Picup widget */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	int32 NumberOfStars;

	/**  Backgroud icon in the inventory */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Rarity, meta = (AllowPrivateAccess = "true"))
	UTexture2D* IconBackgroud;

protected:
	// Called when the Overlapping AreaSphere
	UFUNCTION()
	void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	// Called when End Overlapping AreaShpere
	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	/** Sets the ActiveStars array of bools based on rarity*/
	void SetActiveStars();

	/** Sets properties of the Item's components based on State*/
	virtual void SetItemProperties(EItemState State);

	/** Called when ItemInterpTimer is finished*/
	void FinishInterping();

	/** Handels item interpolation when in the EquipInterping state*/
	void ItemInterp(float DeltaTime);

	/** Get interp location based on the item type */
	FVector GetInterpLocation();

	void PlayPickupSound( bool bForcePlaySound = false); //void PlayPickupSound();

	//virtual void EnableCustomDeth();
	//virtual void DisableCustomDepth();
	virtual void InitializeCustomDepth();

	virtual void OnConstruction(const FTransform& Transform) override;

	void UpdatePulse();
	void ResetPulseTimer();
	void StartPulseTimer();


public:

	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE UBoxComponent* GetCollisionBox()const { return CollisionBox; }
	FORCEINLINE EItemState GetItemState() const { return ItemState; }
	void SetItemState(EItemState State);
	FORCEINLINE USkeletalMeshComponent* GetItemMesh() const { return ItemMesh; }
	FORCEINLINE USoundCue* GetPickupSound() const { return PickupSound; }
	FORCEINLINE USoundCue* GetEquipSound() const { return EquipSound; }
	FORCEINLINE UMaterialInstance* GetMaterialInstance() const { return MaterialInstance; }
	FORCEINLINE UMaterialInstanceDynamic* GetDynamicMaterialInstance() const { return DynamicMaterialInstance; }
	FORCEINLINE FLinearColor GetGlowColor() const { return GlowColor; }
	FORCEINLINE int32 GetMaterialIndex() const { return MaterialIndex; }

	FORCEINLINE void SetMaterialIndex(int32 Index) { MaterialIndex = Index; }
	FORCEINLINE void SetDynamicMaterialInstance(UMaterialInstanceDynamic* Instance) { DynamicMaterialInstance = Instance; }
	FORCEINLINE void SetPickupWidget(UWidgetComponent* Widget) { PickupWidget = Widget; }
	FORCEINLINE void SetPickupSound(USoundCue* Sound) { PickupSound = Sound; }
	FORCEINLINE void SetEquipSound(USoundCue* Sound) { EquipSound = Sound; }
	FORCEINLINE void SetItemName(FString Name) { ItemName = Name; }
	// Set item icon for hte inventory
	FORCEINLINE void SetIconItem(UTexture2D* Icon) { IconItem = Icon; }
	// Set ammo icon for the pickup widget
	FORCEINLINE void SetAmmoIcon(UTexture2D* Icon) { AmmoIcon = Icon; }
	FORCEINLINE void SetMaterialInstance(UMaterialInstance* Instance) { MaterialInstance = Instance; }

	FORCEINLINE int32 GetItemCount() const { return ItemCount; }
	FORCEINLINE int32 GetSlotIndex() const { return SlotIndex; }
	FORCEINLINE void SetSlotIndex(int32 Index)  { SlotIndex = Index; }
	FORCEINLINE void SetCharacter(AShooterCharacter* Char) { Character  = Char; }
	FORCEINLINE void SetCharacterInventoryFull(bool bFull) { bCharacterInventoryFull = bFull; }

	/** Called from the AShooterCharacter class*/
	void StartItemCurve(AShooterCharacter* Char , bool bForcePlaySound = false); //void StartItemCurve(AShooterCharacter* Char);

	// Called in AShooterCharacter::GetPickupItem
	void PlayEquipSound(bool bForcePlaySound = false);                   //  void PlayEquipSound();

	virtual void EnableCustomDepth();
	virtual void DisableCustomDepth();


	/** 控制边框的开启关闭*/
	void EnableGlowMaterial();
	void DisableGlowMaterial();
};
