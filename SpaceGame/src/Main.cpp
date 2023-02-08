#include "core/Run.h"
#include "core/Root.h"
#include "utility/Options.h"

int main(void)
{   
	int p;
	do
	{
		//Load options 
		SGOptions::LoadOptions();

		//Run program loop
		p = SG::Run<Game>(SGOptions::WIDTH, SGOptions::HEIGHT, SGOptions::WINDOWED);
	} 
	while (SGRoot::CheckShouldRestart());

	return p;
}

