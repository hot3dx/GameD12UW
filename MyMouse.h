//--------------------------------------------------------------------------------------
// File: MyMouse.h
//
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkId=248929
// http://go.microsoft.com/fwlink/?LinkID=615561
//--------------------------------------------------------------------------------------

#pragma once

#include <memory>

//#if (defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP))// || (defined(_XBOX_ONE) && defined(_TITLE) && (_XDK_VER >= 0x42D907D1))
//namespace ABI { namespace Windows { namespace UI { namespace Core { struct ICoreWindow; } } } }
//#endif

namespace DX
{
	class MyMouse
	{
	public:
		MyMouse() noexcept(false);
		MyMouse(MyMouse&& moveFrom) noexcept;
		MyMouse& operator= (MyMouse&& moveFrom) noexcept;

		MyMouse(MyMouse const&) = delete;
		MyMouse& operator=(MyMouse const&) = delete;

		virtual ~MyMouse();

		enum Mode
		{
			MODE_ABSOLUTE = 0,
			MODE_RELATIVE,
		};

		struct State
		{
			bool    leftButton;
			bool    middleButton;
			bool    rightButton;
			bool    xButton1;
			bool    xButton2;
			int     x;
			int     y;
			int     scrollWheelValue;
			Mode    positionMode;
		};

		class ButtonStateTracker
		{
		public:
			enum ButtonState
			{
				UP = 0,         // Button is up
				HELD = 1,       // Button is held down
				RELEASED = 2,   // Button was just released
				PRESSED = 3,    // Buton was just pressed
			};

			ButtonState leftButton;
			ButtonState middleButton;
			ButtonState rightButton;
			ButtonState xButton1;
			ButtonState xButton2;

#pragma prefast(suppress: 26495, "Reset() performs the initialization")
			ButtonStateTracker() noexcept { Reset(); }

			void __cdecl Update(const State& state);

			void __cdecl Reset() noexcept;

			State __cdecl GetLastState() const { return lastState; }

		private:
			State lastState;
		};

		// Retrieve the current state of the mouse
		State __cdecl GetState() const;

		// Resets the accumulated scroll wheel value
		void __cdecl ResetScrollWheelValue();

		// Sets mouse mode (defaults to absolute)
		void __cdecl SetMode(Mode mode);

		// Feature detection
		bool __cdecl IsConnected() const;

		// Cursor visibility
		bool __cdecl IsVisible() const;
		void __cdecl SetVisible(bool visible);

#ifdef __cplusplus_winrt
		void __cdecl SetWindow(Windows::UI::Core::CoreWindow^ window)
		{
			// See https://msdn.microsoft.com/en-us/library/hh755802.aspx
			SetWindow(window);
		}
#endif

		static void __cdecl SetDpi(float dpi);

		// Singleton
		static MyMouse& __cdecl Get();

	private:
		// Private implementation.
		class Impl;

		std::unique_ptr<Impl> pImpl;
	};
}

