#include <Engine/OS.h>
#include <Sound/Sound.h>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <iostream>
#include <Engine/Log.h>
#include <World/Stats.h>

int Start(int argc, char** argv);

int main(int argc, char** argv)
{
	try
	{
		return Start(argc, argv);

	}
	catch (const std::exception& e)
	{
		OS::SetConsoleWindowVisible(true);

		Sound::StopAllSounds();
		Sound::End();
		system("CLS");
		std::cout << "Thrown uncaught exception at: " << Debugging::EngineStatus << ": \n";
		std::cout << e.what();
		std::cout << "\nPress enter to display engine log\n";
		std::cin.get();
		std::cout << "Engine Log:\n";
		for (Log::Message i : Log::Messages)
		{
			std::cout << "    " <<
				std::string(i.Ammount != 0 ?
					i.Text + std::string(" (x") + std::to_string(i.Ammount + 1) + std::string(")") :
					i.Text) << "\n";
		}
		std::cout << "\nPress enter to continue";
		std::cin.get();
	}
	catch (const char* e)
	{
		OS::SetConsoleWindowVisible(true);
		Sound::StopAllSounds();
		Sound::End();
		system("CLS");
		std::cout << "Unknown error at: " << Debugging::EngineStatus << ": \n";
		std::cout << e;
		std::cout << "\nPress enter to display engine log\n";
		std::cin.get();
		std::cout << "Engine Log:\n";
		for (Log::Message i : Log::Messages)
		{
			std::cout << "    " <<
				std::string(i.Ammount != 0 ?
					i.Text + std::string(" (x") + std::to_string(i.Ammount + 1) + std::string(")") :
					i.Text) << "\n";
		}
		std::cout << "\nPress enter to continue";
		std::cin.get();
	}
}