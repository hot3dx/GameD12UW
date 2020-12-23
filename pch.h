﻿//--------------------------------------------------------------------------------------
// pch.h
//
// Header for standard system include files.
//
// ATG Dependence is removed in this
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

// Use the C++ standard templated min/max
#define NOMINMAX
#include <windows.h>
#include <handleapi.h>
#include <combaseapi.h>

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

#include "Audio.h"
#include "CommonStates.h"
#include "DirectXHelpers.h"
#include "DDSTextureLoader.h"
#include "DescriptorHeap.h"
#include "Effects.h"
#include "Gamepad.h"
#include "GeometricPrimitive.h"
#include "GraphicsMemory.h"
#include "Keyboard.h"
#include "Model.h"
#include "Mouse.h"
#include "PrimitiveBatch.h"
#include "ResourceUploadBatch.h"
#include "RenderTargetState.h"
#include "SimpleMath.h"
#include "SpriteBatch.h"
#include "SpriteFont.h" 
#include "VertexTypes.h"

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

