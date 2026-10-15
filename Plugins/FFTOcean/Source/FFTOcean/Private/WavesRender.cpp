#include "WavesRender.h"

#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "RenderTargetPool.h"
#include "RHI.h"

#include "Modules/ModuleManager.h"
#include "../Shaders/Private/ShaderDefinitions.ush"

#define NUM_THREADS_PER_GROUP_DIMENSION 8 //Change this after

//Initial spectrum pass
class FSpectrum : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FSpectrum)
	SHADER_USE_PARAMETER_STRUCT(FSpectrum, FGlobalShader)
	
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, OutputTexture)
		SHADER_PARAMETER(float, scale)
		SHADER_PARAMETER(float, angle)
		SHADER_PARAMETER(float, spreadBlend)
		SHADER_PARAMETER(float, swell)
		SHADER_PARAMETER(float, alpha)
		SHADER_PARAMETER(float, peakOmega)
		SHADER_PARAMETER(float, gamma)
		SHADER_PARAMETER(float, shortWavesFade)

	END_SHADER_PARAMETER_STRUCT()
	

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static inline void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		//We're using it here to add some preprocessor defines. That way we don't have to change both C++ and HLSL code when we change the value for NUM_THREADS_PER_GROUP_DIMENSION
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_X"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Y"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Z"), 1);
	}
};
IMPLEMENT_GLOBAL_SHADER(FSpectrum, "/CustomShaders/Private/FFTWaves.usf", "MainCS", SF_Compute);

//Conjugate pass
class FConjugate : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FConjugate)
	SHADER_USE_PARAMETER_STRUCT(FConjugate, FGlobalShader)
	
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, OutputTexture)
	END_SHADER_PARAMETER_STRUCT()
	

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
	
};
IMPLEMENT_GLOBAL_SHADER(FConjugate, "/CustomShaders/Private/FFTWaves.usf", "ConjugateCS", SF_Compute);

//Time evolution pass
class FTimeEvolution : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FTimeEvolution)
	SHADER_USE_PARAMETER_STRUCT(FTimeEvolution, FGlobalShader)
	
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SpectrumTexture)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, DisplacementTexture)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, PreSlopeTexture)
	SHADER_PARAMETER(float, Time)
	END_SHADER_PARAMETER_STRUCT()
	

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
	
};
IMPLEMENT_GLOBAL_SHADER(FTimeEvolution, "/CustomShaders/Private/FFTWaves.usf", "TimeEvolutionCS", SF_Compute);

//Fourier Horizontal pass
class FFourierH : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FFourierH)
	SHADER_USE_PARAMETER_STRUCT(FFourierH, FGlobalShader)
	
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, FourierTexture)
	
	END_SHADER_PARAMETER_STRUCT()
	

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
	
};
IMPLEMENT_GLOBAL_SHADER(FFourierH, "/CustomShaders/Private/FFTWaves.usf", "FourierHorizontalCS", SF_Compute);

//Fourier Vertical pass
class FFourierV : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FFourierV)
	SHADER_USE_PARAMETER_STRUCT(FFourierV, FGlobalShader)
	
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, FourierTexture)
	
	END_SHADER_PARAMETER_STRUCT()
	

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
	
};
IMPLEMENT_GLOBAL_SHADER(FFourierV, "/CustomShaders/Private/FFTWaves.usf", "FourierVerticalCS", SF_Compute);

//Assemble Maps pass
class FAssembleMaps : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FAssembleMaps)
	SHADER_USE_PARAMETER_STRUCT(FAssembleMaps, FGlobalShader)
	
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters,)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, DisplacementTexture)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, PreSlopeTexture)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float2>, SlopeTexture)
	END_SHADER_PARAMETER_STRUCT()
	

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
	
};
IMPLEMENT_GLOBAL_SHADER(FAssembleMaps, "/CustomShaders/Private/FFTWaves.usf", "AssembleMapsCS", SF_Compute);

WavesRender::WavesRender()
{
}



/*void WavesRender::UpdateParameters(SpectrumParameters& DrawParameters)
{
	cachedParams = DrawParameters;
	bCachedParamsAreValid = true;
}*/

void WavesRender::DispatchRenderThread(FRHICommandListImmediate& RHICmdList, SpectrumParameters SpectrumParams, TimeEvolutionParameters TimeParams, AssembleParameters AssembleParams)
{
	FRDGBuilder GraphBuilder(RHICmdList);
	
	FRDGTextureRef OutTexture = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(SpectrumParams.RenderTarget->GetRenderTargetResource()->TextureRHI, TEXT("HeightField")));
	// This is a pointer to the shader-parameters we declared in the .h
	FSpectrum::FParameters* PassParameters;
	// We ask the RDG to allocate some memory for our shader-parameters
	PassParameters = GraphBuilder.AllocParameters<FSpectrum::FParameters>();

	PassParameters->OutputTexture = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(OutTexture));
	PassParameters->scale = SpectrumParams.Scale;
	PassParameters->alpha = SpectrumParams.Alpha;
	PassParameters->angle = SpectrumParams.Angle;
	PassParameters->gamma = SpectrumParams.Gamma;
	PassParameters->peakOmega = SpectrumParams.PeakOmega;
	PassParameters->shortWavesFade = SpectrumParams.ShortWavesFade;
	PassParameters->spreadBlend = SpectrumParams.SpreadBlend;
	PassParameters->swell = SpectrumParams.Swell;
	
	//Get a reference to our shader type from global shader map
	//Pass 1 (static initial spectrum texture)
	TShaderMapRef<FSpectrum> SpectrumCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("Waves Pass"), SpectrumCS, PassParameters, 
		FIntVector(
	FMath::DivideAndRoundUp(SpectrumParams.GetRenderTargetSize().X, NUM_THREADS_PER_GROUP_DIMENSION),
	FMath::DivideAndRoundUp(SpectrumParams.GetRenderTargetSize().Y, NUM_THREADS_PER_GROUP_DIMENSION),
	1
));
	//Pass 2 (Conjugate)
	FConjugate::FParameters* PassParameters2 = GraphBuilder.AllocParameters<FConjugate::FParameters>();
	
	PassParameters2->OutputTexture = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(OutTexture));
	TShaderMapRef<FConjugate> ConjugateCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("Conjugate Pass"), ConjugateCS, PassParameters2, 
		FIntVector(
	FMath::DivideAndRoundUp(SpectrumParams.GetRenderTargetSize().X, NUM_THREADS_PER_GROUP_DIMENSION),
	FMath::DivideAndRoundUp(SpectrumParams.GetRenderTargetSize().Y, NUM_THREADS_PER_GROUP_DIMENSION),
	1));
	
	//Pass 3 (Time Evolution)
	
	FRDGTextureRef DisplacementTexture = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(TimeParams.DisplacementRT->GetRenderTargetResource()->TextureRHI, TEXT("Displacement")));
	FRDGTextureRef PreSlopeTexture = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(TimeParams.PreSlopeRT->GetRenderTargetResource()->TextureRHI, TEXT("PreSlope")));
	
	FTimeEvolution::FParameters* TimeParameters = GraphBuilder.AllocParameters<FTimeEvolution::FParameters>();
	
	TimeParameters->SpectrumTexture = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(OutTexture));
	TimeParameters->DisplacementTexture = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(DisplacementTexture));
	TimeParameters->PreSlopeTexture = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(PreSlopeTexture));
	TimeParameters->Time = TimeParams.Time;
	
	TShaderMapRef<FTimeEvolution> TimeEvolutionCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("TimeEvolution Pass"), TimeEvolutionCS, TimeParameters, 
		FIntVector(
	FMath::DivideAndRoundUp(SpectrumParams.GetRenderTargetSize().X, NUM_THREADS_PER_GROUP_DIMENSION),
	FMath::DivideAndRoundUp(SpectrumParams.GetRenderTargetSize().Y, NUM_THREADS_PER_GROUP_DIMENSION),
	1));
	
	//Pass 4 (Fourier Horizontal Pass)
	
	
	FFourierH::FParameters* FourierHParameters = GraphBuilder.AllocParameters<FFourierH::FParameters>();
	

	FourierHParameters->FourierTexture = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(DisplacementTexture));
	
	TShaderMapRef<FFourierH> FourierHorizontalCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("FFT Horizontal Pass"), FourierHorizontalCS, FourierHParameters, 
		FIntVector(
	1,
	TEX_SIZE,
	1));
	
	//Pass 5 (Fourier Vertical Pass)
	
	FFourierV::FParameters* FourierVParameters = GraphBuilder.AllocParameters<FFourierV::FParameters>();
	

	FourierVParameters->FourierTexture = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(DisplacementTexture));
	
	TShaderMapRef<FFourierV> FourierVerticalCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	
	FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("FFT Vertical Pass"), FourierVerticalCS, FourierVParameters, 
		FIntVector(
	1,
	TEX_SIZE,
	1));
	
	//Redo FFT Pass For slope maps
	
	//Pass 4.2 (Slope Fourier Horizontal Pass)
	
	
	FFourierH::FParameters* FourierHParameters2 = GraphBuilder.AllocParameters<FFourierH::FParameters>();
	
	FourierHParameters2->FourierTexture = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(PreSlopeTexture));
	FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("FFT Horizontal Slope Pass"), FourierHorizontalCS, FourierHParameters2, 
		FIntVector(
	1,
	TEX_SIZE,
	1));
	
	//Pass 5.2 (Slope Fourier Vertical Pass)
	
	FFourierV::FParameters* FourierVParameters2 = GraphBuilder.AllocParameters<FFourierV::FParameters>();
	

	FourierVParameters2->FourierTexture = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(PreSlopeTexture));
	
	FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("FFT Vertical Slope Pass"), FourierVerticalCS, FourierVParameters2, 
		FIntVector(
	1,
	TEX_SIZE,
	1));
	
	//Pass 6 (Assemble Maps)
	
	FAssembleMaps::FParameters* AssembleParameters = GraphBuilder.AllocParameters<FAssembleMaps::FParameters>();
	FRDGTextureRef SlopeTexture = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(AssembleParams.SlopeRT->GetRenderTargetResource()->TextureRHI, TEXT("Slope")));
	AssembleParameters->DisplacementTexture = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(DisplacementTexture));
	AssembleParameters->PreSlopeTexture = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(PreSlopeTexture));
	AssembleParameters->SlopeTexture = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(SlopeTexture));
	
	TShaderMapRef<FAssembleMaps> AssembleMapsCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	//change spectrumParams.Get renderSize for a dfixed value to avoid confusion
	FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("Assemble Maps Pass"), AssembleMapsCS, AssembleParameters, 
		FIntVector(
	FMath::DivideAndRoundUp(SpectrumParams.GetRenderTargetSize().X, NUM_THREADS_PER_GROUP_DIMENSION),
	FMath::DivideAndRoundUp(SpectrumParams.GetRenderTargetSize().Y, NUM_THREADS_PER_GROUP_DIMENSION),
	1));
	GraphBuilder.Execute();
}

