#pragma once


#include "CoreMinimal.h"

#include "RenderGraphUtils.h"
#include "Engine/TextureRenderTargetVolume.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"

struct SpectrumParameters
{
	UTextureRenderTarget2D* RenderTarget;
	float Scale;
	float Angle;
	float SpreadBlend;
	float Swell;
	float Alpha;
	float PeakOmega;
	float Gamma;
	float ShortWavesFade;

	FIntVector2 GetRenderTargetSize() const
	{
		return CachedRenderTargetSize;
	}

	SpectrumParameters()
	{
		
	}
	SpectrumParameters(UTextureRenderTarget2D* IORenderTarget, float InScale, float InAngle, float InSpreadBlend,
	float InSwell, float InAlpha, float InPeakOmega, float InGamma, float InShortWavesFade)
		: RenderTarget(IORenderTarget)
	, Scale(InScale), Angle(InAngle), SpreadBlend(InSpreadBlend), Swell(InSwell)
	, Alpha(InAlpha), PeakOmega(InPeakOmega), Gamma(InGamma), ShortWavesFade(InShortWavesFade)
	{
		CachedRenderTargetSize = RenderTarget ? FIntVector2(RenderTarget->SizeX, RenderTarget->SizeY) : FIntVector2::ZeroValue;
	}


private:
	FIntVector2 CachedRenderTargetSize;

public:
};


/**
 * 
 */
class FFTOCEAN_API WavesRender
{
public:
	WavesRender();

	// Executes this shader on the render thread


	void UpdateParameters(SpectrumParameters& DrawParameters);


	static void Dispatch(SpectrumParameters Params)
	{
		ENQUEUE_RENDER_COMMAND(SceneDrawCompletion)(
			[Params](FRHICommandListImmediate& RHICmdList)
			{
				DispatchRenderThread(RHICmdList, Params);
			});
	}

	static void DispatchRenderThread(
		FRHICommandListImmediate& RHICmdList,
		SpectrumParameters Params

	);
	
private:

	//Cached Shader Manager Parameters
	SpectrumParameters cachedParams;
	//Whether we have cached parameters to pass to the shader or not
	volatile bool bCachedParamsAreValid;

	//Reference to a pooled render target where the shader will write its output
	TRefCountPtr<IPooledRenderTarget> ComputeShaderOutput;
};





