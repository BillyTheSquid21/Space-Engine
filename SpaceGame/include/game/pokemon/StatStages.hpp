#pragma once
#ifndef STAT_STAGES_HPP
#define STAT_STAGES_HPP

#define STAGES_COUNT 14

#include <stdint.h>

static enum class Stage : int8_t
{
	STAGE_N6 = 0,
	STAGE_N5 = 1,
	STAGE_N4 = 2,
	STAGE_N3 = 3,
	STAGE_N2 = 4,
	STAGE_N1 = 5,
	STAGE_0 = 6,
	STAGE_P1 = 7,
	STAGE_P2 = 8,
	STAGE_P3 = 9,
	STAGE_P4 = 10,
	STAGE_P5 = 11,
	STAGE_P6 = 12,
	STAGE_MISS = 13
};

static constexpr float StatStageLookup[STAGES_COUNT]
{
	1.0f / 4.0f, 2.0f / 7.0f, 2.0f / 6.0f, 2.0f / 5.0f, 2.0f / 4.0f, 2.0f / 3.0f,
	1.0f,
	3.0f / 2.0f, 4.0f / 2.0f, 5.0f / 2.0f, 6.0f / 2.0f, 7.0f / 2.0f, 8.0f / 2.0f, 0.0f
};

struct CurrentStages
{
	Stage attackStage = Stage::STAGE_0;
	Stage defenseStage = Stage::STAGE_0;
	Stage spAttackStage = Stage::STAGE_0;
	Stage spDefenseStage = Stage::STAGE_0;
	Stage speedStage = Stage::STAGE_0;
	Stage critStage = Stage::STAGE_0;
	Stage accStage = Stage::STAGE_0;
	Stage evasStage = Stage::STAGE_0;
};

#endif