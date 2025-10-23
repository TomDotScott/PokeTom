#ifndef INPUTMAPPER_H
#define INPUTMAPPER_H
#include <cstdint>
#include <unordered_map>
#include "InputEvents.h"
#include "InputValue.h"

class InputMapper
{
public:
	InputMapper();

	void Update();

	void Map(int inputId, eInputType type, int button);

	bool IsButtonDown(int inputID) const;
	bool IsButtonPressed(int inputID) const;
	bool IsButtonReleased(int inputID) const;

	void OnButtonDown(int inputID, const std::function<void()>& callback);
	void OnButtonPressed(int inputID, const std::function<void()>& callback);
	void OnButtonReleased(int inputID, const std::function<void()>& callback);

	// TODO: Define a set of defaults and read the rest of the values in from an XML file so we can support remapping
	struct GameAction
	{
		GameAction();
		void OnButtonDown(std::function<void()> callback);
		void OnButtonPressed(std::function<void()> callback);
		void OnButtonReleased(std::function<void()> callback);


		InputValue PrimaryInput;
		InputValue SecondaryInput;
		InputEvent m_events = InputEvent();
	};

private:
	std::unordered_map<int, GameAction> m_inputs;
};
#endif
