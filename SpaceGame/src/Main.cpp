#include "game/data/Options.h"
#include "game/DemoGame.h"
#include "core/Run.h"

int main(void)
{   
	//Load options from file
	Options::load();
	return SG::Run<DemoGame>(Options::width, Options::height);
}

