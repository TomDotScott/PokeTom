#ifndef INPUTVALUE_H
#define INPUTVALUE_H
#include <cstdint>

enum class eInputType : int8_t
{
	NONE = -1,
	Keyboard,
	Mouse,
	// TODO...
	Controller
};

#define UNASSIGNED_BUTTON (-1)

struct InputValue
{
	InputValue(const eInputType type, const int button) :
		m_value(0),
		m_previousValue(0),
		m_type(type),
		m_button(button)
	{
	}

	inline uint8_t GetValue() const { return m_value; }

	// True the first frame the button is pressed
	inline bool IsPressed() const { return m_value && !m_previousValue; }

	// True whilst the button is pressed
	inline bool IsDown() const { return GetValue(); }

	// True the frame the button is released
	inline bool IsReleased() const { return m_previousValue && !m_value; }

	inline eInputType GetType() const { return m_type; }

	inline int GetButton() const { return m_button; }

	uint8_t m_value;
	uint8_t m_previousValue;

private:
	eInputType m_type;
	int m_button;
};



#endif
