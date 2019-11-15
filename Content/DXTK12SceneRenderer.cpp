﻿//--------------------------------------------------------------------------------------
// File: DXTK12SceneRenderer.cpp
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

#include "pch.h"
#include "DXTK12SceneRenderer.h"

#include "DDSTextureLoader.h"

#include "..\Common\DirectXHelper.h"	// For ThrowIfFailed and ReadDataAsync

using namespace GameD12UW;

using namespace DirectX;
using namespace Windows::Foundation;

DXTK12SceneRenderer::DXTK12SceneRenderer(const std::shared_ptr<DX::DeviceResources12>& deviceResources) :
    m_deviceResources(deviceResources)
{
	CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
    CreateAudioResources();
}

// Initializes view parameters when the window size changes.
void DXTK12SceneRenderer::CreateWindowSizeDependentResources()
{
	RECT r = m_deviceResources->GetOutputSize();
	Size outputSize;
	outputSize.Width = r.right - r.left;
	outputSize.Height = r.bottom - r.top;
    float aspectRatio = outputSize.Width / outputSize.Height;
    float fovAngleY = 70.0f * XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
		fovAngleY,
		aspectRatio,
		0.01f,
		100.0f
		);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

    XMMATRIX projection = XMMatrixMultiply(perspectiveMatrix, orientationMatrix);

    m_batchEffect->SetProjection(projection);

    m_sprites->SetRotation( m_deviceResources->ComputeDisplayRotation() );

	XMStoreFloat4x4(
        &m_projection,
		projection
		);
}

void DXTK12SceneRenderer::CreateAudioResources()
{
    // Create DirectXTK for Audio objects
    AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
    eflags = eflags | AudioEngine_Debug;
#endif

    m_audEngine.reset(new AudioEngine(eflags));

    m_audioEvent = 0;
    m_audioTimerAcc = 10.f;
    m_retryDefault = false;

    m_waveBank.reset(new WaveBank(m_audEngine.get(), L"assets\\adpcmdroid.xwb"));
    m_soundEffect.reset(new SoundEffect(m_audEngine.get(), L"assets\\MusicMono_adpcm.wav"));
    m_effect1 = m_soundEffect->CreateInstance();
    m_effect2 = m_waveBank->CreateInstance(10);

    m_effect1->Play(true);
    m_effect2->Play();
}

void DXTK12SceneRenderer::Update(DX::StepTimer const& timer)
{
    XMVECTOR eye = XMVectorSet(0.0f, 0.7f, 1.5f, 0.0f);
    XMVECTOR at = XMVectorSet(0.0f, -0.1f, 0.0f, 0.0f);
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMMATRIX view = XMMatrixLookAtRH(eye, at, up);
    XMMATRIX world = XMMatrixRotationY( float(timer.GetTotalSeconds() * XM_PIDIV4) );

    m_batchEffect->SetView(view);
    m_batchEffect->SetWorld(XMMatrixIdentity());

    XMStoreFloat4x4(&m_view, view);
    XMStoreFloat4x4(&m_world, world);

    m_audioTimerAcc -= (float)timer.GetElapsedSeconds();
    if (m_audioTimerAcc < 0)
    {
        if (m_retryDefault)
        {
            m_retryDefault = false;
            if (m_audEngine->Reset())
            {
                // Restart looping audio
                m_effect1->Play(true);
            }
        }
        else
        {
            m_audioTimerAcc = 4.f;

            m_waveBank->Play(m_audioEvent++);

            if (m_audioEvent >= 11)
                m_audioEvent = 0;
        }
    }

    if (!m_audEngine->IsCriticalError() && m_audEngine->Update())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
}

void DXTK12SceneRenderer::NewAudioDevice()
{
    if (m_audEngine && !m_audEngine->IsAudioDevicePresent())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
}

void XM_CALLCONV DXTK12SceneRenderer::DrawGrid(FXMVECTOR xAxis, FXMVECTOR yAxis, FXMVECTOR origin, size_t xdivs, size_t ydivs, GXMVECTOR color)
{
    auto context = m_deviceResources->GetD3DDeviceContext11();
    m_batchEffect->Apply(context);

    context->IASetInputLayout(m_batchInputLayout.Get());

    m_batch->Begin();

    xdivs = std::max<size_t>(1, xdivs);
    ydivs = std::max<size_t>(1, ydivs);

    for (size_t i = 0; i <= xdivs; ++i)
    {
        float fPercent = float(i) / float(xdivs);
        fPercent = (fPercent * 2.0f) - 1.0f;
        XMVECTOR vScale = XMVectorScale(xAxis, fPercent);
        vScale = XMVectorAdd(vScale, origin);

        VertexPositionColor v1(XMVectorSubtract(vScale, yAxis), color);
        VertexPositionColor v2(XMVectorAdd(vScale, yAxis), color);
        m_batch->DrawLine(v1, v2);
    }

    for (size_t i = 0; i <= ydivs; i++)
    {
        FLOAT fPercent = float(i) / float(ydivs);
        fPercent = (fPercent * 2.0f) - 1.0f;
        XMVECTOR vScale = XMVectorScale(yAxis, fPercent);
        vScale = XMVectorAdd(vScale, origin);

        VertexPositionColor v1(XMVectorSubtract(vScale, xAxis), color);
        VertexPositionColor v2(XMVectorAdd(vScale, xAxis), color);
        m_batch->DrawLine(v1, v2);
    }

    m_batch->End();
}

void DXTK12SceneRenderer::Render()
{
	auto context = m_deviceResources->GetD3DDeviceContext11();

	// Set render targets to the screen.
	ID3D11RenderTargetView *const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView11() };
	context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView11());

    // Draw procedurally generated dynamic grid
    const XMVECTORF32 xaxis = { 20.f, 0.f, 0.f };
    const XMVECTORF32 yaxis = { 0.f, 0.f, 20.f };
    DrawGrid(xaxis, yaxis, g_XMZero, 20, 20, Colors::Gray);

	DirectX::SpriteSortMode sortMode = DirectX::SpriteSortMode_Deferred;
	DirectX::FXMMATRIX matrix;
    // Draw sprite
    m_sprites->Begin(m_deviceResources->GetCommandList(),
		sortMode,
		matrix,i
	);
	


    m_sprites->Draw(m_texture2.Get(), new XMUINT2(10, 75), nullptr, Colors::White, SetXMVVector2(0,0), 1, SpriteEffects.None, 0, );

    m_font->DrawString(m_sprites.get(), L"DirectXTK GameD12UW", XMFLOAT2(100, 10), Colors::Yellow);
    m_sprites->End();

    // Draw 3D object
    XMMATRIX world = XMLoadFloat4x4(&m_world);
    XMMATRIX view = XMLoadFloat4x4(&m_view);
    XMMATRIX projection = XMLoadFloat4x4(&m_projection);

    XMMATRIX local = XMMatrixMultiply(world, XMMatrixTranslation(-2.f, -2.f, -4.f));
    m_shape->Draw(local, view, projection, Colors::White, m_texture1.Get());

    XMVECTOR qid = XMQuaternionIdentity();
    const XMVECTORF32 scale = { 0.01f, 0.01f, 0.01f };
    const XMVECTORF32 translate = { 3.f, -2.f, -4.f };
    XMVECTOR rotate = XMQuaternionRotationRollPitchYaw(0, XM_PI / 2.f, -XM_PI / 2.f);
    local = XMMatrixMultiply(world, XMMatrixTransformation(g_XMZero, qid, scale, g_XMZero, rotate, translate));
    m_model->Draw(context, *m_states, local, view, projection);
}

void DXTK12SceneRenderer::CreateDeviceDependentResources()
{
    // Create DirectXTK objects
    auto device = m_deviceResources->GetD3DDevice();
    m_states.reset(new CommonStates(device));

    auto fx = new EffectFactory( device );
    fx->SetDirectory( L"Assets" );
    m_fxFactory.reset( fx );

    auto context = m_deviceResources->GetD3DDeviceContext();
    m_sprites.reset(new SpriteBatch(context));
    m_batch.reset(new PrimitiveBatch<VertexPositionColor>(context));

    m_batchEffect.reset(new BasicEffect(device));
    m_batchEffect->SetVertexColorEnabled(true);

    {
        void const* shaderByteCode;
        size_t byteCodeLength;

        m_batchEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

        DX::ThrowIfFailed(
            device->CreateInputLayout(VertexPositionColor::InputElements,
            VertexPositionColor::InputElementCount,
            shaderByteCode, byteCodeLength,
            m_batchInputLayout.ReleaseAndGetAddressOf())
            );
    }

    m_font.reset(new SpriteFont(device, L"assets\\italic.spritefont"));

    m_shape = GeometricPrimitive::CreateTeapot(context, 4.f, 8);

    // SDKMESH has to use clockwise winding with right-handed coordinates, so textures are flipped in U
    m_model = Model::CreateFromSDKMESH(device, L"assets\\tiny.sdkmesh", *m_fxFactory);

    // Load textures
    DX::ThrowIfFailed(
        CreateDDSTextureFromFile(device, L"assets\\seafloor.dds", nullptr, m_texture1.ReleaseAndGetAddressOf())
        );

    DX::ThrowIfFailed(
        CreateDDSTextureFromFile(device, L"assets\\windowslogo.dds", nullptr, m_texture2.ReleaseAndGetAddressOf())
        );
}

void DXTK12SceneRenderer::ReleaseDeviceDependentResources()
{
    m_states.reset();
    m_fxFactory.reset();
    m_sprites.reset();
    m_batch.reset();
    m_batchEffect.reset();
    m_font.reset();
    m_shape.reset();
    m_model.reset();
    m_texture1.Reset();
    m_texture2.Reset();
    m_batchInputLayout.Reset();
}