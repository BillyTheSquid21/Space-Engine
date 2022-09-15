#include "game/data/Options.h"

FMOD::ChannelGroup* Options::effects = nullptr;
FMOD::ChannelGroup* Options::music = nullptr;
int Options::grassDensity = 40;
int Options::shadowSamples = 16;

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

    //Shadow samples
    shadowSamples = doc["shadowSamples"].GetInt();
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

    float musVol; music->getVolume(&musVol);
    doc["musicVol"].SetFloat(musVol);

    doc["grassDensity"].SetInt(grassDensity);

    doc["shadowSamples"].SetInt(shadowSamples);

    std::ofstream ofs("options.json");
    rapidjson::OStreamWrapper osw(ofs);
    Writer<OStreamWrapper> writer{ osw };
    doc.Accept(writer);
}