#include "utility/Options.h"

int SGOptions::WIDTH = 1280;
const char WIDTH_NAME[] = "width";
int SGOptions::HEIGHT = 720;
const char HEIGHT_NAME[] = "height";
bool SGOptions::VSYNC_ENABLED = false;
const char VSYNC_NAME[] = "vsync";
bool SGOptions::WINDOWED = true;
const char WINDOWED_NAME[] = "windowed";

const char OPTIONS_PATH[] = "options.json";

bool SGOptions::SaveOptions()
{
	using namespace rapidjson;
	Document doc;
	doc.SetObject();

	//Set all the options here
	doc.AddMember(WIDTH_NAME, WIDTH, doc.GetAllocator());
	doc.AddMember(HEIGHT_NAME, HEIGHT, doc.GetAllocator());
	doc.AddMember(VSYNC_NAME, VSYNC_ENABLED, doc.GetAllocator());
	doc.AddMember(WINDOWED_NAME, WINDOWED, doc.GetAllocator());

	//Serialize
	EngineLog("Writing options...");
	StringBuffer buffer{};
	Writer<StringBuffer> writer{ buffer };
	doc.Accept(writer);

	//Check has parsed correctly
	if (doc.HasParseError())
	{
		EngineLogFail("Options Write");
		EngineLog("Error:", doc.GetParseError());
		EngineLog("Offset:", doc.GetErrorOffset());
		return false;
	}

	const std::string jsonStr{ buffer.GetString() };
	std::ofstream ofs{ OPTIONS_PATH };

	if (!ofs.is_open())
	{
		EngineLogFail("OfStream failed to open!");
		return false;
	}

	OStreamWrapper osw{ ofs };
	Writer<OStreamWrapper> writer2{ osw };
	doc.Accept(writer2);

	EngineLogOk("Options Write");
	return true;
}

bool SGOptions::LoadOptions()
{
	using namespace rapidjson;

	//Check file exists, if not, generate from default values
	if (!std::filesystem::exists(OPTIONS_PATH))
	{
		EngineLog("Generating options file...");
		SaveOptions();
		return true;
	}
	EngineLog("Loading options file...");

	//Load the file and write to global values
	std::ifstream ifs{ OPTIONS_PATH };
	if (!ifs.is_open())
	{
		EngineLogFail("Options Load");
		return false;
	}

	IStreamWrapper isw{ ifs };
	Document doc{};
	doc.ParseStream(isw);
	
	//Assert the doc is a valid options file to read from
	bool valid = true;
	valid &= doc.HasMember(WIDTH_NAME);
	valid &= doc.HasMember(HEIGHT_NAME);
	valid &= doc.HasMember(VSYNC_NAME);
	valid &= doc.HasMember(WINDOWED_NAME);

	if (!valid)
	{
		//Overwrite file with the correct one
		SaveOptions();
		return true;
	}

	//Load
	WIDTH = doc.FindMember(WIDTH_NAME)->value.GetInt();
	HEIGHT = doc.FindMember(HEIGHT_NAME)->value.GetInt();
	VSYNC_ENABLED = doc.FindMember(VSYNC_NAME)->value.GetBool();
	WINDOWED = doc.FindMember(WINDOWED_NAME)->value.GetBool();

	EngineLogOk("Options Load");
	return true;
}