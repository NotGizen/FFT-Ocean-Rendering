// Fill out your copyright notice in the Description page of Project Settings.


#include "Waves.h"
#include "ProceduralMeshComponent.h"
// Sets default values
AWaves::AWaves()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	TileSize = 1000.f;
	AmountOfTiles = 100;
	int32 PerRowAmount = 10;
	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;
	int32 NameCounter = 0;
	
	for (int32 i = 0; i < PerRowAmount; ++i)
	{
		for (int32 j = 0; j < PerRowAmount; ++j)
		{
			
			FName CompName = FName(*FString::Printf(TEXT("ProceduralMeshComponent_%d"), NameCounter));
			UProceduralMeshComponent* MeshComp = CreateDefaultSubobject<UProceduralMeshComponent>(CompName);
			MeshComp->SetupAttachment(RootComponent);
			MeshComp->SetRelativeLocation(FVector(i * TileSize, j * TileSize, 0.0f));
			CreateGrid(TileSize, PerRowAmount * PerRowAmount, MeshComp);
			NameCounter++;
		}
	}
	
	//Static mesh (Delete later)
	// VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	// VisualMesh->SetupAttachment(RootComponent);
	// 	
	// static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneVisualAsset(TEXT("/Engine/BasicShapes/Plane.Plane"));
	// 	
	// if (PlaneVisualAsset.Succeeded())
	// {
	// 	VisualMesh->SetStaticMesh(PlaneVisualAsset.Object);
	// 	FTransform NewTransform = FTransform::Identity;
	// 	NewTransform.SetLocation(FVector(0.0f, 0.0f, 0.0f));
	// 	NewTransform.SetScale3D(FVector(10.0f));
	// 	VisualMesh->SetRelativeLocation(NewTransform.GetLocation());
	// 	VisualMesh->SetRelativeTransform(NewTransform);
	// }
	
		
}

// Called when the game starts or when spawned
void AWaves::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWaves::CreateGrid(float tileSize, int32 amount, UProceduralMeshComponent*& mesh)
{
	
	
	TArray<FVector> Vertices;
	Vertices.Add(FVector(0.0f , 0.0f, 0.0f));
	Vertices.Add(FVector(0.0f , tileSize , 0.0f));
	Vertices.Add(FVector(tileSize , 0.0f , 0.0f));
	Vertices.Add(FVector(tileSize , tileSize , 0.0f));
	
	TArray<int32> Triangles;
	Triangles.Add(0);
	Triangles.Add(1);
	Triangles.Add(2);
	
	Triangles.Add(2);
	Triangles.Add(1);
	Triangles.Add(3);
	
	TArray<FVector> Normals;
	for (int32 i = 0; i < Vertices.Num(); ++i)
	{
		Normals.Add(FVector(0, 0, 1)); 
	}
	
	TArray<FVector2D> UVs;
	UVs.Add(FVector2D(0, 0));
	UVs.Add(FVector2D(0, 1));
	UVs.Add(FVector2D(1, 0));
	UVs.Add(FVector2D(1, 1));
	
	TArray<FColor> Colors;
	Colors.Add(FColor(255, 0, 0, 255));
	Colors.Add(FColor(0, 255, 0, 255));	
	Colors.Add(FColor(0, 0, 255, 255));
	Colors.Add(FColor(255, 255, 0, 255));
	
	if (mesh)
	{
		mesh->ClearAllMeshSections();
		mesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, Colors, TArray<FProcMeshTangent>(), false);
		Meshes.Add(mesh);
	}
	
}

// Called every frame
void AWaves::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FVector NewLocation = GetActorLocation();
	FRotator NewRotation = GetActorRotation();
	float RunningTime = GetGameTimeSinceCreation();
	float DeltaHeight = (FMath::Sin(RunningTime + DeltaTime) - FMath::Sin(RunningTime));
	NewLocation.Z += DeltaHeight * 20.0f;       //Scale our height by a factor of 20
	float DeltaRotation = DeltaTime * 20.0f;	//Rotate by 20 degrees per second
	NewRotation.Yaw += DeltaRotation;
	SetActorLocationAndRotation(NewLocation, NewRotation);
}

