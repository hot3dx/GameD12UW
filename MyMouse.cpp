#include "pch.h"
#include "MyMouse.h"

#include "MyPlatformHelpers.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;
#include <Windows.Devices.Input.h>

using namespace DX;

class DX::MyMouse::Impl
{
	

public:
	Impl(MyMouse* owner) :
		mState{},
		mOwner(owner),
		mDPI(96.f),
		mMode(MODE_ABSOLUTE),
		mPointerPressedToken{},
		mPointerReleasedToken{},
		mPointerMovedToken{},
		mPointerWheelToken{},
		mPointerMouseMovedToken{}
	{
		if (s_mouse)
		{
			throw std::exception("MyMouse is a singleton");
		}

		s_mouse = this;

		mScrollWheelValue.reset(CreateEventEx(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_MODIFY_STATE | SYNCHRONIZE));
		mRelativeRead.reset(CreateEventEx(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_MODIFY_STATE | SYNCHRONIZE));
		if (!mScrollWheelValue
			|| !mRelativeRead)
		{
			throw std::exception("CreateEventEx");
		}
	}

	~Impl()
	{
		s_mouse = nullptr;

		RemoveHandlers();
	}

	void GetState(State& state) const
	{
		memcpy(&state, &mState, sizeof(State));

		DWORD result = WaitForSingleObjectEx(mScrollWheelValue.get(), 0, FALSE);
		if (result == WAIT_FAILED)
			throw std::exception("WaitForSingleObjectEx");

		if (result == WAIT_OBJECT_0)
		{
			state.scrollWheelValue = 0;
		}

		if (mMode == MODE_RELATIVE)
		{
			result = WaitForSingleObjectEx(mRelativeRead.get(), 0, FALSE);

			if (result == WAIT_FAILED)
				throw std::exception("WaitForSingleObjectEx");

			if (result == WAIT_OBJECT_0)
			{
				state.x = 0;
				state.y = 0;
			}
			else
			{
				SetEvent(mRelativeRead.get());
			}
		}

		state.positionMode = mMode;
	}

	void ResetScrollWheelValue()
	{
		SetEvent(mScrollWheelValue.get());
	}

	void SetMode(Mode mode)
	{
		using namespace Microsoft::WRL;
		using namespace Microsoft::WRL::Wrappers;
		using namespace Windows::UI::Core;
		using namespace Windows::Foundation;

		if (mMode == mode)
			return;

		ComPtr<ICoreWindowStatic> statics;
		HRESULT hr = GetActivationFactory(HStringReference(RuntimeClass_Windows_UI_Core_CoreWindow).Get(), statics.GetAddressOf());
		ThrowIfFailed(hr);

		ComPtr<ICoreWindow> window;
		hr = statics->GetForCurrentThread(window.GetAddressOf());
		ThrowIfFailed(hr);

		if (mode == MODE_RELATIVE)
		{
			hr = window->get_PointerCursor(mCursor.ReleaseAndGetAddressOf());
			ThrowIfFailed(hr);

			hr = window->put_PointerCursor(nullptr);
			ThrowIfFailed(hr);

			SetEvent(mRelativeRead.get());

			mMode = MODE_RELATIVE;
		}
		else
		{
			if (!mCursor)
			{
				ComPtr<ICoreCursorFactory> factory;
				hr = GetActivationFactory(HStringReference(RuntimeClass_Windows_UI_Core_CoreCursor).Get(), factory.GetAddressOf());
				ThrowIfFailed(hr);

				hr = factory->CreateCursor(CoreCursorType_Arrow, 0, mCursor.GetAddressOf());
				ThrowIfFailed(hr);
			}

			hr = window->put_PointerCursor(mCursor.Get());
			ThrowIfFailed(hr);

			mCursor.Reset();

			mMode = MODE_ABSOLUTE;
		}
	}

	bool IsConnected() const
	{
		using namespace Microsoft::WRL;
		using namespace Microsoft::WRL::Wrappers;
		using namespace Windows::Devices::Input;
		using namespace Windows::Foundation;

		ComPtr<IMouseCapabilities> caps;
		HRESULT hr = RoActivateInstance(HStringReference(RuntimeClass_Windows_Devices_Input_MouseCapabilities).Get(), &caps);
		ThrowIfFailed(hr);

		INT32 value;
		if (SUCCEEDED(caps->get_MousePresent(&value)))
		{
			return value != 0;
		}

		return false;
	}

	bool IsVisible() const
	{
		if (mMode == MODE_RELATIVE)
			return false;

		ComPtr<Windows::UI::Core::ICoreCursor> cursor;
		HRESULT hr = mWindow->get_PointerCursor(cursor.GetAddressOf());
		ThrowIfFailed(hr);

		return cursor != 0;
	}

	void SetVisible(bool visible)
	{
		using namespace Microsoft::WRL::Wrappers;
		using namespace Windows::Foundation;
		using namespace Windows::UI::Core;

		if (mMode == MODE_RELATIVE)
			return;

		if (visible)
		{
			if (!mCursor)
			{
				ComPtr<ICoreCursorFactory> factory;
				HRESULT hr = GetActivationFactory(HStringReference(RuntimeClass_Windows_UI_Core_CoreCursor).Get(), factory.GetAddressOf());
				ThrowIfFailed(hr);

				hr = factory->CreateCursor(CoreCursorType_Arrow, 0, mCursor.GetAddressOf());
				ThrowIfFailed(hr);
			}

			HRESULT hr = mWindow->put_PointerCursor(mCursor.Get());
			ThrowIfFailed(hr);
		}
		else
		{
			HRESULT hr = mWindow->put_PointerCursor(nullptr);
			ThrowIfFailed(hr);
		}
	}

	void SetWindow(Windows::UI::Core::ICoreWindow* window)
	{
		using namespace Microsoft::WRL;
		using namespace Microsoft::WRL::Wrappers;
		using namespace Windows::Foundation;
		using namespace Windows::Devices::Input;

		if (mWindow.Get() == window)
			return;

		RemoveHandlers();

		mWindow = window;

		if (!window)
		{
			mCursor.Reset();
			mMouse.Reset();
			return;
		}

		ComPtr<IMouseDeviceStatics> mouseStatics;
		HRESULT hr = GetActivationFactory(HStringReference(RuntimeClass_Windows_Devices_Input_MouseDevice).Get(), mouseStatics.GetAddressOf());
		ThrowIfFailed(hr);

		hr = mouseStatics->GetForCurrentView(mMouse.ReleaseAndGetAddressOf());
		ThrowIfFailed(hr);

		typedef __FITypedEventHandler_2_Windows__CDevices__CInput__CMouseDevice_Windows__CDevices__CInput__CMouseEventArgs MouseMovedHandler;
		hr = mMouse->add_MouseMoved(Callback<MouseMovedHandler>(MouseMovedEvent).Get(), &mPointerMouseMovedToken);
		ThrowIfFailed(hr);

		typedef __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CPointerEventArgs PointerHandler;
		auto cb = Callback<PointerHandler>(PointerEvent);

		hr = window->add_PointerPressed(cb.Get(), &mPointerPressedToken);
		ThrowIfFailed(hr);

		hr = window->add_PointerReleased(cb.Get(), &mPointerReleasedToken);
		ThrowIfFailed(hr);

		hr = window->add_PointerMoved(cb.Get(), &mPointerMovedToken);
		ThrowIfFailed(hr);

		hr = window->add_PointerWheelChanged(Callback<PointerHandler>(PointerWheel).Get(), &mPointerWheelToken);
		ThrowIfFailed(hr);
	}

	State           mState;
	MyMouse* mOwner;
	float           mDPI;

	static MyMouse::Impl* s_mouse;

private:
	Mode            mMode;

	ComPtr<Windows::UI::Core::ICoreWindow> mWindow;
	ComPtr<Windows::Devices::Input::IMouseDevice> mMouse;
	ComPtr<Windows::UI::Core::ICoreCursor> mCursor;

	ScopedHandle    mScrollWheelValue;
	ScopedHandle    mRelativeRead;

	EventRegistrationToken mPointerPressedToken;
	EventRegistrationToken mPointerReleasedToken;
	EventRegistrationToken mPointerMovedToken;
	EventRegistrationToken mPointerWheelToken;
	EventRegistrationToken mPointerMouseMovedToken;

	void RemoveHandlers()
	{
		if (mWindow)
		{
			(void)mWindow->remove_PointerPressed(mPointerPressedToken);
			mPointerPressedToken.value = 0;

			(void)mWindow->remove_PointerReleased(mPointerReleasedToken);
			mPointerReleasedToken.value = 0;

			(void)mWindow->remove_PointerMoved(mPointerMovedToken);
			mPointerMovedToken.value = 0;

			(void)mWindow->remove_PointerWheelChanged(mPointerWheelToken);
			mPointerWheelToken.value = 0;
		}

		if (mMouse)
		{
			(void)mMouse->remove_MouseMoved(mPointerMouseMovedToken);
			mPointerMouseMovedToken.value = 0;
		}
	}

	static HRESULT PointerEvent(IInspectable*, Windows::UI::Core::IPointerEventArgs* args)
	{
		using namespace Windows::Foundation;
		using namespace Windows::UI::Input;
		using namespace Windows::Devices::Input;

		if (!s_mouse)
			return S_OK;

		ComPtr<IPointerPoint> currentPoint;
		HRESULT hr = args->get_CurrentPoint(currentPoint.GetAddressOf());
		ThrowIfFailed(hr);

		ComPtr<IPointerDevice> pointerDevice;
		hr = currentPoint->get_PointerDevice(pointerDevice.GetAddressOf());
		ThrowIfFailed(hr);

		PointerDeviceType devType;
		hr = pointerDevice->get_PointerDeviceType(&devType);
		ThrowIfFailed(hr);

		if (devType == PointerDeviceType::PointerDeviceType_Mouse)
		{
			ComPtr<IPointerPointProperties> props;
			hr = currentPoint->get_Properties(props.GetAddressOf());
			ThrowIfFailed(hr);

			boolean value;
			hr = props->get_IsLeftButtonPressed(&value);
			ThrowIfFailed(hr);
			s_mouse->mState.leftButton = value != 0;

			hr = props->get_IsRightButtonPressed(&value);
			ThrowIfFailed(hr);
			s_mouse->mState.rightButton = value != 0;

			hr = props->get_IsMiddleButtonPressed(&value);
			ThrowIfFailed(hr);
			s_mouse->mState.middleButton = value != 0;

			hr = props->get_IsXButton1Pressed(&value);
			ThrowIfFailed(hr);
			s_mouse->mState.xButton1 = value != 0;

			hr = props->get_IsXButton2Pressed(&value);
			ThrowIfFailed(hr);
			s_mouse->mState.xButton2 = value != 0;
		}

		if (s_mouse->mMode == MODE_ABSOLUTE)
		{
			Point pos;
			hr = currentPoint->get_Position(&pos);
			ThrowIfFailed(hr);

			float dpi = s_mouse->mDPI;

			s_mouse->mState.x = static_cast<int>(pos.X * dpi / 96.f + 0.5f);
			s_mouse->mState.y = static_cast<int>(pos.Y * dpi / 96.f + 0.5f);
		}

		return S_OK;
	}

	static HRESULT PointerWheel(IInspectable*, Windows::UI::Core::IPointerEventArgs* args)
	{
		using namespace Windows::Foundation;
		using namespace Windows::UI::Input;
		using namespace Windows::Devices::Input;

		if (!s_mouse)
			return S_OK;

		ComPtr<IPointerPoint> currentPoint;
		HRESULT hr = args->get_CurrentPoint(currentPoint.GetAddressOf());
		ThrowIfFailed(hr);

		ComPtr<IPointerDevice> pointerDevice;
		hr = currentPoint->get_PointerDevice(pointerDevice.GetAddressOf());
		ThrowIfFailed(hr);

		PointerDeviceType devType;
		hr = pointerDevice->get_PointerDeviceType(&devType);
		ThrowIfFailed(hr);

		if (devType == PointerDeviceType::PointerDeviceType_Mouse)
		{
			ComPtr<IPointerPointProperties> props;
			hr = currentPoint->get_Properties(props.GetAddressOf());
			ThrowIfFailed(hr);

			INT32 value;
			hr = props->get_MouseWheelDelta(&value);
			ThrowIfFailed(hr);

			HANDLE evt = s_mouse->mScrollWheelValue.get();
			if (WaitForSingleObjectEx(evt, 0, FALSE) == WAIT_OBJECT_0)
			{
				s_mouse->mState.scrollWheelValue = 0;
				ResetEvent(evt);
			}

			s_mouse->mState.scrollWheelValue += value;

			if (s_mouse->mMode == MODE_ABSOLUTE)
			{
				Point pos;
				hr = currentPoint->get_Position(&pos);
				ThrowIfFailed(hr);

				float dpi = s_mouse->mDPI;

				s_mouse->mState.x = static_cast<int>(pos.X * dpi / 96.f + 0.5f);
				s_mouse->mState.y = static_cast<int>(pos.Y * dpi / 96.f + 0.5f);
			}
		}

		return S_OK;
	}

	static HRESULT MouseMovedEvent(IInspectable*, Windows::Devices::Input::IMouseEventArgs* args)
	{
		using namespace Windows::Devices::Input;

		if (!s_mouse)
			return S_OK;

		if (s_mouse->mMode == MODE_RELATIVE)
		{
			MouseDelta delta;
			HRESULT hr = args->get_MouseDelta(&delta);
			ThrowIfFailed(hr);

			s_mouse->mState.x = delta.X;
			s_mouse->mState.y = delta.Y;

			ResetEvent(s_mouse->mRelativeRead.get());
		}

		return S_OK;
	}
};


MyMouse::Impl* MyMouse::Impl::s_mouse = nullptr;


void MyMouse::SetWindow(Windows::UI::Core::CoreWindow^ window)
{
	pImpl->SetWindow(window);
}


void MyMouse::SetDpi(float dpi)
{
	auto pImpl = Impl::s_mouse;

	if (!pImpl)
		return;

	pImpl->mDPI = dpi;
}

#pragma warning( disable : 4355 )

// Public constructor.
MyMouse::MyMouse() noexcept(false)
	: pImpl(std::make_unique<Impl>(this))
{
}


// Move constructor.
MyMouse::MyMouse(MyMouse&& moveFrom) noexcept
	: pImpl(std::move(moveFrom.pImpl))
{
	pImpl->mOwner = this;
}


// Move assignment.
MyMouse& MyMouse::operator= (MyMouse&& moveFrom) noexcept
{
	pImpl = std::move(moveFrom.pImpl);
	pImpl->mOwner = this;
	return *this;
}


// Public destructor.
MyMouse::~MyMouse()
{
}


MyMouse::State MyMouse::GetState() const
{
	State state;
	pImpl->GetState(state);
	return state;
}


void MyMouse::ResetScrollWheelValue()
{
	pImpl->ResetScrollWheelValue();
}


void MyMouse::SetMode(Mode mode)
{
	pImpl->SetMode(mode);
}


bool MyMouse::IsConnected() const
{
	return pImpl->IsConnected();
}

bool MyMouse::IsVisible() const
{
	return pImpl->IsVisible();
}

void MyMouse::SetVisible(bool visible)
{
	pImpl->SetVisible(visible);
}

MyMouse& MyMouse::Get()
{
	if (!Impl::s_mouse || !Impl::s_mouse->mOwner)
		throw std::exception("MyMouse is a singleton");

	return *Impl::s_mouse->mOwner;
}



//======================================================================================
// ButtonStateTracker
//======================================================================================

#define UPDATE_BUTTON_STATE(field) field = static_cast<ButtonState>( ( !!state.field ) | ( ( !!state.field ^ !!lastState.field ) << 1 ) );

void MyMouse::ButtonStateTracker::Update(const MyMouse::State& state)
{
	UPDATE_BUTTON_STATE(leftButton)

		assert((!state.leftButton && !lastState.leftButton) == (leftButton == UP));
	assert((state.leftButton && lastState.leftButton) == (leftButton == HELD));
	assert((!state.leftButton && lastState.leftButton) == (leftButton == RELEASED));
	assert((state.leftButton && !lastState.leftButton) == (leftButton == PRESSED));

	UPDATE_BUTTON_STATE(middleButton)
		UPDATE_BUTTON_STATE(rightButton)
		UPDATE_BUTTON_STATE(xButton1)
		UPDATE_BUTTON_STATE(xButton2)

		lastState = state;
}

#undef UPDATE_BUTTON_STATE


void MyMouse::ButtonStateTracker::Reset() noexcept
{
	memset(this, 0, sizeof(ButtonStateTracker));
}