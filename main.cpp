#include <iostream>
#include "Engine.h"

int main(int argc, char* argv[]) {

	//Initializes the engine state
	Engine::EngineState state = Engine::EngineState::CREATE;
	int result = EXIT_FAILURE;

	while (state != Engine::EngineState::EXIT)
	{
		switch (state)
		{
			// Allocate the engine --------------------------------------------
		case Engine::EngineState::CREATE:
			state = Engine::EngineState::AWAKE;
			break;

			// Awake all modules -----------------------------------------------
		case Engine::EngineState::AWAKE:
			if (Engine::GetInstance().Awake() == true)
				state = Engine::EngineState::START;
			else
			{
				state = Engine::EngineState::FAIL;
			}

			break;

			// Call all modules before first frame  ----------------------------
		case Engine::EngineState::START:
			if (Engine::GetInstance().Start() == true )
			{
				state = Engine::EngineState::LOOP;
			}
			else
			{
				state = Engine::EngineState::FAIL;
			}
			break;

			// Loop all modules until we are asked to leave ---------------------
		case Engine::EngineState::LOOP:
			if (Engine::GetInstance().Update() == false)
				state = Engine::EngineState::CLEAN;
			break;

			// Cleanup allocated memory -----------------------------------------
		case Engine::EngineState::CLEAN:
			if (Engine::GetInstance().CleanUp() == true)
			{
				result = EXIT_SUCCESS;
				state = Engine::EngineState::EXIT;
			}
			else
				state = Engine::EngineState::FAIL;

			break;

			// Exit with errors and shame ---------------------------------------
		case Engine::EngineState::FAIL:
			result = EXIT_FAILURE; 
			state = Engine::EngineState::EXIT;
			break;
		}
	}

	return result;
}