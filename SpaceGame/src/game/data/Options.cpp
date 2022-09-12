#include "game/data/Options.h"

FMOD::ChannelGroup* Options::effects = nullptr;
FMOD::ChannelGroup* Options::music = nullptr;
int Options::grassDensity = 40;

void Options::load()
{
	using namespace rapidjson;
    std::ifstream ifs("options.json");
    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document doc;
    doc.ParseStream(isw);
    ifs.close();

    //Volumes
    effects->setVolume(doc["effectsVol"].GetFloat());
    music->setVolume(doc["musicVol"].GetFloat());

    //Grass Density
    grassDensity = doc["grassDensity"].GetInt();
}

void Options::write()
{
    using namespace rapidjson;
    std::ifstream ifs("options.json");
    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document doc;
    doc.ParseStream(isw);
    ifs.close();

    float effVol; effects->getVolume(&effVol);
    doc["effectsVol"].SetFloat(effVol);

    float musVol; effects->getVolume(&musVol);
    doc["musicVol"].SetFloat(musVol);

    doc["grassDensity"].SetInt(grassDensity);

    std::ofstream ofs("options.json");
    rapidjson::OStreamWrapper osw(ofs);
    Writer<OStreamWrapper> writer{ osw };
    doc.Accept(writer);
}