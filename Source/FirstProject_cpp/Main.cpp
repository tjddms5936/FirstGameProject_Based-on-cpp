// Fill out your copyright notice in the Description page of Project Settings.


#include "Main.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"


// Sets default values
AMain::AMain()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Create Camera Boom (pulls towards the player if there's a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 600.f; // Camera follows at this distance
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
	BaseLookUpRate = 65.f;

	// We don't want to rotate the character along with the rotation
	// Let that just affect the camera.
	// If you want to rotate your character along with camera, All of this value need to change for true.
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationPitch = false;

	// ����Ű ������ �� �������� ĳ���Ͱ� ��
	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.f, 0.0f); // ... at this rotation rate // �󸶳� ������ ����ȸ���ϴ���
	GetCharacterMovement()->JumpZVelocity = 650.f; // Jump ���� 
	GetCharacterMovement()->AirControl = 0.2f; // Character can moving in the air 

	// Default Player Stats
	MaxHealth = 100.f;
	Health = 65.f;
	MaxStamina = 150.f;
	Stamina = 120.f;
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
}

// Called when the game starts or when spawned
void AMain::BeginPlay()
{
	Super::BeginPlay();

	 
}

// Called every frame
void AMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	float DeltaStamina = StaminaDrainRate * DeltaTime;

	switch (StaminaStatus)
	{
		case EStaminasStatus::ESS_Normal:
			// ���׹̳ʰ� ���� ������ ��
			if (bShiftKeyDown) {
				// �޸��� ���� ���¿���
				if (Stamina - DeltaStamina <= MinSprintStamina) {
					// �����ִ� ���׹̳��� �������� ���϶��
					SetStaminaStatus(EStaminasStatus::ESS_BelowMinimum);
					Stamina -= DeltaStamina;
				}
				else {
					// ���� �̻��̶�� ���׹̳� ���� ������Ʈ �ʿ� x.
					Stamina -= DeltaStamina;
				}
				// �޸����ִ� ���·� ��ȯ���ֱ� 
				SetMovementStatus(EMovementStatus::EMS_Sprinting);
			}
			else {
				// ����Ʈ Ű �� ���¿���
				if (Stamina + DeltaStamina >= MaxStamina) {
					// �߰��Ϸ��� �ִ�ġ �̻��̶��
					Stamina = MaxStamina; // �׳� �ִ�ġ�� �ʱ�ȭ
				}
				else {
					// �ִ�ġ �ƴ϶�� ���׹̳� ȸ�������ֱ�
					Stamina += DeltaStamina;
				}
				// MovementStatus �������ֱ� 
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
			break;
		case EStaminasStatus::ESS_BelowMinimum:
			// ���׹̳� ���°� ���������� ����
			if (bShiftKeyDown) {
				// �޸��� ���� ���¿���
				if (Stamina - DeltaStamina <= 0.f) {
					// ���׹̳� <= 0.f �϶�
					SetStaminaStatus(EStaminasStatus::ESS_Exhausted);
					Stamina = 0.f;
					SetMovementStatus(EMovementStatus::EMS_Normal);
				}
				else {
					// 0 < ���׹̳� <= BelowMinimun �϶�
					Stamina -= DeltaStamina;
					SetMovementStatus(EMovementStatus::EMS_Sprinting);
				}
			}
			else {
				// ����Ʈ Ű �� ���¿���
				if (Stamina + DeltaStamina >= MinSprintStamina) {
					// �߰�ȸ���ؼ� >= Min �� �ȴٸ� 
					SetStaminaStatus(EStaminasStatus::ESS_Normal);
					Stamina += DeltaStamina;
				}
				else {
					// �߰� ȸ�� �ص� < min �̶��
					Stamina += DeltaStamina;
				}
				// � ���µ� �ȴ°� ����
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
}

// Called to bind functionality to input
void AMain::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent); // PlayerInputComponent�� false��� �ڵ� ����

	// ����
	// IE_Pressed : ������ ��
	// IE_Released : ���� ��
	// ACharacter�� �̹� ����߰� �� �ȿ��� Jump�� �����Ǿ� ����.
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMain::ShiftKeyDown);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMain::ShiftKeyUp);

	PlayerInputComponent->BindAction("LMB", IE_Pressed, this, &AMain::LMBDown);
	PlayerInputComponent->BindAction("LMB", IE_Released, this, &AMain::LMBUp);




	PlayerInputComponent->BindAxis("MoveForward", this, &AMain::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMain::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMain::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMain::LookUpAtRate);

}


void AMain::MoveForward(float Value) {
	if (Controller != nullptr && Value != 0.0f && (!bAttacking)) {
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		// ī�޶� ��ȯ�Ǹ� �ڿ������� ĳ���͵� �� ���� ��
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AMain::MoveRight(float Value) {
	if (Controller != nullptr && Value != 0.0f && (!bAttacking)) {
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
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
	if (ActiveOverlappingItem) {
		// ActiveOverlappingItem�� ���ӽ����� Main�������¿��� ������->Item����
		// ActiveOverlappingItem = ���� ������. �׷��ٰ� ����� �������Ǹ� �ٲ�
		// ���� ���ʸ��콺�� �����µ� �����۰� ������ �� ��Ȳ�̶��
		AWeapon* Weapon = Cast<AWeapon>(ActiveOverlappingItem); // ����ȯ �ϰ�
		if (Weapon) {
			UE_LOG(LogTemp, Warning, TEXT("Weapon Equip!"));
			Weapon->Equip(this); // ���� ����! 
			// �ؿ� �������ϸ� �����ϸ� ActiveOverlappingItem�� ��� �����Ǿ�����
			SetActiveOverlappingItem(nullptr); // null�� ���༭ �ٸ� ���⵵ ������ �� �ְ� ��
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



void AMain::DecrementHealth(float Amount) {
	Health -= Amount;
	if (Health - Amount <= 0.f) {
		Die();
	}
}

void AMain::Die() {

}

void AMain::IncrementCoins(int32 Amount) {
	this->Coins += Amount;
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
	// Debug �� ���� 
	/*
	UObject* WorldContextObject : ��� ����?
	FVector const Center : �� ���� ��ġ��?
	float Radius : �� ��������?
	int32 Segments : �� �� ����?
	FLinearColor Color : �� ����?
	float LifeTime : �� �����ϰ� ���ʵ��� ����?
	float Thickness : �� �β���?
	*/

	for (int32 i = 0; i < PickupLocations.Num(); i++) {
		UKismetSystemLibrary::DrawDebugSphere(this, PickupLocations[i] + FVector(0, 0, 100.f), 25.f, 8, FLinearColor::Red, 10.f, .5f);
	}
	// �����̳ʿ��� ���� ������� ���� �ݺ��� ����
	for (FVector Location : PickupLocations) {
		// FVector Location = auto Location
		UKismetSystemLibrary::DrawDebugSphere(this, Location + FVector(0, 0, 100.f), 25.f, 8, FLinearColor::Green, 3.f, 2.f);
	}

}

void AMain::SetEquippedWeapon(AWeapon* WeaponToSet)
{
	
	if (EquippedWeapon) {
		// �̹� �����Ǿ��ִ°� �ִٸ� ������ �� Destroy�ع���
		 EquippedWeapon->Destroy();
		 UE_LOG(LogTemp, Warning, TEXT("Weapon Destroy"));
	}
	EquippedWeapon = WeaponToSet;
}

void AMain::Attack()
{
	if (!bAttacking) {
		bAttacking = true;

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && CombatMontage) {

			int32 Section = FMath::RandRange(0, 1);
			switch (Section)
			{
			case 0:
				AnimInstance->Montage_Play(CombatMontage, 2.2f);
				AnimInstance->Montage_JumpToSection("Attack_1", CombatMontage);
				break;
			case 1:
				AnimInstance->Montage_Play(CombatMontage, 1.8f);
				AnimInstance->Montage_JumpToSection("Attack_2", CombatMontage);
				break;
			default:
				;
			}
		}
	}
	
}

void AMain::AttackEnd()
{
	bAttacking = false;
	if (bLMBDown) {
		// ���ʸ��콺 ��ư ������ �ִ� ���¶�� ��� ����
		Attack();
	}
}