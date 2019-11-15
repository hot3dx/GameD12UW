//--------------------------------------------------------------------------------------
// pch.h
//
// Header for standard system include files.
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

// Use the C++ standard templated min/max
#define NOMINMAX

#include <wrl.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

#include "Common\d3dx12.h"

#include <algorithm>
#include <exception>
#include <memory>
#include <stdexcept>

#include <stdio.h>

// To use graphics and CPU markup events with the latest version of PIX, change this to include <pix3.h> 
// then add the NuGet package WinPixEventRuntime to the project. 
#include <pix3.h>

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#include "DirectXTK12/Inc/Audio.h"
#include "DirectXTK12/Inc/CommonStates.h"
#include "DirectXTK12/Inc/DirectXHelpers.h"
#include "DirectXTK12/Inc/DDSTextureLoader.h"
#include "DirectXTK12/Inc/DescriptorHeap.h"
#include "DirectXTK12/Inc/Effects.h"
#include "DirectXTK12/Inc/Gamepad.h"
#include "DirectXTK12/Inc/GeometricPrimitive.h"
#include "DirectXTK12/Inc/GraphicsMemory.h"
#include "DirectXTK12/Inc/Keyboard.h"
#include "DirectXTK12/Inc/Model.h"
#include "DirectXTK12/Inc/Mouse.h"
#include "DirectXTK12/Inc/PrimitiveBatch.h"
#include "DirectXTK12/Inc/ResourceUploadBatch.h"
#include "DirectXTK12/Inc/RenderTargetState.h"
#include "DirectXTK12/Inc/SimpleMath.h"
#include "DirectXTK12/Inc/SpriteBatch.h"
#include "DirectXTK12/Inc/SpriteFont.h" 
#include "DirectXTK12/Inc/VertexTypes.h"

namespace DX
{
	// Helper class for COM exceptions
	class com_exception : public std::exception
	{
	public:
		com_exception(HRESULT hr) : result(hr) {}

		virtual const char* what() const override
		{
			static char s_str[64] = {};
			sprintf_s(s_str, "Failure with HRESULT of %08X", result);
			return s_str;
		}

	private:
		HRESULT result;
	};

	// Helper utility converts D3D API failures into exceptions.
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			throw com_exception(hr);
		}
	}
}

