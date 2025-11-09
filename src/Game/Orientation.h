#ifndef ORIENTATION_H
#define ORIENTATION_H
#include <cstdint>
#include <string>

enum class eOrientation : uint8_t
{
	Up = 1 << 0,
	Down = 1 << 1,
	Left = 1 << 2,
	Right = 1 << 3
};

inline const char* OrientationToString(const eOrientation e)
{
	switch (e)
	{
	case eOrientation::Up: return "Up";
	case eOrientation::Down: return "Down";
	case eOrientation::Left: return "Left";
	case eOrientation::Right: return "Right";
	default: return "unknown";
	}
}

inline eOrientation StringToOrientation(const std::string& orientation)
{
	if (orientation == "Up")
	{
		return eOrientation::Up;
	}

	if (orientation == "Down")
	{
		return eOrientation::Down;
	}

	if (orientation == "Left")
	{
		return eOrientation::Left;
	}

	if (orientation == "Right")
	{
		return eOrientation::Right;
	}

	return eOrientation::Down;
}

#endif // !ORIENTATION_H
