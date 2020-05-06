// copyright Kepos 2020

#include "EscapeGame.h"
#include "Grabber.h"
#include <Runtime\Engine\Public\DrawDebugHelpers.h>
#include <Runtime\Engine\Public\CollisionQueryParams.h>

#define OUT

// Sets default values for this component's properties
UGrabber::UGrabber()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

}


// Called when the game starts
void UGrabber::BeginPlay()
{
	Super::BeginPlay();
	FindPhysicsHandleComponent();
	SetupInputComponent();
}

// Called every frame
void UGrabber::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if(!PhysicsHandle) {return;}
	// if the physics handle is attached
	if(PhysicsHandle->GrabbedComponent){
		// move the object that we're holding
		PhysicsHandle->SetTargetLocation(GetReachLineEnd());
	}

}

// Look for atteached Physics Handle
void UGrabber::FindPhysicsHandleComponent(){
	PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	if (PhysicsHandle == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s missing physics handle component"), *GetOwner()->GetName());
	}
}

// Look for atteached Input Component (only appears at run time)
void UGrabber::SetupInputComponent(){
	InputComponent = GetOwner()->FindComponentByClass<UInputComponent>();
	if (InputComponent)
	{
		InputComponent->BindAction("Grab", IE_Pressed, this, &UGrabber::Grab);
		InputComponent->BindAction("Grab", IE_Released, this, &UGrabber::Release);
	}
}


void UGrabber::Grab(){
	//LINE TRACE and see if we reach any actors with physics body collision channel set
	auto HitResult = GetFirstPhysicsBodyInReach();
	auto ComponentToGrab = HitResult.GetComponent(); // gets the mesh in our case
	auto ActorHit = HitResult.GetActor();

	// If we his something then attach physics handle
	if(ActorHit != nullptr)
	{
		if(!PhysicsHandle) {return;}
		//attach physics handle
		PhysicsHandle->GrabComponent(
			ComponentToGrab, 
			NAME_None,// no bones needed
			ComponentToGrab->GetOwner()->GetActorLocation(),
			true // allow rotation
			);
	}
	
}

void UGrabber::Release(){
	if(!PhysicsHandle) {return;}
	PhysicsHandle->ReleaseComponent();
}


const FHitResult UGrabber::GetFirstPhysicsBodyInReach(){
	// Line-trace (AKA ray-cast) out to reach distance
	FHitResult HitResult;
	FCollisionQueryParams TraceParameters(FName(TEXT("")), false, GetOwner());
	GetWorld()->LineTraceSingleByObjectType(
		OUT	HitResult,
		GetReachLineStart(),
		GetReachLineEnd(),
		FCollisionObjectQueryParams(ECollisionChannel::ECC_PhysicsBody),
		TraceParameters
	);
	return HitResult;
}


FVector UGrabber::GetReachLineStart(){
	// Get player view point this tick
	FVector PlayerViewPointLocation;
	FRotator PlayerViewPointRotation;
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(
		OUT PlayerViewPointLocation,
		OUT PlayerViewPointRotation
	);
	// Draw a red trace in the world to visual
	return PlayerViewPointLocation;
}

FVector UGrabber::GetReachLineEnd(){
	// Get player view point this tick
	FVector PlayerViewPointLocation;
	FRotator PlayerViewPointRotation;

	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(
		OUT PlayerViewPointLocation,
		OUT PlayerViewPointRotation
	);

	// Draw a red trace in the world to visual
	return PlayerViewPointLocation + PlayerViewPointRotation.Vector() * Reach;
}