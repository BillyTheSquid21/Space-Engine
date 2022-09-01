#pragma once
#ifndef SG_RANDOM_H
#define SG_RANDOM_H

#include "random"
#include "vector"
#include "cmath"
#include "glm/vec2.hpp"
#include "glm/glm.hpp"
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include "utility/SGUtil.h"

namespace SGRandom
{
	//Container for basic random
	class RandomContainer
	{
	public:
		void seed(float a, float b) { std::random_device rd;  mt.seed(rd()); dist = std::uniform_real_distribution<float>::uniform_real_distribution(a, b); m_Seeded = true; }
		float next() { return dist(mt); }
		bool isSeeded() const { return m_Seeded; }
	private:
		bool m_Seeded = false;
		std::mt19937 mt;
		std::uniform_real_distribution<float> dist;
	};

	//Perlin noise implementation
	//+y
	//|
	//|
	//|
	//|___________+x

	template
	<
		typename T,					//Type the data is stored in memory as
		size_t dataDimension,		//How many data points per coord
		unsigned int resolution,	//Resolution of the data
		size_t size					//Dimension of the map
	>
	class Perlin2D
	{
	public:
		Perlin2D() = default;

		template<typename... Args>
		Perlin2D(bool shareRandom, Args && ... valueRanges)
		{
			static_assert(dataDimension > 0, "Must have at least 1 data dimension");
			static_assert(size > 3, "Must be at least size 3");

			m_Resolution = resolution;
			m_ShareRandom = shareRandom;

			//Work out total elements
			m_Size = size;
			m_Data.resize(m_Size);

			//If different fields do not share random, store more fields
			if (!m_ShareRandom)
			{
				m_NoiseField.resize(m_Resolution * dataDimension);
			}
			else
			{
				m_NoiseField.resize(m_Resolution);
			}

			//Seed randoms
			std::vector<T> range{ { valueRanges... } };
			m_Range = range;
			
			//Check correct amount of range arguments
			int rangeArguments = range.size();
			int rangeExpected = dataDimension * 2;
			assert(rangeArguments == rangeExpected || "Ranges given do not match data dimensions!");
			
			//Seed random
			random.seed(-1.0f, 1.0f);
		}

		T& at(int x, int y, int data)
		{
			bool invalidIndex = (x >= m_Size || x < 0 || y >= m_Size || y < 0);
			assert(!invalidIndex);
			assert(data < dataDimension);

			return m_Data[y].line[x].data[data];
		}

		void generateField(int data, float noiseReduction)
		{
			//Find corners and dot product
			for (int y = 0; y < m_Size; y++)
			{
				for (int x = 0; x < m_Size; x++)
				{
					//Get the coordinates in grid space
					glm::vec2 pos;
					pos.x = ((float)x / (float)m_Size) * (float)m_Resolution;
					pos.y = ((float)y / (float)m_Size) * (float)m_Resolution;

					//Displace slightly
					pos.x += random.next() / noiseReduction;
					pos.y += random.next() / noiseReduction;

					//Get bottom left corner
					glm::vec2 corners[4];
					corners[0].x = floor(pos.x); corners[0].y = floor(pos.y);
					
					//Get other corners
					corners[1] = corners[0]; corners[1].x++;
					corners[2] = corners[0]; corners[2].y++;
					corners[3] = corners[2]; corners[3].x++;

					//Get displacements
					glm::vec2 displace[4];
					displace[0] = pos - corners[0]; displace[1] = pos - corners[1];
					displace[2] = pos - corners[2]; displace[3] = pos - corners[3];

					//Get the dot products
					float dots[4];

					//Set dot products and check to ensure validity
					for (int i = 0; i < 4; i++)
					{
						if (pos == corners[i])
						{
							dots[i] = 0.0f;
						}
						else if (corners[i].x < 0 || corners[i].x >= m_Resolution || corners[i].y < 0 || corners[i].y >= m_Resolution)
						{
							dots[i] = 0.0f;
						}
						else
						{
							if (m_ShareRandom)
							{
								dots[i] = glm::dot(displace[i], m_NoiseField[(int)corners[i].y].line[(int)corners[i].x].data);
							}
							else
							{
								int yIndex = (data * m_Resolution) + (int)corners[i].y;
								dots[i] = glm::dot(displace[i], m_NoiseField[yIndex].line[(int)corners[i].x].data);
							}
						}
					}

					//Lerp dot product for point from the corners
					float lX1 = std::lerp(dots[0], dots[1], pos.x - corners[0].x);
					float lX2 = std::lerp(dots[2], dots[3], pos.x - corners[0].x);

					float value = std::lerp(lX1, lX2, pos.y - corners[0].y);
					value = glm::smoothstep(-1.0f, 1.0f, value);

					int rangeIndex = data * 2;
					float fullRange = m_Range[rangeIndex+1] - m_Range[rangeIndex];
					value = ((value + 1.0f) * (fullRange/2.0f)) + m_Range[rangeIndex];

					m_Data[y].line[x].data[data] = (uint8_t)value;
				}
			}
		}

		bool scrollNoise(int deltaX, int deltaY, float noiseReduction)
		{
			for (int i = 0; i < ((dataDimension - 1) * !m_ShareRandom) + 1; i++)
			{
				if (deltaY > 0)
				{
					//Move noise up
					memmove(&m_NoiseField[i * m_Resolution], &m_NoiseField[(i * m_Resolution) + 1], (m_Resolution - 1) * sizeof(RandomLine));

					//Add new noise below
					for (int x = 0; x < m_Resolution; x++)
					{
						m_NoiseField[(i * m_Resolution) + (m_Resolution - 1)].line[x].data.x = random.next();
						m_NoiseField[(i * m_Resolution) + (m_Resolution - 1)].line[x].data.y = random.next();
					}
				}
				else if (deltaY < 0)
				{
					//Move noise up
					memmove(&m_NoiseField[(i * dataDimension) + 1], &m_NoiseField[i * dataDimension], (m_Resolution - 1) * sizeof(RandomLine));

					//Add new noise above
					for (int x = 0; x < m_Resolution; x++)
					{
						m_NoiseField[i * dataDimension].line[x].data.x = random.next();
						m_NoiseField[i * dataDimension].line[x].data.y = random.next();
					}
				}
				if (deltaX > 0)
				{
					for (int y = i*m_Resolution; y < i * m_Resolution + m_Resolution; y++)
					{
						RandomUnit* line = m_NoiseField[y].line;
						memmove(&line[1], &line[0], (m_Resolution - 1) * sizeof(RandomUnit));
						line[0].data.x = random.next();
						line[0].data.y = random.next();
					}
				}
				else if (deltaX < 0)
				{
					for (int y = i * m_Resolution; y < i * m_Resolution + m_Resolution; y++)
					{
						RandomUnit* line = m_NoiseField[y].line;
						memmove(&line[0], &line[1], (m_Resolution - 1) * sizeof(RandomUnit));
						line[m_Resolution-1].data.x = random.next();
						line[m_Resolution-1].data.y = random.next();
					}
				}
			}

			for (int i = 0; i < dataDimension; i++)
			{
				generateField(i, noiseReduction);
			}
			return true;
		}

		void randomize(float noiseReduction)
		{
			if (m_ShareRandom)
			{
				randomizeGradients();
			}
			for (int i = 0; i < dataDimension; i++)
			{
				if (!m_ShareRandom)
				{
					randomizeGradients(i);
				}
				generateField(i, noiseReduction);
			}
		}

		void randomizeGradients()
		{
			//Seed array
			for (int y = 0; y < m_Resolution; y++)
			{
				for (int x = 0; x < m_Resolution; x++)
				{
					RandomUnit& randomUnit = m_NoiseField[y].line[x];
					//Create 2D vector and normalise
					randomUnit.data.x = random.next();
					randomUnit.data.y = random.next();
				}
			}
		}

		void randomizeGradients(int data)
		{
			//Seed array
			for (int y = data*m_Resolution; y < data * m_Resolution + m_Resolution; y++)
			{
				for (int x = 0; x < m_Resolution; x++)
				{
					RandomUnit& randomUnit = m_NoiseField[y].line[x];
					//Create 2D vector and normalise
					randomUnit.data.x = random.next();
					randomUnit.data.y = random.next();
				}
			}
		}

		void* data()
		{
			return &m_Data[0];
		}

		size_t dataSize()
		{
			return m_Data.size();
		}

		size_t dataSizeBytes()
		{
			return m_Data.size() * sizeof(DataLine);
		}

		void writeAsPng(const char* name)
		{
			stbi_write_png(name, m_Size, m_Size, dataDimension, &m_Data[0], m_Size * dataDimension);
		}

		void clear()
		{
			m_Data.clear();
			m_NoiseField.clear();
			m_Range.clear();
		}

	private:

		struct DataUnit
		{
			T data[dataDimension] = {};
		};

		struct DataLine
		{
			DataUnit line[size] = {};
		};

		struct RandomUnit
		{
			glm::vec2 data;
		};

		struct RandomLine
		{
			RandomUnit line[resolution] = {};
		};

		RandomContainer random;
		
		//Data
		std::vector<DataLine> m_Data;
		std::vector<RandomLine> m_NoiseField;
		std::vector<T> m_Range;
		size_t m_Size;
		bool m_ShareRandom = true;

		//Constants
		unsigned int m_Resolution;
	};
}

#endif