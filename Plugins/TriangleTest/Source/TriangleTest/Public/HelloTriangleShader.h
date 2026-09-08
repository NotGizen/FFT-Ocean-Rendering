#pragma once
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "Runtime/Renderer/Public/ScreenPass.h"

//Vertex shader setup
BEGIN_SHADER_PARAMETER_STRUCT(FTriangleVSParams, )
END_SHADER_PARAMETER_STRUCT()
class FTriangleVS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FTriangleVS);
	using FParameters = FTriangleVSParams;
	SHADER_USE_PARAMETER_STRUCT(FTriangleVS, FGlobalShader);

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters) {
		return true;
	}
};

//Fragment Shader Setup
BEGIN_SHADER_PARAMETER_STRUCT(FTrianglePSParams, )
	RENDER_TARGET_BINDING_SLOTS()
END_SHADER_PARAMETER_STRUCT()
class FTrianglePS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FTrianglePS);
	using FParameters = FTrianglePSParams;
	SHADER_USE_PARAMETER_STRUCT(FTrianglePS, FGlobalShader)
};



//-----------------

//Shader Data
struct FHelloVertex
{
	FVector2f Position;
	FVector4f Color;
};

//Rendering resources to hold and lay out the data on the GPU

class FTriangleVertexBuffer : public FVertexBuffer
{
public:
	void InitRHI(FRHICommandListBase& RHICmdList);
};

class FTriangleIndexBuffer : public FIndexBuffer
{
public:
	void InitRHI(FRHICommandListBase& RHICmdList);
};

class FTrianlgeVertexBufferElementDesc : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;
	virtual ~FTrianlgeVertexBufferElementDesc() {}

	virtual void InitRHI(FRHICommandListBase& RHICmdList);
	virtual void ReleaseRHI();
};

extern TRIANGLETEST_API TGlobalResource<FTriangleVertexBuffer> GTriangleVertexBuf;
extern TRIANGLETEST_API TGlobalResource<FTrianlgeVertexBufferElementDesc> GTriangleVertexBufElementDesc;
extern TRIANGLETEST_API TGlobalResource<FTriangleIndexBuffer> GTriangleIndexBuf;