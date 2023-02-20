#include "core/Run.h"
#include "core/Root.h"
#include "utility/Options.h"
#include "example/ExampleScene.h"

int main(void)
{   
	int status;
	
	//Load options 
	SGOptions::LoadOptions();

	//Run program loop
	status = SG::Run<ExampleScene>(SGOptions::WIDTH, SGOptions::HEIGHT, SGOptions::WINDOWED);
	
	//If signalled to restart, repeat
	while (SGRoot::CheckShouldRestart())
	{
		SGOptions::LoadOptions();
		status = SG::Run<ExampleScene>(SGOptions::WIDTH, SGOptions::HEIGHT, SGOptions::WINDOWED);
	}

	return status;
}

