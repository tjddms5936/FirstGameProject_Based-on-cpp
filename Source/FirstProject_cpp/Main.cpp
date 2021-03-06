// Fill out your copyright notice in the Description page of Project Settings.


#include "Main.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Weapon.h"
#include "Animation/AnimInstance.h"
#include "Sound/SoundCue.h"
#include "Enemy.h"
#include "MainPlayerController.h"
#include "FirstSaveGame.h"
#include "ItemStorage.h"
#include "CameraShaking.h"
#include "SkillBase.h"
#include "Particles/ParticleSystemComponent.h"


// Sets default values
AMain::AMain()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Create Camera Boom (pulls towards the player if there's a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 1600.f; // Camera follows at this distance
	CameraBoom->bUsePawnControlRotation = true; // Rotate arm based on controller

	// Set size for collision capsule
	GetCapsuleComponent()->SetCapsuleSize(48.f, 105.f);

	// Create Follow Camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// Attach the camera to the end of the boom and let the boom adjust to match
	// the controller orientation
	FollowCamera->bUsePawnControlRotation = false;
	// Set our turn rates for input
	BaseTurnRate = 65.f;
	BaseLookUpRate = 10.f;

	SkillSpawningBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SkillSpawningBox"));
	SkillSpawningBox->SetupAttachment(GetRootComponent());


	// We don't want to rotate the character along with the rotation
	// Let that just affect the camera.
	// If you want to rotate your character along with camera, All of this value need to change for true.
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationPitch = false;

	// ?????? ?????? ?? ???????? ???????? ??
	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.f, 0.0f); // ... at this rotation rate // ?????? ?????? ??????????????
	GetCharacterMovement()->JumpZVelocity = 650.f; // Jump ???? 
	GetCharacterMovement()->AirControl = 0.2f; // Character can moving in the air 

	// Default Player Stats
	MaxHealth = 500.f;
	Health = 250.f;
	MaxStamina = 500.f;
	Stamina = 250.f;
	Coins = 0;

	RunningSpeed = 650.f;
	SprintingSpeed = 950.f;
	
	bShiftKeyDown = false;

	// Initialize Enums
	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminasStatus::ESS_Normal;

	StaminaDrainRate = 25.f;
	MinSprintStamina = 50.f;
	
	bLMBDown = false;
	IsMoveKeyDown = false;

	InterpSpeed = 15.f;
	bInterpToEnemy = false;

	bHasCombatTarget = false;
	
	bMovingForward = false;
	bMovingRight = false;

	bESCDown = false;
	bSkillKeyDown = false;
}

// Called when the game starts or when spawned
void AMain::BeginPlay()
{
	Super::BeginPlay();

	MainPlayerController = Cast<AMainPlayerController>(GetController());

	// ???? ?????? ?????? ?? ????, ????, ???? ?????? ?? ???????? ????. ???????? ???? x
	FString Map = GetWorld()->GetMapName();
	Map.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
	if (Map != "SunTemple") {
		LoadGameNoSwitch();
		if (MainPlayerController) {
			MainPlayerController->GameModeOnly();
		}
	}
}

// Called every frame
void AMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (MovementStatus == EMovementStatus::EMS_Dead) return;

	float DeltaStamina = StaminaDrainRate * DeltaTime;

	switch (StaminaStatus)
	{
		case EStaminasStatus::ESS_Normal:
			// ?????????? ???? ?????? ??
			if (bShiftKeyDown) {
				// ?????? ???? ????????
				if (Stamina - DeltaStamina <= MinSprintStamina) {
					// ???????? ?????????? ???????? ????????
					SetStaminaStatus(EStaminasStatus::ESS_BelowMinimum);
					Stamina -= DeltaStamina;
				}
				else {
					// ???? ?????????? ???????? ???? ???????? ???? x.
					Stamina -= DeltaStamina;
				}
				// ?????????? ?????? ?????????? 
				if (bMovingForward || bMovingRight) {
					SetMovementStatus(EMovementStatus::EMS_Sprinting);
				}
				else {
					SetMovementStatus(EMovementStatus::EMS_Normal);
				}
			}
			else {
				// ?????? ?? ?? ????????
				if (Stamina + DeltaStamina >= MaxStamina) {
					// ?????????? ?????? ??????????
					Stamina = MaxStamina; // ???? ???????? ??????
				}
				else {
					// ?????? ???????? ???????? ????????????
					Stamina += DeltaStamina;
				}
				// MovementStatus ?????????? 
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
			break;
		case EStaminasStatus::ESS_BelowMinimum:
			// ???????? ?????? ?????????? ????
			if (bShiftKeyDown) {
				// ?????? ???? ????????
				if (Stamina - DeltaStamina <= 0.f) {
					// ???????? <= 0.f ????
					SetStaminaStatus(EStaminasStatus::ESS_Exhausted);
					Stamina = 0.f;
					SetMovementStatus(EMovementStatus::EMS_Normal);
				}
				else {
					// 0 < ???????? <= BelowMinimun ????
					Stamina -= DeltaStamina;
					if (bMovingForward || bMovingRight) {
						SetMovementStatus(EMovementStatus::EMS_Sprinting);
					}
					else {
						SetMovementStatus(EMovementStatus::EMS_Normal);
					}
				}
			}
			else {
				// ?????? ?? ?? ????????
				if (Stamina + DeltaStamina >= MinSprintStamina) {
					// ???????????? >= Min ?? ?????? 
					SetStaminaStatus(EStaminasStatus::ESS_Normal);
					Stamina += DeltaStamina;
				}
				else {
					// ???? ???? ???? < min ??????
					Stamina += DeltaStamina;
				}
				// ???? ?????? ?????? ????
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
			break;
		case EStaminasStatus::ESS_Exhausted:
			if (bShiftKeyDown) {
				Stamina = 0.f;
			}
			else {
				SetStaminaStatus(EStaminasStatus::ESS_ExhaustedRecovering);
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
			break;
		case EStaminasStatus::ESS_ExhaustedRecovering:
			if (Stamina + DeltaStamina >= MinSprintStamina) {
				SetStaminaStatus(EStaminasStatus::ESS_Normal);
				Stamina += DeltaStamina;
			}
			else {
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
			break;
		default:
			;
	}

	if (bInterpToEnemy && CombatTarget) {
		// ?? ?????? ?????? ???????? ???? ???? : ???? ???? ???????? ?????? ????????
		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed);
		
		SetActorRotation(InterpRotation);
	}

	if (CombatTarget) {
		CombatTargetLocation = CombatTarget->GetActorLocation();
		if (MainPlayerController) {
			if (CombatTarget->IsBoss == false) {
				MainPlayerController->EnemyLocation = CombatTargetLocation;
			}
			else {
				MainPlayerController->BossEnemyLocation = CombatTargetLocation;
			}
		}
	}
}

FRotator AMain::GetLookAtRotationYaw(FVector Target)
{
	// FindLookAtRotation : Find a rotation for an object at Start location to point at Target location.
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);
	FRotator LookAtRotationYaw(0.f, LookAtRotation.Yaw, 0.f);
	return LookAtRotationYaw;
}

// Called to bind functionality to input
void AMain::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent); // PlayerInputComponent?? false???? ???? ????

	// ????
	// IE_Pressed : ?????? ??
	// IE_Released : ???? ??
	// ACharacter?? ???? ???????? ?? ?????? Jump?? ???????? ????.
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMain::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMain::ShiftKeyDown);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMain::ShiftKeyUp);

	PlayerInputComponent->BindAction("LMB", IE_Pressed, this, &AMain::LMBDown);
	PlayerInputComponent->BindAction("LMB", IE_Released, this, &AMain::LMBUp);

	PlayerInputComponent->BindAction("ESC", IE_Pressed, this, &AMain::ESCDown);
	PlayerInputComponent->BindAction("ESC", IE_Released, this, &AMain::ESCUp);

	PlayerInputComponent->BindAction("Skill1", IE_Pressed, this, &AMain::Skill1Down);
	PlayerInputComponent->BindAction("Skill1", IE_Released, this, &AMain::SkillKeyUp);

	PlayerInputComponent->BindAction("Skill2", IE_Pressed, this, &AMain::Skill2Down);
	PlayerInputComponent->BindAction("Skill2", IE_Released, this, &AMain::SkillKeyUp);

	PlayerInputComponent->BindAction("Skill3", IE_Pressed, this, &AMain::Skill3Down);
	PlayerInputComponent->BindAction("Skill3", IE_Released, this, &AMain::SkillKeyUp);

	PlayerInputComponent->BindAction("Skill4", IE_Pressed, this, &AMain::Skill4Down);
	PlayerInputComponent->BindAction("Skill4", IE_Released, this, &AMain::SkillKeyUp);

	PlayerInputComponent->BindAction("Skill5", IE_Pressed, this, &AMain::Skill5Down);
	PlayerInputComponent->BindAction("Skill5", IE_Released, this, &AMain::SkillKeyUp);

	PlayerInputComponent->BindAction("Skill6", IE_Pressed, this, &AMain::Skill6Down);
	PlayerInputComponent->BindAction("Skill6", IE_Released, this, &AMain::SkillKeyUp);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &AMain::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMain::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &AMain::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMain::LookUp);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMain::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMain::LookUpAtRate);

}

bool AMain::CanMove(float Value)
{
	if (MainPlayerController) {
		// ???? 5???? ???? true?? ???? true ?????? ????.
		return Value != 0.0f &&
			!bAttacking &&
			MovementStatus != EMovementStatus::EMS_Dead &&
			!MainPlayerController->bPauseMenuVisible;
	}
	return false;
}

void AMain::Turn(float Value)
{
	if (CanMove(Value)) {
		AddControllerYawInput(Value);
	}
}

void AMain::LookUp(float Value)
{
	if (CanMove(Value)) {
		AddControllerPitchInput(Value);
	}
}


void AMain::MoveForward(float Value) {
	bMovingForward = false; // ???????? ???? false?? ??????
	if (CanMove(Value)) {
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		// ?????? ???????? ?????????? ???????? ?? ???? ??
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);

		bMovingForward = true; // ???????? ???? true?? ??????
	}
}

void AMain::MoveRight(float Value) {
	bMovingRight = false; // ???????? ???? false?? ??????
	if (CanMove(Value)) {
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);

		bMovingRight = true; // ???????? ???? true?? ??????
	}
}

void AMain::TurnAtRate(float Rate) {
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMain::LookUpAtRate(float Rate) {
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMain::LMBDown()
{
	bLMBDown = true;

	if (MovementStatus == EMovementStatus::EMS_Dead) return;

	if (MainPlayerController) {
		// PausMenu?? ???? ???????? ?????????? ?????? ???????? ???????? ????????..
		if (MainPlayerController->bPauseMenuVisible) return;
	}

	if (ActiveOverlappingItem) {
		// ActiveOverlappingItem?? ?????????? Main???????????? ??????->Item????
		// ActiveOverlappingItem = ???? ??????. ???????? ?????? ?????????? ????
		// ???? ???????????? ???????? ???????? ?????? ?? ??????????
		AWeapon* Weapon = Cast<AWeapon>(ActiveOverlappingItem); // ?????? ????
		if (Weapon) {
			Weapon->Equip(this); // ???? ????! 
			Weapon->SetMainReference(this);
			// ???? ?????????? ???????? ActiveOverlappingItem?? ???? ????????????
			SetActiveOverlappingItem(nullptr); // null?? ?????? ???? ?????? ?????? ?? ???? ??
		}
	}

	else if(EquippedWeapon)
	{
		Attack();
	}
}

void AMain::LMBUp()
{
	bLMBDown = false;
}

void AMain::ESCDown()
{
	bESCDown = true;
	if (MainPlayerController) {
		MainPlayerController->TogglePauseMenu();
	}
}

void AMain::ESCUp()
{
	bESCDown = false;
}

FVector AMain::GetSpawnPoint() {
	// GetScaledBoxExtent() : Box?? Extent?? ?????? ????
	FVector Extent = SkillSpawningBox->GetScaledBoxExtent();
	// GetComponentLocation() : Box?? Origin ???? ?????? ????
	FVector Origin = SkillSpawningBox->GetComponentLocation();
	// RandomPointInBoundingBox : ?? ???? ?????? ???????? ???????? ?? ???? ?????? ???? ?????? ???????? ?????? ???? ???? ???? ???? ???? ??????????.
	FVector Point = UKismetMathLibrary::RandomPointInBoundingBox(Origin, Extent);
	return Point;
}

void AMain::DecrementHealth(float Amount) {
	Health -= Amount;
	if (Health - Amount <= 0.f) {
		Die();
	}
}

void AMain::Die() {
	// ???? ???? ???? ???????? ???? ???? ?? ?????????? ???????? ?????? ????
	if (MovementStatus == EMovementStatus::EMS_Dead) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && CombatMontage) {
		AnimInstance->Montage_Play(CombatMontage, 1.0f);
		AnimInstance->Montage_JumpToSection("Death", CombatMontage);
	}
	SetMovementStatus(EMovementStatus::EMS_Dead);
}

void AMain::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;
}

void AMain::Jump()
{
	if (MainPlayerController) {
		// PausMenu?? ???? ???????? Jump????????..
		if (MainPlayerController->bPauseMenuVisible) return;
	}

	if (MovementStatus != EMovementStatus::EMS_Dead && !bSkillKeyDown) {
		ACharacter::Jump();
	}
}


void AMain::IncrementCoins(int32 Amount) {
	this->Coins += Amount;
}

void AMain::IncrementHealth(float Amount) {
	if (Health + Amount >= MaxHealth) {
		Health = MaxHealth;
		return;
	}
	Health += Amount;
}

void AMain::IncrementStamina(float Amount)
{
	if (Stamina + Amount >= MaxStamina) {
		Stamina = MaxStamina;
		return;
	}
	Stamina += Amount;
}

void AMain::SetMovementStatus(EMovementStatus Status) {
	MovementStatus = Status;
	if (MovementStatus == EMovementStatus::EMS_Sprinting) {
		GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
	}
	else {
		GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;
	}
}

void AMain::ShiftKeyDown()
{
	bShiftKeyDown = true;
}

void AMain::ShiftKeyUp()
{
	bShiftKeyDown = false;
}

void AMain::ShowPickupLocation() {
	// Debug ?? ???? 
	/*
	UObject* WorldContextObject : ???? ?????
	FVector const Center : ?? ???? ???????
	float Radius : ?? ?????????
	int32 Segments : ?? ?? ?????
	FLinearColor Color : ?? ?????
	float LifeTime : ?? ???????? ???????? ?????
	float Thickness : ?? ???????
	*/

	for (int32 i = 0; i < PickupLocations.Num(); i++) {
		UKismetSystemLibrary::DrawDebugSphere(this, PickupLocations[i] + FVector(0, 0, 100.f), 25.f, 8, FLinearColor::Red, 10.f, .5f);
	}
	// ???????????? ???? ???????? ???? ?????? ????
	for (FVector Location : PickupLocations) {
		// FVector Location = auto Location
		UKismetSystemLibrary::DrawDebugSphere(this, Location + FVector(0, 0, 100.f), 25.f, 8, FLinearColor::Green, 3.f, 2.f);
	}

}

void AMain::SetEquippedWeapon(AWeapon* WeaponToSet)
{
	
	if (EquippedWeapon) {
		// ???? ?????????????? ?????? ?????? ?? Destroy??????
		 EquippedWeapon->Destroy();
	}
	EquippedWeapon = WeaponToSet;
}

void AMain::Attack()
{
	if (!bAttacking && MovementStatus != EMovementStatus::EMS_Dead && !bSkillKeyDown) {
		bAttacking = true;
		SetInterpToEnemy(true);
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		// ?????? ?????? ????
		GetWorld()->GetFirstPlayerController()->PlayerCameraManager->PlayCameraShake(CameraShake, 1.f);

		if (AnimInstance && CombatMontage) {

			int32 Section = FMath::RandRange(1, 2);
			switch (Section)
			{
			case 1:
				AnimInstance->Montage_Play(CombatMontage, 2.2f);
				AnimInstance->Montage_JumpToSection("Attack_1", CombatMontage);
				if (Skill_1) {
					FTransform SkillTransForm = FTransform(GetActorRotation(), GetMesh()->GetSocketLocation("SkillSocket"), GetActorScale3D());
					GetWorld()->SpawnActor<ASkillBase>(Skill_1, SkillTransForm);
				}
				break;
			case 2:
				AnimInstance->Montage_Play(CombatMontage, 1.8f);
				AnimInstance->Montage_JumpToSection("Attack_2", CombatMontage);
				if (Skill_2) {
					FTransform SkillTransForm = FTransform(GetActorRotation(), GetMesh()->GetSocketLocation("SkillSocket"), GetActorScale3D());
					GetWorld()->SpawnActor<ASkillBase>(Skill_2, SkillTransForm);
				}
				break;

			/* 1,2???? ???? ???????????? ????.. 3???????? ???? ?? ?????? ?????? ????????. 
			case 3:
				AnimInstance->Montage_Play(CombatMontage, 2.8f);
				AnimInstance->Montage_JumpToSection("MeteoSkill", CombatMontage);
				if (Skill_3) {
					FTransform SkillTransForm = FTransform(GetActorRotation(), GetMesh()->GetSocketLocation("SkillSocket"), GetActorScale3D());
					GetWorld()->SpawnActor<ASkillBase>(Skill_3, SkillTransForm);
				}
				break;*/
			default:
				;
			}
		}
	}
}

void AMain::AttackEnd()
{
	// ?????????? ?????????????? ???????? ???? ????
	bAttacking = false;
	SetInterpToEnemy(false);
	if (bLMBDown) {
		// ?????????? ???? ?????? ???? ???????? ???? ????
		Attack();
	}
}

void AMain::PlaySwingSound()
{
	if (EquippedWeapon->SwingSound) {
		UGameplayStatics::PlaySound2D(this, EquippedWeapon->SwingSound);
	}
}

void AMain::SetInterpToEnemy(bool Interp)
{
	bInterpToEnemy = Interp;
}

float AMain::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Health -= DamageAmount;
	if (Health - DamageAmount <= 0.f) {
		Die();
		if (DamageCauser) {
			AEnemy* Enemy = Cast<AEnemy>(DamageCauser);
			if (Enemy) {
				Enemy->bHasValidTarget = false;
			}
		}
	}

	return DamageAmount;
}

void AMain::UpdateCombatTarget()
{
	// ???? ?????? ?? ???????? ????
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, EnemyFilter);

	if (OverlappingActors.Num() == 0) {
		if (MainPlayerController) {
			MainPlayerController->RemoveEnemyHealthBar();
			MainPlayerController->RemoveBossEnemyHealthBar();
		}
		return;
	}
	AEnemy* ClosestEnemy = Cast<AEnemy>(OverlappingActors[0]);
	if (ClosestEnemy) {
		float MinDistance = (ClosestEnemy->GetActorLocation() - GetActorLocation()).Size();

		for (auto Actor : OverlappingActors) {
			AEnemy* Enemy = Cast<AEnemy>(Actor);
			if (Enemy) {
				float DistanceToActor = (Enemy->GetActorLocation() - GetActorLocation()).Size();
				if (MinDistance > DistanceToActor) {
					MinDistance = DistanceToActor;
					ClosestEnemy = Enemy;
				}
			}
		}
		if (MainPlayerController) {
			if(ClosestEnemy->IsBoss == false){
				MainPlayerController->DisplayEnemyHealthBar();
			}
			else {
				// ?????? ????
				MainPlayerController->DisplayBossEnemyHealthBar();
			}
			
		}
		SetCombatTarget(ClosestEnemy);
		bHasCombatTarget = true;
	}
}

void AMain::SwitchLevel(FName LevelName)
{
	UWorld* World = GetWorld();
	if (World) {
		FString CurrentLevel = World->GetMapName();
		CurrentLevel.RemoveFromStart(World->GetMapName());
		// FName?? FString???? ?????? ???? but ?????? ???? 
		// ???? ?????? ???? ???? ???????? ????????
		FName CurrentLevelName(*CurrentLevel);
		if (CurrentLevelName != LevelName) {
			UGameplayStatics::OpenLevel(World, LevelName);
			LoadGameNoSwitch();
		}
	}
}

void AMain::SaveGame()
{
	/*
	UFirstSaveGame::StaticClass() :  USaveGame ???????? ????
	 UGameplayStatics::CreateSaveGameObject(UFirstSaveGame::StaticClass());
	 CreateSaveGameObject : ???????? ?????? ???? ???? ?? SaveGame ?????? ???? ???? SaveGameToSlot?? ??????????.
	*/
	UFirstSaveGame* SaveGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::CreateSaveGameObject(UFirstSaveGame::StaticClass()));
	SaveGameInstance->CharacterStats.Health = Health;
	SaveGameInstance->CharacterStats.MaxHealth = MaxHealth;
	SaveGameInstance->CharacterStats.Stamina = Stamina;
	SaveGameInstance->CharacterStats.MaxStamina = MaxStamina;
	SaveGameInstance->CharacterStats.Coins = Coins;
	SaveGameInstance->CharacterStats.Location = GetActorLocation();
	SaveGameInstance->CharacterStats.Rotation = GetActorRotation();
	
	FString MapName = GetWorld()->GetMapName();
	// LOG?????? ?? ?????? UEDPIE_0_SunTemple ?????????? ???? ???????? ???????? ???? ??????????
	MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	SaveGameInstance->CharacterStats.LevelName = MapName;

	if (EquippedWeapon) {
		// Main.cpp???????? FirstSaveGame.cpp?? ?????? Weapon.cpp?????? ?????? 
		SaveGameInstance->CharacterStats.WeaponName = EquippedWeapon->Name;
	}
	/*
	???? ?? ???????? ???????? ???????? ????
	SaveGameObject?? ?????? ???????? ???? ????/?????? ??????????.
	@note ?????? ???? ???????? ?????? ?????? ?????? SaveGame ???? ???????? ???????? ????????
	*/
	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->PlayerName, SaveGameInstance->UserIndex);
}

void AMain::LoadGame(bool SetPosition)
{
	UE_LOG(LogTemp, Warning, TEXT("****************************LoadGame Begin****************************"));
	UFirstSaveGame* LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::CreateSaveGameObject(UFirstSaveGame::StaticClass()));

	LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	Health = LoadGameInstance->CharacterStats.Health;
	MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
	Stamina = LoadGameInstance->CharacterStats.Stamina;
	MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;
	Coins = LoadGameInstance->CharacterStats.Coins;

	if (WeaponStorage) {
		AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);
		if (Weapons) {
			FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;

			if (Weapons->WeaponMap.Contains(WeaponName)) {
				// TMap?? Key???? WeaponName?? ???? ????????
				// Weapons->WeaponMap[WeaponName] -> Key : WeaponName  return : ???????? value??
				AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);
				WeaponToEquip->Equip(this);
			}
			
		}
	}
	/*
	???? ?????? ???? ?????????? ???? Save Load??????????. 
	???????? ???? ?????? ????????
	 ?????? ?????? ?????????? ?????? ?????? ????
	*/
	if (SetPosition) {
		// SetPosition?? true???? ?? ?????????? ???????? ??.
		SetActorLocation(LoadGameInstance->CharacterStats.Location);
		SetActorRotation(LoadGameInstance->CharacterStats.Rotation);
	 }

	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;
	if (LoadGameInstance->CharacterStats.LevelName != TEXT("") && LoadGameInstance->CharacterStats.LevelName != TEXT("SunTemple")) {
		FName LevelName(*LoadGameInstance->CharacterStats.LevelName);
		SwitchLevel(LevelName);
	}
}

void AMain::LoadGameNoSwitch()
{
	UE_LOG(LogTemp, Warning, TEXT("****************************LoadGameNoSwitch Begin****************************"));
	UFirstSaveGame* LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::CreateSaveGameObject(UFirstSaveGame::StaticClass()));

	LoadGameInstance = Cast<UFirstSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->UserIndex));

	Health = LoadGameInstance->CharacterStats.Health;
	MaxHealth = LoadGameInstance->CharacterStats.MaxHealth;
	Stamina = LoadGameInstance->CharacterStats.Stamina;
	MaxStamina = LoadGameInstance->CharacterStats.MaxStamina;
	Coins = LoadGameInstance->CharacterStats.Coins;

	if (WeaponStorage) {
		AItemStorage* Weapons = GetWorld()->SpawnActor<AItemStorage>(WeaponStorage);
		if (Weapons) {
			FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;

			if (Weapons->WeaponMap.Contains(WeaponName)) {
				// TMap?? Key???? WeaponName?? ???? ????????
				// Weapons->WeaponMap[WeaponName] -> Key : WeaponName  return : ???????? value??
				AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Weapons->WeaponMap[WeaponName]);
				WeaponToEquip->Equip(this);
			}

		}
	}

	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetMesh()->bPauseAnims = false;
	GetMesh()->bNoSkeletonUpdate = false;
}


// ====================================== Skill Setting ===============================================
void AMain::SkillKeyUp()
{
	bSkillKeyDown = false;
}


void AMain::Skill1Down()
{
	bSkillKeyDown = true;
	if (MainPlayerController) {
		// PausMenu?? ???? ???????? Jump????????..
		if (MainPlayerController->bPauseMenuVisible) return;
	}

	if (!bAttacking && MovementStatus != EMovementStatus::EMS_Dead) {
		bAttacking = true;
		SetInterpToEnemy(true);
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && CombatMontage) {
			AnimInstance->Montage_Play(CombatMontage, 2.2f);
			AnimInstance->Montage_JumpToSection("MeteoSkill", CombatMontage);
			if (Skill_3 && Skill_3_2) {
				// ?????? ?????? ?????????? ???? ?????? ????
				for (int i = 0; i < 5; i++) {
					FVector RandLocation = GetSpawnPoint();
					FTransform SkillTransForm = FTransform(GetActorRotation(), RandLocation, GetActorScale3D());
					// FTransform SkillTransForm2 = FTransform(GetActorRotation(), RandLocation, GetActorScale3D());
					GetWorld()->SpawnActor<ASkillBase>(Skill_3, SkillTransForm); // ?????? ???? ??????x
					GetWorld()->SpawnActor<ASkillBase>(Skill_3_2, SkillTransForm); // ?????? ??????o
					// ?????? ?????? ????
					GetWorld()->GetFirstPlayerController()->PlayerCameraManager->PlayCameraShake(CameraShake, 1.f);
				}
			}
		}
	}

}
void AMain::Skill2Down()
{
	bSkillKeyDown = true;
	if (MainPlayerController) {
		if (MainPlayerController->bPauseMenuVisible) return;
	}
	if (!bAttacking && MovementStatus != EMovementStatus::EMS_Dead) {
		bAttacking = true;
		SetInterpToEnemy(true);
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance) {
			AnimInstance->Montage_Play(CombatMontage, 2.2f);
			AnimInstance->Montage_JumpToSection("BuffSkill", CombatMontage);
		}
		if (Skill_4) {
			FTransform SkillTransForm = FTransform(GetActorRotation(), GetMesh()->GetSocketLocation("BuffSkillLocation"), GetActorScale3D());
			GetWorld()->SpawnActor<ASkillBase>(Skill_4, SkillTransForm);
		}
	}
}


void AMain::Skill3Down() {
	bSkillKeyDown = true;
	if (MainPlayerController) {
		if (MainPlayerController->bPauseMenuVisible) return;
	}
	if (!bAttacking && MovementStatus != EMovementStatus::EMS_Dead) {
		bAttacking = true;
		SetInterpToEnemy(true);
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance) {
			AnimInstance->Montage_Play(CombatMontage, 1.8f);
			AnimInstance->Montage_JumpToSection("BuffSkill", CombatMontage);
		}
		if (Skill_5) {
			FTransform SkillTransForm = FTransform(GetActorRotation(), GetMesh()->GetSocketLocation("SkillSocket"), GetActorScale3D());
			GetWorld()->SpawnActor<ASkillBase>(Skill_5, SkillTransForm);
		}
	}
}
void AMain::Skill4Down() {
	bSkillKeyDown = true;
	if (MainPlayerController) {
		if (MainPlayerController->bPauseMenuVisible) return;
	}
	if (!bAttacking && MovementStatus != EMovementStatus::EMS_Dead) {
		bAttacking = true;
		SetInterpToEnemy(true);
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance) {
			AnimInstance->Montage_Play(CombatMontage, 2.2f);
			AnimInstance->Montage_JumpToSection("BuffSkill", CombatMontage);
		}
		if (Skill_6) {
			if (CombatTarget) {
				FTransform SkillTransForm = FTransform(GetActorRotation(), CombatTarget->GetMesh()->GetSocketLocation("MainSkillSpawnLocation"), GetActorScale3D());
				GetWorld()->SpawnActor<ASkillBase>(Skill_6, SkillTransForm);
			}
			else return;
		}
	}
}
void AMain::Skill5Down() {
	bSkillKeyDown = true;
	if (MainPlayerController) {
		if (MainPlayerController->bPauseMenuVisible) return;
	}
	if (!bAttacking && MovementStatus != EMovementStatus::EMS_Dead) {
		bAttacking = true;
		SetInterpToEnemy(true);
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance) {
			AnimInstance->Montage_Play(CombatMontage, 2.2f);
			AnimInstance->Montage_JumpToSection("BuffSkill", CombatMontage);
		}
		if (Skill_7) {
			if (CombatTarget) {
				FTransform SkillTransForm = FTransform(GetActorRotation(), CombatTarget->GetMesh()->GetSocketLocation("MainSkillSpawnLocation"), GetActorScale3D());
				GetWorld()->SpawnActor<ASkillBase>(Skill_7, SkillTransForm);
			}
			else return;
		}
	}
}
void AMain::Skill6Down() {
	bSkillKeyDown = true;
	if (MainPlayerController) {
		if (MainPlayerController->bPauseMenuVisible) return;
	}
	if (!bAttacking && MovementStatus != EMovementStatus::EMS_Dead) {
		bAttacking = true;
		SetInterpToEnemy(true);
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance) {
			AnimInstance->Montage_Play(CombatMontage, 2.2f);
			AnimInstance->Montage_JumpToSection("BuffSkill", CombatMontage);
		}
		if (Skill_8) {
			if (CombatTarget) {
				FTransform SkillTransForm = FTransform(GetActorRotation(), CombatTarget->GetMesh()->GetSocketLocation("MainSkillSpawnLocation"), GetActorScale3D());
				GetWorld()->SpawnActor<ASkillBase>(Skill_8, SkillTransForm);
			}
			else return;
		}
	}
}