#pragma once

#include <WinUser.h>
#include <string>
#include <sstream>
#include <iostream>
#include <utility>
#include "imgui/imgui.h"


DWORD keys[] = { VK_LMENU, VK_MENU, VK_SHIFT, VK_RSHIFT, VK_CONTROL, VK_RCONTROL, VK_LBUTTON, VK_RBUTTON, VK_XBUTTON1, VK_XBUTTON2 };
const char* keyItems[] = { "LAlt", "RAlt", "LShift", "RShift", "LControl", "RControl", "LMouse", "RMouse", "Mouse4", "Mouse5"};

inline namespace Configuration
{
	class Settings : public Option<Settings>
	{
	public:
		const char* AimType[2] = { "head", "Body" };
		const char* BoxTypes[2] = { "Full Box","Cornered Box" };
		const char* LineTypes[3] = { "Bottom To Enemy","Top To Enemy","Crosshair To Enemy" };

		bool b_thirdPerson = false;
		bool b_glowEnabled = false;
		bool b_MenuShow = true;
		bool b_Aimbot = false;
		bool b_AimFOV = false;
		bool b_isPredictionAim = false;
		bool b_Smoothing = false;
		bool b_LockWhenClose = false;
		bool b_Locked;

		bool b_TracerMenu = false;
		bool b_SpecialBox = false;
		bool b_Misc = false;
		bool b_Visual = false;
		bool b_Boxtracer = false;
		bool b_EspBox = false;
		bool b_EspName = false;
		bool b_EspHealth = false;
		bool b_EspLine = false;
		bool b_EspLineTracer = false;
		bool b_AIm = false;

		ImColor BoxVisColor = ImColor(255.f / 255, 0.f, 0.f, 1.f);
		ImColor HealthBarColor = ImColor(0.f, 255.f / 255.f, 0.f, 1.f);
		ImColor NameBarColor = ImColor(0.f, 255.f / 255.f, 0.f, 1.f);
		ImColor LineColor = ImColor(0.f, 0.f, 255.f / 255, 1.f);
		ImColor FovColor = ImColor(255.f / 255, 0.f, 0.f, 1.f);

		float fl_BoxVisColor[4] = { 255.f / 255,0.f,0.f,1.f };//
		float fl_HealthBarColor[4] = { 0.f,255.f / 255,0.f,1.f };//
		float fl_NameBarColor[4] = { 0.f,255.f / 255,0.f,1.f };//
		float fl_LineColor[4] = { 0,0,255.f / 255,1 };  //
		float fl_FovColor[4] = { 255.f / 255,0.f,0.f,1.f };  //
		float fl_red[4] = { 255.f / 255,0.f,0.f,1.f };  //

		float fl_CurrentFOV;
		float fl_SmoothingValue = 0.1f; // from 0-1
		float fl_Speed = 7000.0f;
		float fl_AimFov = 20.f;

		int in_AimType = 0;
		int in_BoxType = 0;
		int in_LineType = 0;
		int in_CurrentHealth;
		int in_CurrentLoopFrame;
		int in_tab_index = 0;
		int in_AimKey = 0;
	};
#define PKU Configuration::Settings::Get()
}

bool GetAimKey()
{
	return GetAsyncKeyState(keys[PKU.in_AimKey]);
}


namespace
{
	constexpr int const_atoi(char c)
	{
		return c - '0';
	}
}

template<typename T>
class Option
{
protected:
	Option() {}
	~Option() {}

	Option(const Option&) = delete;
	Option& operator=(const Option&) = delete;

	Option(Option&&) = delete;
	Option& operator=(Option&&) = delete;
public:
	static T& Get()
	{
		static T inst{};
		return inst;
	}
};

#ifdef _MSC_VER
#define ALWAYS_INLINE __forceinline
#else
#define ALWAYS_INLINE __attribute__((always_inline))
#endif

template<typename _string_type, size_t _length>
class _Basic_XorStr
{
	using value_type = typename _string_type::value_type;
	static constexpr auto _length_minus_one = _length - 1;

public:
	constexpr ALWAYS_INLINE _Basic_XorStr(value_type const (&str)[_length])
		: _Basic_XorStr(str, std::make_index_sequence<_length_minus_one>())
	{

	}

	inline auto c_str() const
	{
		decrypt();

		return data;
	}

	inline auto str() const
	{
		decrypt();

		return _string_type(data, data + _length_minus_one);
	}

	inline operator _string_type() const
	{
		return str();
	}

private:
	template<size_t... indices>
	constexpr ALWAYS_INLINE _Basic_XorStr(value_type const (&str)[_length], std::index_sequence<indices...>)
		: data{ crypt(str[indices], indices)..., '\0' },
		encrypted(true)
	{

	}

	static constexpr auto XOR_KEY = static_cast<value_type>(
		const_atoi(__TIME__[7]) +
		const_atoi(__TIME__[6]) * 10 +
		const_atoi(__TIME__[4]) * 60 +
		const_atoi(__TIME__[3]) * 600 +
		const_atoi(__TIME__[1]) * 3600 +
		const_atoi(__TIME__[0]) * 36000
		);

	static ALWAYS_INLINE constexpr auto crypt(value_type c, size_t i)
	{
		return static_cast<value_type>(c ^ (XOR_KEY + i));
	}

	inline void decrypt() const
	{
		if (encrypted)
		{
			for (size_t t = 0; t < _length_minus_one; t++)
			{
				data[t] = crypt(data[t], t);
			}
			encrypted = false;
		}
	}

	mutable value_type data[_length];
	mutable bool encrypted;
};


//---------------------------------------------------------------------------
template<size_t _length>
using XorStrA = _Basic_XorStr<std::string, _length>;
template<size_t _length>
using XorStrW = _Basic_XorStr<std::wstring, _length>;
template<size_t _length>
using XorStrU16 = _Basic_XorStr<std::u16string, _length>;
template<size_t _length>
using XorStrU32 = _Basic_XorStr<std::u32string, _length>;
//---------------------------------------------------------------------------
template<typename _string_type, size_t _length, size_t _length2>
inline auto operator==(const _Basic_XorStr<_string_type, _length>& lhs, const _Basic_XorStr<_string_type, _length2>& rhs)
{
	static_assert(_length == _length2, "XorStr== different length");

	return _length == _length2 && lhs.str() == rhs.str();
}
//---------------------------------------------------------------------------
template<typename _string_type, size_t _length>
inline auto operator==(const _string_type& lhs, const _Basic_XorStr<_string_type, _length>& rhs)
{
	return lhs.size() == _length && lhs == rhs.str();
}
//---------------------------------------------------------------------------
template<typename _stream_type, typename _string_type, size_t _length>
inline auto& operator<<(_stream_type& lhs, const _Basic_XorStr<_string_type, _length>& rhs)
{
	lhs << rhs.c_str();

	return lhs;
}
//---------------------------------------------------------------------------
template<typename _string_type, size_t _length, size_t _length2>
inline auto operator+(const _Basic_XorStr<_string_type, _length>& lhs, const _Basic_XorStr<_string_type, _length2>& rhs)
{
	return lhs.str() + rhs.str();
}
//---------------------------------------------------------------------------
template<typename _string_type, size_t _length>
inline auto operator+(const _string_type& lhs, const _Basic_XorStr<_string_type, _length>& rhs)
{
	return lhs + rhs.str();
}
//---------------------------------------------------------------------------
template<size_t _length>
constexpr ALWAYS_INLINE auto XorStr(char const (&str)[_length])
{
	return XorStrA<_length>(str);
}
//---------------------------------------------------------------------------
template<size_t _length>
constexpr ALWAYS_INLINE auto XorStr(wchar_t const (&str)[_length])
{
	return XorStrW<_length>(str);
}
//---------------------------------------------------------------------------
template<size_t _length>
constexpr ALWAYS_INLINE auto XorStr(char16_t const (&str)[_length])
{
	return XorStrU16<_length>(str);
}
//---------------------------------------------------------------------------
template<size_t _length>
constexpr ALWAYS_INLINE auto XorStr(char32_t const (&str)[_length])
{
	return XorStrU32<_length>(str);
}
//---------------------------------------------------------------------------

#define enc(x) (XorStr(x).c_str())
#define xor(x) (XorStr(x))