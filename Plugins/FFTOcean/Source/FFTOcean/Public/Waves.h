// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Waves.generated.h"



class UProceduralMeshComponent;
class UTextureRenderTarget2D;

UCLASS()
class AWaves : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWaves();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	//UStaticMeshComponent* VisualMesh;
	UPROPERTY(EditAnywhere)
	TArray<UProceduralMeshComponent*> Meshes;
	
	UPROPERTY(EditAnywhere)
	float TileSize;
	
	UPROPERTY(EditAnywhere)
	int32 PerRowAmount;
	
	
	void CreateGrid(float tileSize, int32 x, int32 y, UProceduralMeshComponent*& mesh);
	
	//Spectrum parameters fucntions
	float JonswapAlpha(float fetch, float windspeed);
	float JonswapPeakOmega(float fetch, float windspeed);
	float JonswapAngle(float windDirection);
	
	UPROPERTY()
	UTextureRenderTarget2D* HeightField;
	
	UPROPERTY()
	UTextureRenderTarget2D* Displacement;
	
	UPROPERTY()
	UTextureRenderTarget2D* Slope;
	
	UPROPERTY()
	UTextureRenderTarget2D* PreSlope;
	
	UPROPERTY(EditAnywhere)
	UMaterialInstanceDynamic* OceanDynMat;
	
	UPROPERTY(EditAnywhere)
	UMaterialInterface* OceanBaseMat;
	
	//Ocean Params
	UPROPERTY(EditAnywhere, Category = "Ocean Params", Meta = (ClampMin = 1.f, ClampMax =  5.f))
	float Scale;
	
	UPROPERTY(EditAnywhere, Category = "Ocean Params", Meta = (ClampMin = 0.f, ClampMax =  1.f))
	float SpreadBlend;
	
	UPROPERTY(EditAnywhere, Category = "Ocean Params", Meta = (ClampMin = 0.f, ClampMax =  1.f))
	float Swell;
	
	UPROPERTY(EditAnywhere, Category = "Ocean Params", Meta = (ClampMin = 0.f, ClampMax =  7.f))
	float Gamma;
	
	UPROPERTY(EditAnywhere, Category = "Ocean Params", Meta = (ClampMin = 0.f, ClampMax =  1.f))
	float ShortWavesFade;
	
	UPROPERTY(EditAnywhere, Category = "Ocean Params", Meta = (ClampMin = 0.f, ClampMax =  100.f))
	float WindSpeed; //[0.f, 100.f]
	
	UPROPERTY(EditAnywhere, Category = "Ocean Params", Meta = (ClampMin = 0.f, ClampMax =  360.f))
	float WindDirection; // [0.0f, 360.0f]
	
	UPROPERTY(EditAnywhere, Category = "Ocean Params", Meta = (ClampMin = 0.f, ClampMax =  10000.f))
	float Fetch; //[0.0f, 10000.0f]
	
	//Non-modifiable values
	UPROPERTY(VisibleAnywhere, Category = "Ocean Params")
	float Angle;
	
	UPROPERTY(VisibleAnywhere, Category = "Ocean Params")
	float Alpha;
	
	UPROPERTY(VisibleAnywhere, Category = "Ocean Params")
	float PeakOmega;
	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
};
