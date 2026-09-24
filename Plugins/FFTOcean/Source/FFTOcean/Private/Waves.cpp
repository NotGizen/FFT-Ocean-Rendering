// Fill out your copyright notice in the Description page of Project Settings.


#include "Waves.h"
#include "../Public/WavesRender.h"
#include "ProceduralMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
// Sets default values
AWaves::AWaves()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	TileSize = 100.f;
	
	//Set default params
	
	Scale = 1.0;
	Alpha = 1.0f;
	Angle = 0.0f;
	Gamma = 1.0f;
	PeakOmega = 0.1f;
	ShortWavesFade = 1.0f;
	SpreadBlend = 1.0f;
	Swell = 1.0f;
	
	
	PerRowAmount = 10;
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
			CreateGrid(TileSize, i, j, MeshComp);
			NameCounter++;
		}
	}
	
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> OceanMatFinder(
		TEXT("/FFTOcean/M_Waves.M_Waves"));  // adjust to your actual asset path
	if (OceanMatFinder.Succeeded())
	{
		OceanBaseMat = OceanMatFinder.Object;
	}
	
	
	
}

// Called when the game starts or when spawned
void AWaves::BeginPlay()
{
	Super::BeginPlay();
	//Height field texture
	
	HeightField = NewObject<UTextureRenderTarget2D>(this);
	HeightField->RenderTargetFormat = RTF_RGBA16f;
	HeightField->bCanCreateUAV = true;
	HeightField->InitAutoFormat(TEXTURE_RESOLUTION, TEXTURE_RESOLUTION);
	HeightField->UpdateResourceImmediate();
	
	OceanDynMat = UMaterialInstanceDynamic::Create(OceanBaseMat, this);
	OceanDynMat->SetTextureParameterValue(FName("HeightFieldTexture"), HeightField);
	
	for (UProceduralMeshComponent* Tile : Meshes)
	{
		Tile->SetMaterial(0, OceanDynMat);
		UE_LOG(LogTemp, Warning, TEXT("Set material on tile: %s"), *Tile->GetName());
	}
}

void AWaves::CreateGrid(float tileSize, int32 x, int32 y, UProceduralMeshComponent*& mesh)
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
	
	//UV
	float U0 = (float)x / PerRowAmount;
	float U1 = (float)(x + 1) / PerRowAmount;
	float V0 = (float)y / PerRowAmount;
	float V1 = (float)(y + 1) / PerRowAmount;

	TArray<FVector2D> UVs;
	UVs.Add(FVector2D(U0, V0));  // vertex 0: (0,0) corner
	UVs.Add(FVector2D(U0, V1));  // vertex 1: (0,tileSize) corner
	UVs.Add(FVector2D(U1, V0));  // vertex 2: (tileSize,0) corner
	UVs.Add(FVector2D(U1, V1));  // vertex 3: (tileSize,tileSize) corner
	
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
	
	
	SpectrumParameters params(HeightField, Scale, Angle,SpreadBlend, Swell,Alpha, PeakOmega, Gamma, ShortWavesFade);
	
	WavesRender::Dispatch(params);
}

