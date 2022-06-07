// Fill out your copyright notice in the Description page of Project Settings.

#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Legend/Hero/Components/ParkourComponent.h"

#pragma region General
UParkourComponent::UParkourComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UParkourComponent::BeginPlay()
{
	Super::BeginPlay();

	Actor = GetOwner();
	if (!Actor) {
		UE_LOG(LogTemp, Error, TEXT("No owner ref"));
		return;
	}

	TraceCollisionParams.AddIgnoredActor(Actor);
	Collider = Actor->FindComponentByClass<UCapsuleComponent>();
	CharacterMovement = Actor->FindComponentByClass<UCharacterMovementComponent>();
}

bool UParkourComponent::TryParkour(bool bIsAutoCall) {
	if (bIsBusy)
		return false;

	ActorFeet = Actor->GetActorLocation() + FVector::DownVector * RootHeight;

	// Obstacle traces
	RunObstacleTraces();

	bool bObstacleFound =
		LowObstacleTraceResult.bBlockingHit ||
		MidObstacleTraceResult.bBlockingHit ||
		HighObstacleTraceResult.bBlockingHit;

	DebugTrace(LowObstacleTraceResult);
	DebugTrace(MidObstacleTraceResult);
	DebugTrace(HighObstacleTraceResult);

	if (!bObstacleFound)
		return false;

	// Height trace
	RunHeightTrace();

	if (HeightTraceResult.bBlockingHit)
		ObstacleHeight = HeightTraceResult.ImpactPoint.Z - ActorFeet.Z;

	DebugTrace(HeightTraceResult);

	// Depth trace
	RunDepthTrace();
	DebugTrace(DepthTraceResult);

	bool bShouldClimb = ShouldClimb(CanClimb(), bIsAutoCall);
	bool bShouldVault = ShouldVault(CanVault(), bIsAutoCall);

	if (bShouldClimb) {
		UE_LOG(LogTemp, Warning, TEXT("Climb"));
	} else if (bShouldVault) {
		UE_LOG(LogTemp, Warning, TEXT("Vault"));
	}

	return false;
}

void UParkourComponent::RunObstacleTraces() {
	float MidObstacleTraceHeight = (MaxObstacleHeightToParkour + MinObstacleHeightToParkour) / 2;

	TraceForwardFromActor(LowObstacleTraceResult, MinObstacleHeightToParkour, ObstacleTraceRange);
	TraceForwardFromActor(MidObstacleTraceResult, MidObstacleTraceHeight, ObstacleTraceRange);
	TraceForwardFromActor(HighObstacleTraceResult, MaxObstacleHeightToParkour, ObstacleTraceRange);
}

void UParkourComponent::RunHeightTrace() {
	// Set height at which to start trace
	HeightTraceStartHeight = MaxObstacleHeightToParkour + 10;
	if (LowObstacleTraceResult.bBlockingHit || MidObstacleTraceResult.bBlockingHit)
		HeightTraceStartHeight = ActorHeight;

	float HeightTraceRange = HeightTraceStartHeight - ActorHeight;

	TraceDownAheadOfActor(HeightTraceResult, HeightTraceStartHeight, HeightTraceDepth, HeightTraceRange);
}

void UParkourComponent::RunDepthTrace() {
	float DepthTraceRange = HeightTraceStartHeight - ActorHeight + 20; // +20 to not hit ground

	TraceDownAheadOfActor(DepthTraceResult, HeightTraceStartHeight, MaxObstacleDepthToVault, DepthTraceRange);
}


#pragma endregion


#pragma region Climbing
bool UParkourComponent::CanClimb() {
	// Obstacle must be within height parameters
	if (ObstacleHeight > MaxObstacleHeightToParkour)
		return false;

	// Obstacle must have enough depth
	if (!DepthTraceResult.bBlockingHit)
		return false;

	return true;
}

bool UParkourComponent::ShouldClimb(bool bCanClimb, bool bIsAutoCall) {
	if (!bCanClimb)
		return false;

	// Check auto settings
	if (bIsAutoCall) {
		if (!bCanAutoClimb)
			return false;

		if (ObstacleHeight > MaxObstacleHeightToAutoClimb)
			return false;
	}

	return true;
}
#pragma endregion



#pragma region Vaulting
bool UParkourComponent::CanVault() {
	// Obstacle must be within height parameters
	if (ObstacleHeight > MaxObstacleHeightToParkour)
		return false;

	// Obstacle must be short in depth
	if (DepthTraceResult.bBlockingHit)
		return false;

	return true;

	return false;
}

bool UParkourComponent::ShouldVault(bool bCanVault, bool bIsAutoCall) {
	if (!bCanVault)
		return false;

	// Check auto settings
	if (bIsAutoCall) {
		if (!bCanAutoVault)
			return false;

		if (ObstacleHeight > MaxObstacleHeightToAutoVault)
			return false;
	}

	return true;
}
#pragma endregion



#pragma region Utility
void UParkourComponent::TraceForwardFromActor(FHitResult& TraceResult, float TraceHeight, float TraceRange) {
	FVector TraceStart = ActorFeet + FVector::UpVector * TraceHeight;
	FVector TraceEnd = TraceStart + Actor->GetActorForwardVector() * TraceRange;

	GetWorld()->LineTraceSingleByChannel(
		TraceResult,
		TraceStart,
		TraceEnd,
		ECollisionChannel::ECC_WorldStatic,
		TraceCollisionParams
	);
}

void UParkourComponent::TraceDownAheadOfActor(FHitResult& TraceResult, float TraceHeight, float TraceDepth, float TraceRange) {
	FVector ActorForward = Actor->GetActorForwardVector();
	FVector TraceStart = ActorFeet + (FVector::UpVector * TraceHeight) + (ActorForward * TraceDepth);
	FVector TraceEnd = TraceStart + FVector::DownVector * TraceRange;

	GetWorld()->LineTraceSingleByChannel(
		TraceResult,
		TraceStart,
		TraceEnd,
		ECollisionChannel::ECC_WorldStatic,
		TraceCollisionParams
	);
}

void UParkourComponent::DebugTrace(FHitResult TraceResult, bool bPersist, float LifeTime) {

	if (!bUseDebug)
		return;

	bool bHit = TraceResult.bBlockingHit;

	//if (!bHit)
	//	return;

	FColor Color = bHit ? FColor::Red : FColor::Blue;
	FVector TraceEnd = bHit ? TraceResult.ImpactPoint : TraceResult.TraceEnd;

	DrawDebugLine(GetWorld(), TraceResult.TraceStart, TraceEnd, Color, bPersist, LifeTime);

	if (bHit)
		DrawDebugSphere(GetWorld(), TraceEnd, 10, 10, Color, bPersist, LifeTime);
}
#pragma endregion
