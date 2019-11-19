//
// GameD12UW.cpp
//

#include "pch.h"
#include "GameD12UWMain.h"
#include "Common/DeviceResources12.h"
#include"Common/StepTimer.h"

// This is the must have path which when you build yours, check the DirectXTK.lib line for the correct path and put it in here
// It save a ton of link erors you can't figure out until you have this correct and WOW!!!!
//#pragma comment(lib, "..\\..\\..\\..\\..\\GameD12UW\\DirectXTK12\\Bin\\Windows10_2019\\x64\\Debug\\DirectXTK12.lib")
// This was left but a reference can be added to the DirectXTK12 by right clicking this project, 
// choosing from the menu: Add, then Reference, and Selecting the DirectXTK12 checkbox, OK

extern void ExitGameD12UW();

using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace Windows::UI::Core;
using namespace Windows::Graphics::Display;


using Microsoft::WRL::ComPtr;

GameD12UW::GameD12UW()
{
	m_deviceResources = std::make_unique<DX::DeviceResources12>();
	m_deviceResources->RegisterDeviceNotify(this);
}

// For a Windows Store app or Universal Windows Platform(UWP) app, add the following to your existing
// application methods:
//
// void App::SetWindow(CoreWindow^ window )
// {
//     m_mouse->SetWindow(window);
// }
// 
// void App::OnDpiChanged(DisplayInformation^ sender, Object^ args)
// {
//     m_mouse->SetDpi(sender->LogicalDpi);
// }

void GameD12UW::SetWindow(Windows::UI::Core::CoreWindow^ window)
{
	m_keyboard->SetWindow(window);
     m_mouse->SetWindow(window);
}
// 
 void GameD12UW::OnDpiChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args)
 {
	 
     m_mouse->SetDpi(sender->LogicalDpi);
 }

// Initialize the Direct3D resources required to run.
void GameD12UW::Initialize(CoreWindow^ window, int width, int height, DXGI_MODE_ROTATION rotation)
{
	//m_gamePad = std::make_unique<GamePad>();
	m_gamePad = std::make_unique<DirectX::GamePad>();
	// reinterpret_cast<ABI::Windows::UI::Core::ICoreWindow*>(window) for ->SetWindow(params);
	m_keyboard = std::make_unique<DirectX::Keyboard>();
	m_keyboard->SetWindow((window));
    m_mouse = std::make_unique<DirectX::Mouse>();
	m_mouse->SetWindow((window));
	SetWindow(window);

	m_deviceResources->SetWindow((window), width, height, rotation);

	m_deviceResources->CreateDeviceResources();
	CreateDeviceDependentResources();

	m_deviceResources->CreateWindowSizeDependentResources();
	CreateWindowSizeDependentResources();

	// Create DirectXTK for Audio objects
	AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
	eflags = eflags | AudioEngine_Debug;
#endif

	m_audEngine = std::make_unique<AudioEngine>(eflags);

	m_audioEvent = 0;
	m_audioTimerAcc = 10.f;
	m_retryDefault = false;

	m_waveBank = std::make_unique<WaveBank>(m_audEngine.get(), L"assets\\adpcmdroid.xwb");
	m_soundEffect = std::make_unique<SoundEffect>(m_audEngine.get(), L"assets\\MusicMono_adpcm.wav");
	m_effect1 = m_soundEffect->CreateInstance();
	m_effect2 = m_waveBank->CreateInstance(10);

	m_effect1->Play(true);
	m_effect2->Play();
}

#pragma region Frame Update
// Executes basic render loop.
void GameD12UW::Tick()
{
	m_timer.Tick([&]()
	{
		Update(m_timer);
	});

	// Only update audio engine once per frame
	if (!m_audEngine->IsCriticalError() && m_audEngine->Update())
	{
		// Setup a retry in 1 second
		m_audioTimerAcc = 1.f;
		m_retryDefault = true;
	}

	Render();
}


// Updates the world.
void GameD12UW::Update(DX::StepTimer const& timer)
{
	PIXBeginEvent(PIX_COLOR_DEFAULT, L"Update");

	Vector3 eye(0.0f, 0.7f, 1.5f);
	Vector3 at(0.0f, -0.1f, 0.0f);

	m_view = Matrix::CreateLookAt(eye, at, Vector3::UnitY);

	m_world = Matrix::CreateRotationY(float(timer.GetTotalSeconds() * XM_PIDIV4));

	m_lineEffect->SetView(m_view);
	m_lineEffect->SetWorld(Matrix::Identity);

	m_shapeEffect->SetView(m_view);

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
	
	auto pad = m_gamePad->GetState(0);
	if (pad.IsConnected())
	{
		m_gamePadButtons.Update(pad);

		if (pad.IsViewPressed())
		{
			ExitGameD12UW();
		}
	}
	else
	{
		m_gamePadButtons.Reset();
	}

	auto kb = m_keyboard->GetState();
	m_keyboardButtons.Update(kb);

	if (kb.Escape)
	{
		ExitGameD12UW();
	}

	auto mouse = m_mouse->GetState();
	mouse;
	
	PIXEndEvent();
}

#pragma endregion

#pragma region Frame Render
// Draws the scene.
void GameD12UW::Render()
{
	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0)
	{
		return;
	}

	// Prepare the command list to render a new frame.
	m_deviceResources->Prepare();
	Clear();

	auto commandList = m_deviceResources->GetCommandList();
	PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");

	// Draw procedurally generated dynamic grid
	const XMVECTORF32 xaxis = { 20.f, 0.f, 0.f };
	const XMVECTORF32 yaxis = { 0.f, 0.f, 20.f };
	DrawGrid(xaxis, yaxis, g_XMZero, 20, 20, Colors::Gray);

	// Set the descriptor heaps
	ID3D12DescriptorHeap* heaps[] = { m_resourceDescriptors->Heap(), m_states->Heap() };
	commandList->SetDescriptorHeaps(_countof(heaps), heaps);

	// Draw sprite
	PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Draw sprite");
	m_sprites->Begin(commandList);
	m_sprites->Draw(m_resourceDescriptors->GetGpuHandle(Descriptors::WindowsLogo), GetTextureSize(m_texture2.Get()),
		XMFLOAT2(10, 75));

	m_font->DrawString(m_sprites.get(), L"Hot3dx DirectXTK GameD12UW", XMFLOAT2(100, 10), Colors::Yellow);
	m_sprites->End();
	PIXEndEvent(commandList);

	// Draw 3D object
	PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Draw teapot");
	XMMATRIX local = m_world * Matrix::CreateTranslation(-2.f, -2.f, -4.f);
	m_shapeEffect->SetWorld(local);
	m_shapeEffect->Apply(commandList);
	m_shape->Draw(commandList);
	PIXEndEvent(commandList);

	// Draw model
	PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Draw model");
	const XMVECTORF32 scale = { 0.01f, 0.01f, 0.01f };
	const XMVECTORF32 translate = { 3.f, -2.f, -4.f };
	XMVECTOR rotate = Quaternion::CreateFromYawPitchRoll(XM_PI / 2.f, 0.f, -XM_PI / 2.f);
	local = m_world * XMMatrixTransformation(g_XMZero, Quaternion::Identity, scale, g_XMZero, rotate, translate);
	Model::UpdateEffectMatrices(m_modelEffects, local, m_view, m_projection);
	heaps[0] = m_modelResources->Heap();
	commandList->SetDescriptorHeaps(_countof(heaps), heaps);
	m_model->Draw(commandList, m_modelEffects.begin());
	PIXEndEvent(commandList);

	PIXEndEvent(commandList);

	// Show the new frame.
	PIXBeginEvent(m_deviceResources->GetCommandQueue(), PIX_COLOR_DEFAULT, L"Present");
	m_deviceResources->Present();
	m_graphicsMemory->Commit(m_deviceResources->GetCommandQueue());
	PIXEndEvent(m_deviceResources->GetCommandQueue());
}

// Helper method to clear the back buffers.
void GameD12UW::Clear()
{
	auto commandList = m_deviceResources->GetCommandList();
	PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Clear");

	// Clear the views.
	auto rtvDescriptor = m_deviceResources->GetRenderTargetView();
	auto dsvDescriptor = m_deviceResources->GetDepthStencilView();

	commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);
	commandList->ClearRenderTargetView(rtvDescriptor, Colors::CornflowerBlue, 0, nullptr);
	commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// Set the viewport and scissor rect.
	auto viewport = m_deviceResources->GetScreenViewport();
	auto scissorRect = m_deviceResources->GetScissorRect();
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);

	PIXEndEvent(commandList);
}

void XM_CALLCONV GameD12UW::DrawGrid(FXMVECTOR xAxis, FXMVECTOR yAxis, FXMVECTOR origin, size_t xdivs, size_t ydivs, GXMVECTOR color)
{
	auto commandList = m_deviceResources->GetCommandList();
	PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Draw grid");

	m_lineEffect->Apply(commandList);

	m_batch->Begin(commandList);

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
		float fPercent = float(i) / float(ydivs);
		fPercent = (fPercent * 2.0f) - 1.0f;
		XMVECTOR vScale = XMVectorScale(yAxis, fPercent);
		vScale = XMVectorAdd(vScale, origin);

		VertexPositionColor v1(XMVectorSubtract(vScale, xAxis), color);
		VertexPositionColor v2(XMVectorAdd(vScale, xAxis), color);
		m_batch->DrawLine(v1, v2);
	}

	m_batch->End();

	PIXEndEvent(commandList);
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void GameD12UW::OnActivated()
{
}

void GameD12UW::OnDeactivated()
{
}

void GameD12UW::OnSuspending()
{
	m_audEngine->Suspend();
}

void GameD12UW::OnResuming()
{
	m_timer.ResetElapsedTime();
	m_gamePadButtons.Reset();
	m_keyboardButtons.Reset();
	m_audEngine->Resume();
}

void GameD12UW::OnWindowSizeChanged(int width, int height, DXGI_MODE_ROTATION rotation)
{
	if (!m_deviceResources->WindowSizeChanged(width, height, rotation))
		return;

	CreateWindowSizeDependentResources();
}

void GameD12UW::ValidateDevice()
{
	m_deviceResources->ValidateDevice();
}

void GameD12UW::NewAudioDevice()
{
	if (m_audEngine && !m_audEngine->IsAudioDevicePresent())
	{
		// Setup a retry in 1 second
		m_audioTimerAcc = 1.f;
		m_retryDefault = true;
	}
}

// Properties
void GameD12UW::GetDefaultSize(int& width, int& height) const
{
	width = 1280;
	height = 720;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void GameD12UW::CreateDeviceDependentResources()
{
	auto device = m_deviceResources->GetD3DDevice();

	m_graphicsMemory = std::make_unique<GraphicsMemory>(device);

	m_states = std::make_unique<CommonStates>(device);

	m_resourceDescriptors = std::make_unique<DescriptorHeap>(device,
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
		Descriptors::Count);

	m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(device);

	m_shape = GeometricPrimitive::CreateTeapot(4.f, 8);

	
	// SDKMESH has to use clockwise winding with right-handed coordinates, so textures are flipped in U
	//   // C:\\Users\\hot3dx-home\\Source\\Repos\\GameD12UW\\Assets\\


	m_model = Model::CreateFromSDKMESH(L"assets\\tiny.sdkmesh");//, device);
	{
		ResourceUploadBatch resourceUpload(device);

		resourceUpload.Begin();

		m_model->LoadStaticBuffers(device, resourceUpload);

		DX::ThrowIfFailed(
			CreateDDSTextureFromFile(device, resourceUpload, L"assets\\seafloor.dds", m_texture1.ReleaseAndGetAddressOf())
		);

		CreateShaderResourceView(device, m_texture1.Get(), m_resourceDescriptors->GetCpuHandle(Descriptors::SeaFloor));

		DX::ThrowIfFailed(
			CreateDDSTextureFromFile(device, resourceUpload, L"assets\\windowslogo.dds", m_texture2.ReleaseAndGetAddressOf())
		);

		CreateShaderResourceView(device, m_texture2.Get(), m_resourceDescriptors->GetCpuHandle(Descriptors::WindowsLogo));

		RenderTargetState rtState(m_deviceResources->GetBackBufferFormat(), m_deviceResources->GetDepthBufferFormat());

		{
			EffectPipelineStateDescription pd(
				&VertexPositionColor::InputLayout,
				CommonStates::Opaque,
				CommonStates::DepthNone,
				CommonStates::CullNone,
				rtState,
				D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);

			m_lineEffect = std::make_unique<BasicEffect>(device, EffectFlags::VertexColor, pd);
		}

		{
			EffectPipelineStateDescription pd(
				&GeometricPrimitive::VertexType::InputLayout,
				CommonStates::Opaque,
				CommonStates::DepthDefault,
				CommonStates::CullNone,
				rtState);

			m_shapeEffect = std::make_unique<BasicEffect>(device, EffectFlags::PerPixelLighting | EffectFlags::Texture, pd);
			m_shapeEffect->EnableDefaultLighting();
			m_shapeEffect->SetTexture(m_resourceDescriptors->GetGpuHandle(Descriptors::SeaFloor), m_states->AnisotropicWrap());
		}

		{
			SpriteBatchPipelineStateDescription pd(rtState);

			m_sprites = std::make_unique<SpriteBatch>(device, resourceUpload, pd);
		}

		m_modelResources = m_model->LoadTextures(device, resourceUpload, L"assets\\");

		{
			EffectPipelineStateDescription psd(
				nullptr,
				CommonStates::Opaque,
				CommonStates::DepthDefault,
				CommonStates::CullClockwise,    // Using RH coordinates, and SDKMESH is in LH coordiantes
				rtState);

			EffectPipelineStateDescription alphapsd(
				nullptr,
				CommonStates::NonPremultiplied, // Using straight alpha
				CommonStates::DepthRead,
				CommonStates::CullClockwise,    // Using RH coordinates, and SDKMESH is in LH coordiantes
				rtState);

			m_modelEffects = m_model->CreateEffects(psd, alphapsd, m_modelResources->Heap(), m_states->Heap());
		}

		//DirectX::BinaryReader* reader
		//std::unique_ptr<DirectX::SpriteFont> m_smallFont = std::make_unique<DirectX::SpriteFont>(&device, L"SegoeUI_18.spritefont");
		m_font = std::make_unique<SpriteFont>(device, resourceUpload,
			L"SegoeUI_18.spritefont",
			m_resourceDescriptors->GetCpuHandle(Descriptors::SegoeFont),
			m_resourceDescriptors->GetGpuHandle(Descriptors::SegoeFont));

		// Upload the resources to the GPU.
		auto uploadResourcesFinished = resourceUpload.End(m_deviceResources->GetCommandQueue());

		// Wait for the upload thread to terminate
		uploadResourcesFinished.wait();
	}
}

// Allocate all memory resources that change on a window SizeChanged event.
void GameD12UW::CreateWindowSizeDependentResources()
{
	auto size = m_deviceResources->GetOutputSize();
	float aspectRatio = float(size.right) / float(size.bottom);
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
	Matrix perspectiveMatrix = Matrix::CreatePerspectiveFieldOfView(
		fovAngleY,
		aspectRatio,
		0.01f,
		100.0f
	);

	Matrix orientationMatrix = m_deviceResources->GetOrientationTransform3D();

	m_projection = perspectiveMatrix * orientationMatrix;

	m_lineEffect->SetProjection(m_projection);
	m_shapeEffect->SetProjection(m_projection);

	auto viewport = m_deviceResources->GetScreenViewport();
	m_sprites->SetViewport(viewport);

	m_sprites->SetRotation(m_deviceResources->GetRotation());
}

void GameD12UW::OnDeviceLost()
{
	m_texture1.Reset();
	m_texture2.Reset();

	m_font.reset();
	m_batch.reset();
	m_shape.reset();
	m_model.reset();
	m_lineEffect.reset();
	m_shapeEffect.reset();
	m_modelEffects.clear();
	m_modelResources.reset();
	m_sprites.reset();
	m_resourceDescriptors.reset();
	m_states.reset();
	m_graphicsMemory.reset();
}

void GameD12UW::OnDeviceRestored()
{
	CreateDeviceDependentResources();

	CreateWindowSizeDependentResources();
}
#pragma endregion
