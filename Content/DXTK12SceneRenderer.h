//--------------------------------------------------------------------------------------
// File: DXTK12SceneRenderer.h
//
// This is a simple Windows Store app for Windows 8.1 showing use of DirectXTK
//
// http://go.microsoft.com/fwlink/?LinkId=248929
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#include "..\Common\DeviceResources12.h"
#include "..\Common\StepTimer.h"

#include "Audio.h"
#include "CommonStates.h"
#include "Effects.h"
#include "GeometricPrimitive.h"
#include "Model.h"
#include "PrimitiveBatch.h"
#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "VertexTypes.h"

namespace GameD12UW
{
    // This class renders a scene using DirectXTK
	class DXTK12SceneRenderer
	{
	public:
		DXTK12SceneRenderer(const std::shared_ptr<DX::DeviceResources12>& deviceResources);
		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
        void CreateAudioResources();
		void ReleaseDeviceDependentResources();
		void Update(DX::StepTimer const& timer);
		void Render();

        // Signals a new audio device is available
        void NewAudioDevice();

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources12> m_deviceResources;

		void XM_CALLCONV DrawGrid(DirectX::FXMVECTOR xAxis, DirectX::FXMVECTOR yAxis, DirectX::FXMVECTOR origin, size_t xdivs, size_t ydivs, DirectX::GXMVECTOR color);

        std::unique_ptr<DirectX::CommonStates>                                  m_states;
        std::unique_ptr<DirectX::BasicEffect>                                   m_batchEffect;
        std::unique_ptr<DirectX::IEffectFactory>                                m_fxFactory;
        std::unique_ptr<DirectX::GeometricPrimitive>                            m_shape;
        std::unique_ptr<DirectX::Model>                                         m_model;
        std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>  m_batch;
        std::unique_ptr<DirectX::SpriteBatch>                                   m_sprites;
        std::unique_ptr<DirectX::SpriteFont>                                    m_font;

        std::unique_ptr<DirectX::AudioEngine>                                   m_audEngine;
        std::unique_ptr<DirectX::WaveBank>                                      m_waveBank;
        std::unique_ptr<DirectX::SoundEffect>                                   m_soundEffect;
        std::unique_ptr<DirectX::SoundEffectInstance>                           m_effect1;
        std::unique_ptr<DirectX::SoundEffectInstance>                           m_effect2;

		Microsoft::WRL::ComPtr<ID3D12Resource>                                  m_texture1;
		Microsoft::WRL::ComPtr<ID3D12Resource>                                  m_texture2;

       // Microsoft::WRL::ComPtr<ID3D11InputLayout>                               m_batchInputLayout;

		// Variables used with the rendering loop.
        uint32_t                                                                m_audioEvent;
        float                                                                   m_audioTimerAcc;

        bool                                                                    m_retryDefault;

        DirectX::XMFLOAT4X4 m_world;
        DirectX::XMFLOAT4X4 m_view;
        DirectX::XMFLOAT4X4 m_projection;


		
		// Descriptors
		enum Descriptors
		{
			WindowsLogo,
			SeaFloor,
			SegoeFont,
			Count = 256
		};
	};
}

