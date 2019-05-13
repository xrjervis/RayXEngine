#include "Engine/Renderer/d3d11/D3DCommon.hpp"

D3D11_USAGE ToDXMemoryUsage(eMemoryUsage usage) {
	switch (usage) {
	case MEMORY_USAGE_DEFAULT:	return D3D11_USAGE_DEFAULT;
	case MEMORY_USAGE_STATIC:	return D3D11_USAGE_IMMUTABLE;
	case MEMORY_USAGE_DYNAMIC:	return D3D11_USAGE_DYNAMIC;
	case MEMORY_USAGE_STAGING:	return D3D11_USAGE_STAGING;
	default:					return D3D11_USAGE_DEFAULT;
	}
}

u32 ToDXBindFlags(u32 flags) {
	u32 ret = 0U;

	if (flags & BIND_VERTEX_BUFFER) {
		ret |= D3D11_BIND_VERTEX_BUFFER;
	}

	if (flags & BIND_INDEX_BUFFER) {
		ret |= D3D11_BIND_INDEX_BUFFER;
	}

	if (flags & BIND_CONSTANT_BUFFER) {
		ret |= D3D11_BIND_CONSTANT_BUFFER;
	}

	if (flags & BIND_UNORDERED_ACCESS) {
		ret |= D3D11_BIND_UNORDERED_ACCESS;
	}

	if (flags & BIND_SHADER_RESOURCE) {
		ret |= D3D11_BIND_SHADER_RESOURCE;
	}

	if (flags & BIND_RENDER_TARGET) {
		ret |= D3D11_BIND_RENDER_TARGET;
	}

	if (flags & BIND_DEPTH_STENCIL) {
		ret |= D3D11_BIND_DEPTH_STENCIL;
	}

	return ret;
}

DXGI_FORMAT ToDXGIFormat(eTextureFormat textureFormat) {
	switch (textureFormat) {
	case TEXTURE_FORMAT_R8G8B8A8_UNORM:	return DXGI_FORMAT_R8G8B8A8_UNORM;
	case TEXTURE_FORMAT_R32G32B32A32_FLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case TEXTURE_FORMAT_D24_UNORM_S8_UINT: return DXGI_FORMAT_D24_UNORM_S8_UINT;
	default:
		DebuggerPrintf("Not supported texture format!");
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	}
}

D3D11_PRIMITIVE_TOPOLOGY ToDXPrimitiveType(ePrimitiveType primitiveType) {
	switch (primitiveType){
	case PRIMITIVE_TYPE_POINTLIST:		return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
	case PRIMITIVE_TYPE_LINELIST:		return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	case PRIMITIVE_TYPE_LINESTRIP:		return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
	case PRIMITIVE_TYPE_TRIANGLELIST:	return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	case PRIMITIVE_TYPE_TRIANGLESTRIP:	return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	default:							return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	}
}

D3D11_FILL_MODE ToDXFillMode(eFillMode fillMode) {
	switch (fillMode) {
	case FILL_MODE_SOLID:		return D3D11_FILL_SOLID;
	case FILL_MODE_WIREFRAME:	return D3D11_FILL_WIREFRAME;
	default:					return D3D11_FILL_SOLID;
	}
}

D3D11_CULL_MODE ToDXCullMode(eCullMode cullMode) {
	switch (cullMode) {
	case CULL_MODE_DISABLE:		return D3D11_CULL_NONE;
	case CULL_MODE_BACK:		return D3D11_CULL_BACK;
	case CULL_MODE_FRONT:		return D3D11_CULL_FRONT;
	default:					return D3D11_CULL_BACK;
	}
}

D3D11_COMPARISON_FUNC ToDXComparisonFunc(eComparisonFunc compare) {
	switch (compare) {
	case COMPARISON_NEVER:			return D3D11_COMPARISON_NEVER;
	case COMPARISON_LESS:			return D3D11_COMPARISON_LESS;
	case COMPARISON_EQUAL:			return D3D11_COMPARISON_EQUAL;
	case COMPARISON_LESS_EQUAL:		return D3D11_COMPARISON_LESS_EQUAL;
	case COMPARISON_GREATER:		return D3D11_COMPARISON_GREATER;
	case COMPARISON_NOT_EQUAL:		return D3D11_COMPARISON_NOT_EQUAL;
	case COMPARISON_GREATER_EQUAL:	return D3D11_COMPARISON_GREATER_EQUAL;
	case COMPARISON_ALWAYS:			return D3D11_COMPARISON_ALWAYS;
	default:						return D3D11_COMPARISON_ALWAYS;
	}
}

D3D11_BLEND ToDXBlend(eBlendFactor blend) {
	switch (blend) {
	case BLEND_ZERO:			return D3D11_BLEND_ZERO;
	case BLEND_ONE:				return D3D11_BLEND_ONE;
	case BLEND_SRC_COLOR:		return D3D11_BLEND_SRC_COLOR;
	case BLEND_INV_SRC_COLOR:	return D3D11_BLEND_INV_SRC_COLOR;
	case BLEND_SRC_ALPHA:		return D3D11_BLEND_SRC_ALPHA;
	case BLEND_INV_SRC_ALPHA:	return D3D11_BLEND_INV_SRC_ALPHA;
	case BLEND_DST_COLOR:		return D3D11_BLEND_DEST_COLOR;
	case BLEND_INV_DST_COLOR:	return D3D11_BLEND_INV_DEST_COLOR;
	case BLEND_DST_ALPHA:		return D3D11_BLEND_DEST_ALPHA;
	case BLEND_INV_DST_ALPHA:	return D3D11_BLEND_INV_DEST_ALPHA;
	case BLEND_CONSTANT:		return D3D11_BLEND_BLEND_FACTOR;
	case BLEND_INV_CONSTANT:	return D3D11_BLEND_INV_BLEND_FACTOR;
	default:					return D3D11_BLEND_ONE;
	}
}

D3D11_BLEND_OP ToDXBlendOp(eBlendOp blendOp) {
	switch (blendOp) {
	case BLEND_OP_ADD:			return D3D11_BLEND_OP_ADD;
	case BLEND_OP_SUBTRACT:		return D3D11_BLEND_OP_SUBTRACT;
	case BLEND_OP_REV_SUBTRACT: return D3D11_BLEND_OP_REV_SUBTRACT;
	case BLEND_OP_MIN:			return D3D11_BLEND_OP_MIN;
	case BLEND_OP_MAX:			return D3D11_BLEND_OP_MAX;
	default:					return D3D11_BLEND_OP_ADD;
	}
}

D3D11_STENCIL_OP ToDXStencilOp(eStencilOp stencilOp) {
	switch (stencilOp) {
	case STENCIL_OP_KEEP:            return D3D11_STENCIL_OP_KEEP;
	case STENCIL_OP_ZERO:            return D3D11_STENCIL_OP_ZERO;
	case STENCIL_OP_REPLACE:         return D3D11_STENCIL_OP_REPLACE;
	case STENCIL_OP_INCREMENT:       return D3D11_STENCIL_OP_INCR;
	case STENCIL_OP_INCREMENT_SAT:   return D3D11_STENCIL_OP_INCR_SAT;
	case STENCIL_OP_DECREMENT:       return D3D11_STENCIL_OP_DECR;
	case STENCIL_OP_DECREMENT_SAT:   return D3D11_STENCIL_OP_DECR_SAT;
	case STENCIL_OP_INVERT:          return D3D11_STENCIL_OP_INVERT;
	default:						 return D3D11_STENCIL_OP_KEEP;
	}
}

std::string ToDXShaderEntryPoint(eShaderType type) {
	switch(type) {
	case SHADER_TYPE_VERTEX_SHADER:		return "VSMain";
	case SHADER_TYPE_PIXEL_SHADER:		return "PSMain";
	case SHADER_TYPE_HULL_SHADER:		return "HSMain";
	case SHADER_TYPE_DOMAIN_SHADER:		return "DSMain";
	case SHADER_TYPE_GEOMETRY_SHADER:	return "GSMain";
	case SHADER_TYPE_COMPUTE_SHADER:	return "CSMain";
	default:							return "";
	}
}

std::string ToDXShaderCompileModel(eShaderType type) {
	switch (type) {
	case SHADER_TYPE_VERTEX_SHADER:		return "vs_5_0";
	case SHADER_TYPE_PIXEL_SHADER:		return "ps_5_0";
	case SHADER_TYPE_HULL_SHADER:		return "hs_5_0";
	case SHADER_TYPE_DOMAIN_SHADER:		return "ds_5_0";
	case SHADER_TYPE_GEOMETRY_SHADER:	return "gs_5_0";
	case SHADER_TYPE_COMPUTE_SHADER:	return "cs_5_0";
	default:							return "";
	}
}

D3D11_TEXTURE_ADDRESS_MODE ToDXWrapMode(eWrapMode wrap) {
	switch(wrap){
	case WRAP_MODE_BORDER:		return D3D11_TEXTURE_ADDRESS_BORDER;
	case WRAP_MODE_CLAMP:		return D3D11_TEXTURE_ADDRESS_CLAMP;
	case WRAP_MODE_MIRROR:		return D3D11_TEXTURE_ADDRESS_MIRROR;
	case WRAP_MODE_LOOP:		return D3D11_TEXTURE_ADDRESS_WRAP;
	default:					return D3D11_TEXTURE_ADDRESS_WRAP;
	}
}

D3D11_FILTER ToDXFilter(eFilterType filter) {
	switch(filter) {
	case FILTER_TYPE_ANISOTROPIC:			return D3D11_FILTER_ANISOTROPIC;
	case FILTER_TYPE_MIN_MAG_MIP_LINEAR:	return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	case FILTER_TYPE_MIN_MAG_MIP_POINT:		return D3D11_FILTER_MIN_MAG_MIP_POINT;
	default:								return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	}
}

eWrapMode ToWrapMode(const std::string& str) {
	if(str == "loop"){
		return WRAP_MODE_LOOP;
	}
	if (str == "border") {
		return WRAP_MODE_BORDER;
	}
	if (str == "clamp"){
		return WRAP_MODE_CLAMP;
	}
	if (str == "mirror"){
		return WRAP_MODE_MIRROR;
	}
	return WRAP_MODE_LOOP;
}

eFillMode ToFillMode(const std::string& str) {
	if(str == "solid"){
		return FILL_MODE_SOLID;
	}
	if (str == "wireframe"){
		return FILL_MODE_WIREFRAME;
	}
	return FILL_MODE_SOLID;
}

eCullMode ToCullMode(const std::string& str) {
	if(str == "none"){
		return CULL_MODE_DISABLE;
	}
	if (str == "back"){
		return CULL_MODE_BACK;
	}
	if (str == "front"){
		return CULL_MODE_FRONT;
	}
	return CULL_MODE_BACK;
}

eBlendFactor ToBlendFactor(const std::string& str) {
	if(str == "src_alpha"){
		return BLEND_SRC_ALPHA;
	}
	if(str == "inv_src_alpha"){
		return BLEND_INV_SRC_ALPHA;
	}
	if(str == "one"){
		return BLEND_ONE;
	}
	if(str == "zero"){
		return BLEND_ZERO;
	}

	return BLEND_ONE;
}

eBlendOp ToBlendOp(const std::string& str) {
	if(str == "add"){
		return BLEND_OP_ADD;
	}
	if(str == "max"){
		return BLEND_OP_MAX;
	}
	if(str == "min"){
		return BLEND_OP_MIN;
	}
	if(str == "subtract"){
		return BLEND_OP_SUBTRACT;
	}
	return BLEND_OP_ADD;
}

eComparisonFunc ToCompareFunc(const std::string& str) {
	if(str == "less"){
		return COMPARISON_LESS;
	}
	if(str == "less_equal"){
		return COMPARISON_LESS_EQUAL;
	}
	if(str == "never"){
		return COMPARISON_NEVER;
	}
	if(str == "always"){
		return COMPARISON_ALWAYS;
	}
	if(str == "equal"){
		return COMPARISON_EQUAL;
	}
	if(str == "greater"){
		return COMPARISON_GREATER;
	}
	if(str == "greater_equal"){
		return COMPARISON_GREATER_EQUAL;
	}
	if(str == "not_equal") {
		return COMPARISON_NOT_EQUAL;
	}
	ERROR_RECOVERABLE("Unsupported comare function string!");
	return COMPARISON_ALWAYS;
}
