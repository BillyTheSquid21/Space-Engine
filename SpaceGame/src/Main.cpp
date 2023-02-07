#include "core/Run.h"
#include "core/Root.h"
#include "utility/Options.h"

int main(void)
{   
	int p = 0;
	
	//Run loop to be able to be restarted
	do
	{
		SGOptions::LoadOptions();
		p = SG::Run<Game>(1280, 720, false);
	} 
	while (SGRoot::CheckShouldRestart());

	return p;
}

