// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Waves.generated.h"

#define TEXTURE_RESOLUTION 128

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
	
	UPROPERTY()
	UTextureRenderTarget2D* HeightField;
	
	UPROPERTY(EditAnywhere)
	UMaterialInstanceDynamic* OceanDynMat;
	
	UPROPERTY(EditAnywhere)
	UMaterialInterface* OceanBaseMat;
	
	//Ocean Params
	UPROPERTY(EditAnywhere)
	float Scale;
	
	UPROPERTY(EditAnywhere)
	float Angle;
	
	UPROPERTY(EditAnywhere)
	float SpreadBlend;
	
	UPROPERTY(EditAnywhere)
	float Swell;
	
	UPROPERTY(EditAnywhere)
	float Alpha;
	
	UPROPERTY(EditAnywhere)
	float PeakOmega;
	
	UPROPERTY(EditAnywhere)
	float Gamma;
	
	UPROPERTY(EditAnywhere)
	float ShortWavesFade;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
};
