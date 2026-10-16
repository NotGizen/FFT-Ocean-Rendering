// Fill out your copyright notice in the Description page of Project Settings.


#include "Waves.h"
#include "../Public/WavesRender.h"
#include "ProceduralMeshComponent.h"
#include "../Shaders/Private/ShaderDefinitions.ush"
#include "Engine/TextureRenderTarget2D.h"
// Sets default values
AWaves::AWaves()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	//Set default params
	Scale = 1.0;
	Gamma = 3.3f;
	ShortWavesFade = 0.f;
	SpreadBlend = 0.24f;
	Swell = 0.f;

	WindSpeed = 24.f;
	WindDirection = 0.f;
	Fetch = 7600.f;
	
	//Timethy parameters
	/*Scale = 1.0;
	Gamma = 3.3f;
	ShortWavesFade = 0.85f;
	SpreadBlend = 1.0f;
	Swell = 0.f;

	WindSpeed = 50.f;
	WindDirection = 0.f;
	Fetch = 5000.f;*/
	
	//in a grid 10x10 (100) , it will have 10mx10m, make sure to match this with LENGHT_SCALE from shader
	PerRowAmount = 64; // 32x32 will give a total of 102,400 vertices
	//this value is in cm so 100cm is 1m
	TileSize = float(LENGHT_SCALE) * 100.f / PerRowAmount; 
	
	
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
	HeightField->InitAutoFormat(TEX_SIZE, TEX_SIZE);
	HeightField->UpdateResourceImmediate();
	
	Displacement = NewObject<UTextureRenderTarget2D>(this);
	Displacement->RenderTargetFormat = RTF_RGBA16f;
	Displacement->bCanCreateUAV = true;
	Displacement->InitAutoFormat(TEX_SIZE, TEX_SIZE);
	Displacement->UpdateResourceImmediate();
	
	Slope = NewObject<UTextureRenderTarget2D>(this);
	Slope->RenderTargetFormat = RTF_RGBA16f;
	Slope->bCanCreateUAV = true;
	Slope->InitAutoFormat(TEX_SIZE, TEX_SIZE);
	Slope->UpdateResourceImmediate();
	
	PreSlope = NewObject<UTextureRenderTarget2D>(this);
    	PreSlope->RenderTargetFormat = RTF_RGBA16f;
    	PreSlope->bCanCreateUAV = true;
    	PreSlope->InitAutoFormat(TEX_SIZE, TEX_SIZE);
    	PreSlope->UpdateResourceImmediate();
	
	OceanDynMat = UMaterialInstanceDynamic::Create(OceanBaseMat, this);
	OceanDynMat->SetTextureParameterValue(FName("DisplacementTexture"), Displacement);
	OceanDynMat->SetTextureParameterValue(FName("SlopeTexture"), Slope);
	
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

float AWaves::JonswapAlpha(float fetch, float windSpeed)
{
	return 0.076f * pow(GRAVITY * fetch / windSpeed / windSpeed, - 0.22f);
}

float AWaves::JonswapPeakOmega(float fetch, float windSpeed)
{
	return 22.0f * pow(windSpeed * fetch / GRAVITY / GRAVITY, -0.33f);
}

float AWaves::JonswapAngle(float windDirection)
{
	return WindDirection / 180.0f * PI;
}

// Called every frame
void AWaves::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	//modify this for only when values changed
	Alpha =  JonswapAlpha(Fetch, WindSpeed);
	PeakOmega = JonswapPeakOmega(Fetch, WindSpeed);
	Angle = JonswapAngle(WindDirection);
	//
	
	SpectrumParameters SpectrumParams(HeightField, Scale,SpreadBlend, Swell, Gamma, ShortWavesFade, WindSpeed, WindDirection, Fetch, Alpha, PeakOmega, Angle);
	
	float Timer = GetWorld()->GetTimeSeconds();
	TimeEvolutionParameters TimeParameters(HeightField, Displacement, PreSlope, Timer); 
	
	AssembleParameters AssParameters(Displacement, Slope, PreSlope);
	
	//FFTParameters FourierParams();
	WavesRender::Dispatch(SpectrumParams, TimeParameters, AssParameters);
}

