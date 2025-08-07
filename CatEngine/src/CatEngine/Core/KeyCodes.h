#pragma once


namespace CatEngine
{

	enum class KeyCode : uint16_t
	{
		// From glfw3.h
		Space              = 32,
		Apostrophe         = 39,  /* ' */
		Comma              = 44,  /* , */
		Minus              = 45,  /* - */
		Period             = 46,  /* . */
		Slash              = 47,  /* / */
		D0                 = 48,
		D1                 = 49,
		D2                 = 50,
		D3                 = 51,
		D4                 = 52,
		D5                 = 53,
		D6                 = 54,
		D7                 = 55,
		D8                 = 56,
		D9                 = 57,
		SEMICOLON          = 59,  /* ; */
		EQUAL              = 61,  /* = */
		A                  = 65,
		B                  = 66,
		C                  = 67,
		D                  = 68,
		E                  = 69,
		F                  = 70,
		G                  = 71,
		H                  = 72,
		I                  = 73,
		J                  = 74,
		K                  = 75,
		L                  = 76,
		M                  = 77,
		N                  = 78,
		O                  = 79,
		P                  = 80,
		Q                  = 81,
		R                  = 82,
		S                  = 83,
		T                  = 84,
		U                  = 85,
		V                  = 86,
		W                  = 87,
		X                  = 88,
		Y                  = 89,
		Z                  = 90,
		LeftBracket       = 91,  /* [ */
		Backslash          = 92,  /* \ */
		RightBracket      = 93,  /* ] */
		GraceAccent       = 96,  /* ` */
		World1            = 161, /* non-US #1 */
		World2            = 162, /* non-US #2 */

/* Function keys */
		Escape             = 256,
		Enter              = 257,
		Tab                = 258,
		Backspace          = 259,
		Insert             = 260,
		Delete             = 261,
		Right              = 262,
		Left               = 263,
		Down               = 264,
		Up                 = 265,
		PageUp            = 266,
		PageDown          = 267,
		Home               = 268,
		End                = 269,
		CapsLock          = 280,
		ScrollLock        = 281,
		NumLock           = 282,
		PrintScreen       = 283,
		Pause              = 284,
		F1                 = 290,
		F2                 = 291,
		F3                 = 292,
		F4                 = 293,
		F5                 = 294,
		F6                 = 295,
		F7                 = 296,
		F8                 = 297,
		F9                 = 298,
		F10                = 299,
		F11                = 300,
		F12                = 301,
		F13                = 302,
		F14                = 303,
		F15                = 304,
		F16                = 305,
		F17                = 306,
		F18                = 307,
		F19                = 308,
		F20                = 309,
		F21                = 310,
		F22                = 311,
		F23                = 312,
		F24                = 313,
		F25                = 314,
		KP_0               = 320,
		KP_1               = 321,
		KP_2               = 322,
		KP_3               = 323,
		KP_4               = 324,
		KP_5               = 325,
		KP_6               = 326,
		KP_7               = 327,
		KP_8               = 328,
		KP_9               = 329,
		KPDecimal          = 330,
		KPDivide           = 331,
		KPMultiply         = 332,
		KPSubtract         = 333,
		KPAdd              = 334,
		KPEnter            = 335,
		KPEqual            = 336,
		LeftShift         = 340,
		LeftControl        = 341,
		LeftAlt            = 342,
		LeftSuper          = 343,
		RightShift        = 344,
		RightControl      = 345,
		RightAlt          = 346,
		RightSuper        = 347,
		Menu               = 348,

		ModShift           = 0x0001,
		ModControl         = 0x0002,
		ModAlt             = 0x0004,
		ModSuper           = 0x0008,
		ModCapsLock       = 0x0010,
		ModNumLock        = 0x0020,

	};

	inline std::ostream& operator<<(std::ostream& os, KeyCode keyCode)
	{
		os << static_cast<uint32_t>(keyCode);
		return os;
	}
}
// From glfw3.h
#define CE_SPACE           ::CatEngine::Key::Space
#define CE_APOSTROPHE      ::CatEngine::Key::Apostrophe    /* ' */
#define CE_COMMA           ::CatEngine::Key::Comma         /* , */
#define CE_MINUS           ::CatEngine::Key::Minus         /* - */
#define CE_PERIOD          ::CatEngine::Key::Period        /* . */
#define CE_SLASH           ::CatEngine::Key::Slash         /* / */
#define CE_0               ::CatEngine::Key::D0
#define CE_1               ::CatEngine::Key::D1
#define CE_2               ::CatEngine::Key::D2
#define CE_3               ::CatEngine::Key::D3
#define CE_4               ::CatEngine::Key::D4
#define CE_5               ::CatEngine::Key::D5
#define CE_6               ::CatEngine::Key::D6
#define CE_7               ::CatEngine::Key::D7
#define CE_8               ::CatEngine::Key::D8
#define CE_9               ::CatEngine::Key::D9
#define CE_SEMICOLON       ::CatEngine::Key::Semicolon     /* ; */
#define CE_EQUAL           ::CatEngine::Key::Equal         /* = */
#define CE_A               ::CatEngine::Key::A
#define CE_B               ::CatEngine::Key::B
#define CE_C               ::CatEngine::Key::C
#define CE_D               ::CatEngine::Key::D
#define CE_E               ::CatEngine::Key::E
#define CE_F               ::CatEngine::Key::F
#define CE_G               ::CatEngine::Key::G
#define CE_H               ::CatEngine::Key::H
#define CE_I               ::CatEngine::Key::I
#define CE_J               ::CatEngine::Key::J
#define CE_K               ::CatEngine::Key::K
#define CE_L               ::CatEngine::Key::L
#define CE_M               ::CatEngine::Key::M
#define CE_N               ::CatEngine::Key::N
#define CE_O               ::CatEngine::Key::O
#define CE_P               ::CatEngine::Key::P
#define CE_Q               ::CatEngine::Key::Q
#define CE_R               ::CatEngine::Key::R
#define CE_S               ::CatEngine::Key::S
#define CE_T               ::CatEngine::Key::T
#define CE_U               ::CatEngine::Key::U
#define CE_V               ::CatEngine::Key::V
#define CE_W               ::CatEngine::Key::W
#define CE_X               ::CatEngine::Key::X
#define CE_Y               ::CatEngine::Key::Y
#define CE_Z               ::CatEngine::Key::Z
#define CE_LEFT_BRACKET    ::CatEngine::Key::LeftBracket   /* [ */
#define CE_BACKSLASH       ::CatEngine::Key::Backslash     /* \ */
#define CE_RIGHT_BRACKET   ::CatEngine::Key::RightBracket  /* ] */
#define CE_GRAVE_ACCENT    ::CatEngine::Key::GraveAccent   /* ` */
#define CE_WORLD_1         ::CatEngine::Key::World1        /* non-US #1 */
#define CE_WORLD_2         ::CatEngine::Key::World2        /* non-US #2 */

/* Function keys */
#define CE_ESCAPE          ::CatEngine::Key::Escape
#define CE_ENTER           ::CatEngine::Key::Enter
#define CE_TAB             ::CatEngine::Key::Tab
#define CE_BACKSPACE       ::CatEngine::Key::Backspace
#define CE_INSERT          ::CatEngine::Key::Insert
#define CE_DELETE          ::CatEngine::Key::Delete
#define CE_RIGHT           ::CatEngine::Key::Right
#define CE_LEFT            ::CatEngine::Key::Left
#define CE_DOWN            ::CatEngine::Key::Down
#define CE_UP              ::CatEngine::Key::Up
#define CE_PAGE_UP         ::CatEngine::Key::PageUp
#define CE_PAGE_DOWN       ::CatEngine::Key::PageDown
#define CE_HOME            ::CatEngine::Key::Home
#define CE_END             ::CatEngine::Key::End
#define CE_CAPS_LOCK       ::CatEngine::Key::CapsLock
#define CE_SCROLL_LOCK     ::CatEngine::Key::ScrollLock
#define CE_NUM_LOCK        ::CatEngine::Key::NumLock
#define CE_PRINT_SCREEN    ::CatEngine::Key::PrintScreen
#define CE_PAUSE           ::CatEngine::Key::Pause
#define CE_F1              ::CatEngine::Key::F1
#define CE_F2              ::CatEngine::Key::F2
#define CE_F3              ::CatEngine::Key::F3
#define CE_F4              ::CatEngine::Key::F4
#define CE_F5              ::CatEngine::Key::F5
#define CE_F6              ::CatEngine::Key::F6
#define CE_F7              ::CatEngine::Key::F7
#define CE_F8              ::CatEngine::Key::F8
#define CE_F9              ::CatEngine::Key::F9
#define CE_F10             ::CatEngine::Key::F10
#define CE_F11             ::CatEngine::Key::F11
#define CE_F12             ::CatEngine::Key::F12
#define CE_F13             ::CatEngine::Key::F13
#define CE_F14             ::CatEngine::Key::F14
#define CE_F15             ::CatEngine::Key::F15
#define CE_F16             ::CatEngine::Key::F16
#define CE_F17             ::CatEngine::Key::F17
#define CE_F18             ::CatEngine::Key::F18
#define CE_F19             ::CatEngine::Key::F19
#define CE_F20             ::CatEngine::Key::F20
#define CE_F21             ::CatEngine::Key::F21
#define CE_F22             ::CatEngine::Key::F22
#define CE_F23             ::CatEngine::Key::F23
#define CE_F24             ::CatEngine::Key::F24
#define CE_F25             ::CatEngine::Key::F25

/* Keypad */
#define CE_KP_0            ::CatEngine::Key::KP0
#define CE_KP_1            ::CatEngine::Key::KP1
#define CE_KP_2            ::CatEngine::Key::KP2
#define CE_KP_3            ::CatEngine::Key::KP3
#define CE_KP_4            ::CatEngine::Key::KP4
#define CE_KP_5            ::CatEngine::Key::KP5
#define CE_KP_6            ::CatEngine::Key::KP6
#define CE_KP_7            ::CatEngine::Key::KP7
#define CE_KP_8            ::CatEngine::Key::KP8
#define CE_KP_9            ::CatEngine::Key::KP9
#define CE_KP_DECIMAL      ::CatEngine::Key::KPDecimal
#define CE_KP_DIVIDE       ::CatEngine::Key::KPDivide
#define CE_KP_MULTIPLY     ::CatEngine::Key::KPMultiply
#define CE_KP_SUBTRACT     ::CatEngine::Key::KPSubtract
#define CE_KP_ADD          ::CatEngine::Key::KPAdd
#define CE_KP_ENTER        ::CatEngine::Key::KPEnter
#define CE_KP_EQUAL        ::CatEngine::Key::KPEqual

#define CE_LEFT_SHIFT      ::CatEngine::Key::LeftShift
#define CE_LEFT_CONTROL    ::CatEngine::Key::LeftControl
#define CE_LEFT_ALT        ::CatEngine::Key::LeftAlt
#define CE_LEFT_SUPER      ::CatEngine::Key::LeftSuper
#define CE_RIGHT_SHIFT     ::CatEngine::Key::RightShift
#define CE_RIGHT_CONTROL   ::CatEngine::Key::RightControl
#define CE_RIGHT_ALT       ::CatEngine::Key::RightAlt
#define CE_RIGHT_SUPER     ::CatEngine::Key::RightSuper
#define CE_MENU            ::CatEngine::Key::Menu
