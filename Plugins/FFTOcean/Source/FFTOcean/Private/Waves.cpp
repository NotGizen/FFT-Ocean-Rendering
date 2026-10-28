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
	
	//RepeatTime = 200.f;
	
	//Timethy parameters
	/*Scale = 1.0;
	Gamma = 3.3f;
	ShortWavesFade = 0.85f;
	SpreadBlend = 1.0f;
	Swell = 0.f;

	WindSpeed = 50.f;
	WindDirection = 0.f;
	Fetch = 5000.f;*/
	//-----------------------
	
	//Old grid meshes
	//in a grid 10x10 (100) , it will have 10mx10m, make sure to match this with LENGHT_SCALE from shader
	//PerRowAmount = 64; // 32x32 will give a total of 102,400 vertices
	//this value is in cm so 100cm is 1m
	//TileSize = float(LENGHT_SCALE) * 100.f / PerRowAmount; 
	//---------------
	
	//Setting up Ocean mesh
	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;
	FName CompName = FName(*FString::Printf(TEXT("OceanMesh")));
	OceanMesh = CreateDefaultSubobject<UProceduralMeshComponent>(CompName);
	OceanMesh->SetupAttachment(RootComponent);
	OceanMesh->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	
	//Setting up the ocean grid
	OceanSize = FVector2f(1024.f, 1024.f);
	OceanSubDivision = FUintVector2(512, 512);
	OceanSize *= 100; //Converting UE cm to meters so that it matches the lenght for the ocean in shaders

	
	CreateGrid(OceanSize, OceanSubDivision);

	
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
	HeightField->AddressX = TA_Wrap;
	HeightField->AddressY = TA_Wrap;
	HeightField->InitAutoFormat(TEX_SIZE, TEX_SIZE);
	HeightField->UpdateResourceImmediate();
	
	Displacement = NewObject<UTextureRenderTarget2D>(this);
	Displacement->RenderTargetFormat = RTF_RGBA16f;
	Displacement->bCanCreateUAV = true;
	Displacement->AddressX = TA_Wrap;
	Displacement->AddressY = TA_Wrap;
	Displacement->InitAutoFormat(TEX_SIZE, TEX_SIZE);
	Displacement->UpdateResourceImmediate();
	
	Slope = NewObject<UTextureRenderTarget2D>(this);
	Slope->RenderTargetFormat = RTF_RGBA16f;
	Slope->bCanCreateUAV = true;
	Slope->AddressX = TA_Wrap;
	Slope->AddressY = TA_Wrap;
	Slope->InitAutoFormat(TEX_SIZE, TEX_SIZE);
	Slope->UpdateResourceImmediate();
	
	PreSlope = NewObject<UTextureRenderTarget2D>(this);
    PreSlope->RenderTargetFormat = RTF_RGBA16f;
    PreSlope->bCanCreateUAV = true;
	PreSlope->AddressX = TA_Wrap;
	PreSlope->AddressY = TA_Wrap;
    PreSlope->InitAutoFormat(TEX_SIZE, TEX_SIZE);
    PreSlope->UpdateResourceImmediate();
	
	OceanDynMat = UMaterialInstanceDynamic::Create(OceanBaseMat, this);
	OceanDynMat->SetTextureParameterValue(FName("DisplacementTexture"), Displacement);
	OceanDynMat->SetTextureParameterValue(FName("SlopeTexture"), Slope);
	
	
	OceanMesh->SetMaterial(0, OceanDynMat);
	UE_LOG(LogTemp, Warning, TEXT("Set material on OceanGrid"));

}

	void AWaves::CreateGrid(FVector2f size, FUintVector2 subDivision)
	{
		
		float halfWidth = size.X * 0.5f;
		float halfDepth = size.Y * 0.5f;

		float dx = size.X / subDivision.X;
		float dy = size.Y / subDivision.Y;

		float du = 1.0f / subDivision.X;
		float dv = 1.0f / subDivision.Y;

		//Filling Vertices and TexCoords
		TArray<FVector> Vertices;
		TArray<FVector2D> UVs;
		
		for (uint32_t i = 0; i <= subDivision.Y; ++i)
		{
			float y = -halfDepth + i * dy;
			float v = (float)i * dv;

			for (uint32_t j = 0; j <= subDivision.X; ++j)
			{
				float x = -halfWidth + j * dx;
				float u = (float)j * du;
				
				Vertices.Add(FVector(x, y, 0.f));
				UVs.Add(FVector2D(u, v));
			}
		}
		
		TArray<int32> Indices;
		
		UINT32 stride = subDivision.X + 1;

		for (UINT32 i = 0; i < subDivision.Y; ++i)
		{
			for (UINT32 j = 0; j < subDivision.X; ++j)
			{
				UINT32 bottomLeft = i * stride + j;
				UINT32 bottomRight = bottomLeft + 1;
				UINT32 topLeft = (i + 1) * stride + j;
				UINT32 topRight = topLeft + 1;
				
				//First triangle
				Indices.Add(bottomLeft);
				Indices.Add(topLeft);
				Indices.Add(bottomRight);
				
				//Second triangle
				Indices.Add(bottomRight);
				Indices.Add(topLeft);
				Indices.Add(topRight);
			}
		}
		
		TArray<FVector> Normals;
		for (int32 i = 0; i < Vertices.Num(); ++i)
		{
			Normals.Add(FVector(0, 0, 1)); 
		}
		
		
		TArray<FColor> Colors;
		Colors.Add(FColor(255, 0, 0, 255));
		Colors.Add(FColor(0, 255, 0, 255));	
		Colors.Add(FColor(0, 0, 255, 255));
		Colors.Add(FColor(255, 255, 0, 255));
		
		if (OceanMesh)
		{
			OceanMesh->ClearAllMeshSections();
			OceanMesh->CreateMeshSection(0, Vertices, Indices, Normals, UVs, Colors, TArray<FProcMeshTangent>(), false);
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

