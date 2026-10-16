#pragma once


#include "CoreMinimal.h"

#include "RenderGraphUtils.h"
#include "Engine/TextureRenderTargetVolume.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"

struct SpectrumParameters
{
	UTextureRenderTarget2D* RenderTarget;
	float Scale;
	float SpreadBlend;
	float Swell;
	float Gamma;
	float ShortWavesFade;
	float WindSpeed;
	float WindDirection;
	float Fetch;

	float Angle;
	float Alpha;
	float PeakOmega;
	
	FIntVector2 GetRenderTargetSize() const
	{
		return CachedRenderTargetSize;
	}

	SpectrumParameters()
	{
		
	}
	SpectrumParameters(UTextureRenderTarget2D* IORenderTarget, float InScale, float InSpreadBlend,
	float InSwell, float InGamma, float InShortWavesFade, float InWindSpeed, float InWindDirection, float InFetch, float InAlpha, float InPeakOmega, float InAngle)
		: RenderTarget(IORenderTarget)
	, Scale(InScale), Angle(InAngle), SpreadBlend(InSpreadBlend), Swell(InSwell)
	, Alpha(InAlpha), PeakOmega(InPeakOmega), Gamma(InGamma), ShortWavesFade(InShortWavesFade),
	WindSpeed(InWindSpeed), WindDirection(InWindDirection), Fetch(InFetch)
	{
		CachedRenderTargetSize = RenderTarget ? FIntVector2(RenderTarget->SizeX, RenderTarget->SizeY) : FIntVector2::ZeroValue;
	}


private:
	FIntVector2 CachedRenderTargetSize;

public:
};

struct TimeEvolutionParameters
{
	public:
	UTextureRenderTarget2D* SpectrumRT;
	UTextureRenderTarget2D* DisplacementRT;
	UTextureRenderTarget2D* PreSlopeRT;
	float Time;
	
	TimeEvolutionParameters()
	{
		
	}
	
	TimeEvolutionParameters(UTextureRenderTarget2D* SpectrumTex, UTextureRenderTarget2D* DisplacementTex, UTextureRenderTarget2D* PreSlopeTex, float ATime)
		: SpectrumRT(SpectrumTex), DisplacementRT(DisplacementTex), PreSlopeRT(PreSlopeTex), Time(ATime) //not 0 due to division in shaders?
	{
		CachedRenderTargetSize = SpectrumRT ? FIntVector2(SpectrumRT->SizeX, SpectrumRT->SizeY) : FIntVector2::ZeroValue;
	}
	
private:
	
FIntVector2 CachedRenderTargetSize;
	
};

struct FFTParameters
{
public:
	UTextureRenderTarget2D* FFTRT;
	
	FFTParameters()
	{
		
	}
	
	FFTParameters(UTextureRenderTarget2D* FourierRT)
		: FFTRT(FourierRT)
	{
		CachedRenderTargetSize = FourierRT ? FIntVector2(FourierRT->SizeX, FourierRT->SizeY) : FIntVector2::ZeroValue;
	}
	
private:
	
	FIntVector2 CachedRenderTargetSize;
	
};

struct AssembleParameters
{
public:
	UTextureRenderTarget2D* DisplacementRT;
	UTextureRenderTarget2D* PreSlopeRT;
	UTextureRenderTarget2D* SlopeRT;
	
	AssembleParameters()
	{
		
	}
	
	AssembleParameters(UTextureRenderTarget2D* DisplacementTarget, UTextureRenderTarget2D* SlopeTarget, UTextureRenderTarget2D* PreSlopeTarget)
		: DisplacementRT(DisplacementTarget), SlopeRT(SlopeTarget), PreSlopeRT(PreSlopeTarget)
	{
		//Both displacement and slope targets are same size so it safe to use the same CachedRenderTargetSize for both
		CachedRenderTargetSize = DisplacementRT ? FIntVector2(DisplacementRT->SizeX, DisplacementRT->SizeY) : FIntVector2::ZeroValue;
	}
	
private:
	
	FIntVector2 CachedRenderTargetSize;
	
};


class FFTOCEAN_API WavesRender
{
public:
	WavesRender();

	// Executes this shader on the render thread


	//void UpdateParameters(SpectrumParameters& DrawParameters);


	static void Dispatch(SpectrumParameters SpectrumParams, TimeEvolutionParameters TimeParams, AssembleParameters AssembleParams)
	{
		ENQUEUE_RENDER_COMMAND(SceneDrawCompletion)(
			[SpectrumParams, TimeParams, AssembleParams](FRHICommandListImmediate& RHICmdList)
			{
				DispatchRenderThread(RHICmdList, SpectrumParams, TimeParams, AssembleParams);
			});
	}

	static void DispatchRenderThread(
		FRHICommandListImmediate& RHICmdList,
		SpectrumParameters SpectrumParams, TimeEvolutionParameters TimeParams, AssembleParameters AssembleParams

	);
	
private:

	//Cached Shader Manager Parameters
	SpectrumParameters cachedParams;
	//Whether we have cached parameters to pass to the shader or not
	volatile bool bCachedParamsAreValid;

	//Reference to a pooled render target where the shader will write its output
	TRefCountPtr<IPooledRenderTarget> ComputeShaderOutput;
};





