#include "core/Run.h"
#include "core/Root.h"
#include "utility/Options.h"

int main(void)
{   
	int status;
	
	//Load options 
	SGOptions::LoadOptions();

	//Run program loop
	status = SG::Run<Game>(SGOptions::WIDTH, SGOptions::HEIGHT, SGOptions::WINDOWED);
	
	//If signalled to restart, repeat
	while (SGRoot::CheckShouldRestart())
	{
		SGOptions::LoadOptions();
		status = SG::Run<Game>(SGOptions::WIDTH, SGOptions::HEIGHT, SGOptions::WINDOWED);
	}

	return status;
}

