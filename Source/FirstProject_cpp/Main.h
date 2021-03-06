// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Main.generated.h"

UENUM(BlueprintType)
enum class EMovementStatus : uint8
{
	EMS_Normal UMETA(DisplayName = "Normal"),
	EMS_Sprinting UMETA(DisplayName = "Sprinting"),
	EMS_Dead UMETA(DisplayName = "Dead"),

	EMS_MAX UMETA(DisplayName = "DefaulMAX")
};

UENUM(BlueprintType)
enum class EStaminasStatus : uint8
{
	ESS_Normal UMETA(DisplayName = "Normal"),
	ESS_BelowMinimum UMETA(DisplayName = "BelowMinimum"),
	ESS_Exhausted UMETA(DisplayName = "Exhausted"),
	ESS_ExhaustedRecovering UMETA(DisplayName = "ExhaustedRecovering"),

	ESS_MAX UMETA(DisplayName = "Default MAX")
};


//******************************************************************************************
//											public Class
//******************************************************************************************
UCLASS()
class FIRSTPROJECT_CPP_API AMain : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMain();

	UPROPERTY(EditDefaultsOnly, Category = "SaveData")
	TSubclassOf<class AItemStorage> WeaponStorage;

	FORCEINLINE void SetStaminaStatus(EStaminasStatus Status) { this->StaminaStatus = Status; }

	/** Camera boom positioning the camera behind the player */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow Camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class UParticleSystem* HitParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class USoundCue* HitSound;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = Items)
	class AWeapon* EquippedWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	TSubclassOf<class ASkillBase> Skill_1;

	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	TSubclassOf<class ASkillBase> Skill_2;

	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	TSubclassOf<class ASkillBase> Skill_3;

	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	TSubclassOf<class ASkillBase> Skill_3_2;

	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	TSubclassOf<class ASkillBase> Skill_4;

	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	TSubclassOf<class ASkillBase> Skill_5;

	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	TSubclassOf<class ASkillBase> Skill_6;

	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	TSubclassOf<class ASkillBase> Skill_7;

	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	TSubclassOf<class ASkillBase> Skill_8;

	// ?????? ?????? ???? ??????
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	class UBoxComponent* SkillSpawningBox;

	/**
	When you overlapped with item, you can choice whether to equip it or not.
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items)
	class AItem* ActiveOverlappingItem;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* CombatMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class AEnemy* CombatTarget;

	FORCEINLINE void SetCombatTarget(AEnemy* Target) { CombatTarget = Target; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Controller")
	class AMainPlayerController* MainPlayerController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<class AEnemy> EnemyFilter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	TSubclassOf<class UCameraShake> CameraShake;


//******************************************************************************************
// 										 public Valuables
//******************************************************************************************
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EMovementStatus MovementStatus;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EStaminasStatus StaminaStatus;

	// ?????????? ??????????, ?????? ?????? ?? ?????? ?????????? ?? -> Pickup.h???? ????????
	TArray<FVector> PickupLocations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float StaminaDrainRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MinSprintStamina;

	/** Base turn rates to scale turning functions for the camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats")
	float MaxHealth;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Stats")
	float Health;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats")
	float MaxStamina;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Stats")
	float Stamina;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Stats")
	int32 Coins;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Running")
	float RunningSpeed;

	float SprintingSpeed;

	bool bShiftKeyDown;

	bool IsMoveKeyDown;

	bool bLMBDown;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anims")
	bool bAttacking;

	float InterpSpeed;

	bool bInterpToEnemy;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bHasCombatTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	FVector CombatTargetLocation;

	FORCEINLINE void SetHasCombatTarget(bool HasTarget) { bHasCombatTarget = HasTarget; }

	bool bMovingForward;
	bool bMovingRight;

	bool bESCDown;

	bool bSkillKeyDown;

//******************************************************************************************
//										public Protected
//******************************************************************************************
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



//******************************************************************************************
//										public Function
//******************************************************************************************
public:	
	// Called every frame

	UFUNCTION(BlueprintCallable)
	void ShowPickupLocation();

	/** Set movement status and running speed */
	void SetMovementStatus(EMovementStatus Status);

	/** Pressed down to enable sprinting */
	void ShiftKeyDown();

	/** Released to stop sprinting */
	void ShiftKeyUp();

	void DecrementHealth(float Amount);

	void Die();

	UFUNCTION(BlueprintCallable)
	void IncrementCoins(int32 Amount);

	UFUNCTION(BlueprintCallable)
	void IncrementHealth(float Amount);

	UFUNCTION(BlueprintCallable)
	void IncrementStamina(float Amount);

	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Called for forwards/backwards input */
	void MoveForward(float Value);
	
	/** Called for side to side input */
	void MoveRight(float Value);

	/** Called for Yaw rotation */
	void Turn(float Value);

	/** Called for Pitch rotation */
	void LookUp(float Value);

	/** Called via input to turn at a given rate 
	* @param Rate This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void TurnAtRate(float Rate);
	/** Called via input to loot up/down at a given rate
	* @param Rate This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void LookUpAtRate(float Rate);

	void LMBDown();

	void LMBUp();

	void ESCDown();

	void ESCUp();

	void SkillKeyUp();
	void Skill1Down();
	void Skill2Down();
	void Skill3Down();
	void Skill4Down();
	void Skill5Down();
	void Skill6Down();

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	void SetEquippedWeapon(AWeapon* WeaponToSet);

	UFUNCTION(BlueprintCallable)
	AWeapon* GetEquippedWeapon() { return EquippedWeapon; }

	FORCEINLINE void SetActiveOverlappingItem(AItem* ItemToSet) { ActiveOverlappingItem = ItemToSet; }
	
	void Attack();

	UFUNCTION(BlueprintCallable)
	void AttackEnd();

	UFUNCTION(BlueprintCallable)
	void PlaySwingSound();

	void SetInterpToEnemy(bool Interp);

	FRotator GetLookAtRotationYaw(FVector Target);

	// APawn?? TakeDamge() ???? ??????
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable)
	void DeathEnd();

	virtual void Jump() override;

	void UpdateCombatTarget();

	void SwitchLevel(FName LevelName);

	UFUNCTION(BlueprintCallable)
	void SaveGame();

	// SetPosition is boolien whether you just moving another location or jumping another Level
	UFUNCTION(BlueprintCallable)
	void LoadGame(bool SetPosition);

	// Using function when jumping another level.
	void LoadGameNoSwitch();

	bool CanMove(float Value);

	void EquipSocket();

	FVector GetSpawnPoint();
};
