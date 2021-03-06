#include "pch.h"
#include "MouseInput.h"

using namespace HologramJS::Input;
using namespace Windows::UI::Core;
using namespace std;
using namespace Windows::Foundation;

enum class MouseButtons : int
{
	NoButton = 0,
	LeftButton = 1,
	RightButton = 2,
	MiddleButton = 4
};

inline MouseButtons &
operator|=(MouseButtons & __x, MouseButtons __y)
{
	__x = static_cast<MouseButtons>(static_cast<int>(__x) | static_cast<int>(__y));
	return __x;
}

MouseInput::MouseInput()
{
	m_mouseDownToken = CoreWindow::GetForCurrentThread()->PointerPressed += ref new TypedEventHandler<CoreWindow ^, PointerEventArgs ^>(
		[this](CoreWindow ^sender, PointerEventArgs ^args)
	{
		this->CallbackScriptForMouseInput(MouseInputEventType::MouseDown, args);
	});

	m_mouseUpToken = CoreWindow::GetForCurrentThread()->PointerReleased += ref new TypedEventHandler<CoreWindow ^, PointerEventArgs ^>(
		[this](CoreWindow ^sender, PointerEventArgs ^args)
	{
		this->CallbackScriptForMouseInput(MouseInputEventType::MouseUp, args);
	});

	m_mouseWheelToken = CoreWindow::GetForCurrentThread()->PointerWheelChanged += ref new TypedEventHandler<CoreWindow ^, PointerEventArgs ^>(
		[this](CoreWindow ^sender, PointerEventArgs ^args)
	{
		this->CallbackScriptForMouseInput(MouseInputEventType::MouseWheel, args);
	});

	m_mouseMoveToken = CoreWindow::GetForCurrentThread()->PointerMoved += ref new TypedEventHandler<CoreWindow ^, PointerEventArgs ^>(
		[this](CoreWindow ^sender, PointerEventArgs ^args)
	{
		this->CallbackScriptForMouseInput(MouseInputEventType::MouseMove, args);
	});
}


MouseInput::~MouseInput()
{
	CoreWindow::GetForCurrentThread()->PointerPressed -= m_mouseDownToken;
	CoreWindow::GetForCurrentThread()->PointerReleased -= m_mouseUpToken;
	CoreWindow::GetForCurrentThread()->PointerWheelChanged -= m_mouseWheelToken;
	CoreWindow::GetForCurrentThread()->PointerMoved -= m_mouseMoveToken;
}

MouseButtons GetButtonFromArgs(PointerEventArgs^ args)
{
	auto returnValue = MouseButtons::NoButton;

	const auto pointProps = args->CurrentPoint->Properties;

	if (pointProps->IsLeftButtonPressed)
	{
		returnValue |= MouseButtons::LeftButton;
	}

	if (pointProps->IsMiddleButtonPressed)
	{
		returnValue |= MouseButtons::MiddleButton;
	}

	if (pointProps->IsRightButtonPressed)
	{
		returnValue |= MouseButtons::RightButton;
	}

	return returnValue;
}

void
MouseInput::CallbackScriptForMouseInput(
	MouseInputEventType type,
	Windows::UI::Core::PointerEventArgs^ args
)
{
	JsValueRef parameters[6];
	parameters[0] = m_scriptCallback;
	JsValueRef* eventTypeParam = &parameters[1];
	JsValueRef* xParam = &parameters[2];
	JsValueRef* yParam = &parameters[3];
	JsValueRef* buttonParam = &parameters[4];
	JsValueRef* actionParam = &parameters[5];

	EXIT_IF_JS_ERROR(JsIntToNumber(static_cast<int>(NativeToScriptInputType::Mouse), eventTypeParam));
	EXIT_IF_JS_ERROR(JsDoubleToNumber(args->CurrentPoint->Position.X, xParam));
	EXIT_IF_JS_ERROR(JsDoubleToNumber(args->CurrentPoint->Position.Y, yParam));
	EXIT_IF_JS_ERROR(JsIntToNumber(static_cast<int>(GetButtonFromArgs(args)), buttonParam));
	EXIT_IF_JS_ERROR(JsIntToNumber(static_cast<int>(type), actionParam));

	JsValueRef result;
	EXIT_IF_JS_ERROR(JsCallFunction(m_scriptCallback, parameters, ARRAYSIZE(parameters), &result));
}
