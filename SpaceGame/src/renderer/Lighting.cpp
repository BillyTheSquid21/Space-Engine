#include "renderer/Lighting.h"

void SGRender::Lighting::set(int width, int height, Camera* camera)
{
	//Clean to avoid spillover
	clean();

	//Create SSBO and upload base lighting
	m_PointLightSSBO.create(1);
	updateLightBuffer();
	EngineLog("Lighting bound to: ", m_PointLightSSBO.bindingPoint());

	m_Camera = camera;
	m_Width = width; m_Height = height;

	//Create Cluster
	buildClusters();

	m_Set = true;
	m_ModifiedLights = false;
}

void SGRender::Lighting::clean()
{
	if (!m_Set)
	{
		return;
	}

	m_Camera = nullptr;
	m_BaseData = BaseLightData();
	m_LightList.clear();
	m_LightIDs.clear();
	m_NextLightID = 0;
	m_LightCount = 0;

	m_Set = false;
}

SGRender::LightID SGRender::Lighting::addLight(const glm::vec3& pos, float brightness, const glm::vec3& color, float radius)
{
	LightID id = m_NextLightID;
	m_LightList.emplace_back(pos, brightness, color, radius);
	m_LightIDs.emplace_back(id, m_LightCount);

	m_LightCount++;
	m_NextLightID++;

	m_ModifiedLights = true;
	return id;
}

bool SGRender::Lighting::removeLight(LightID id)
{
	int32_t light_index = -1;
	int32_t id_index = -1;
	for (int i = 0; i < m_LightIDs.size(); i++)
	{
		if (m_LightIDs[i].id == id)
		{
			light_index = m_LightIDs[i].index;
			m_LightIDs.erase(m_LightIDs.begin() + i);
			id_index = i; 
			break;
		}
	}

	if (light_index == -1)
	{
		return false;
	}

	m_LightList.erase(m_LightList.begin() + light_index);
	
	//Update ids above position
	for (int i = id_index; i < m_LightIDs.size(); i++)
	{
		m_LightIDs[i].index--;
	}

	m_LightCount--;
	m_ModifiedLights = true;
	return true;
}

void SGRender::Lighting::updateLightBuffer()
{
	//Allocate temporary cpu side buffer with all data contiguous
	size_t data_size = sizeof(BaseLightData) + (m_CulledLightList.size() * sizeof(PointLight));
	void* tmp_buffer = malloc(data_size);

	//Then insert elements and upload
	memcpy_s(tmp_buffer, data_size, &m_BaseData, sizeof(BaseLightData));

	if (m_CulledLightList.size() > 0)
	{
		void* point_array = (void*)((size_t)tmp_buffer + sizeof(BaseLightData));
		memcpy_s(point_array, m_CulledLightList.size() * sizeof(PointLight), &m_CulledLightList[0], m_CulledLightList.size() * sizeof(PointLight));
	}

	m_PointLightSSBO.bind();
	m_PointLightSSBO.bufferData(tmp_buffer, data_size);
	m_PointLightSSBO.unbind();

	free(tmp_buffer);
}

void SGRender::Lighting::linkShader(SGRender::Shader& shader)
{
	shader.bindToUniformBlock("SG_Lighting", m_PointLightSSBO.bindingPoint());
}

float SGRender::Lighting::lightRadius(const PointLight& light)
{
	float sizeFactor = light.radius * light.brightness;

	//Get attenuation terms
	float adjustedLin = m_BaseData.pointLinear / sizeFactor;
	float adjustedQua = m_BaseData.pointQuad / sizeFactor;

	//Solve attenuation for cutoff
	//Check discriminant isn't negative
	float disc = (adjustedLin * adjustedLin)
		- 4.0f * (adjustedQua * (m_BaseData.pointConstant - (1.0f / m_BaseData.pointCutoff)));

	if (disc < 0.0)
	{
		EngineLog("Error solving attenuation!");
		return 0.0f;
	}

	//Otherwise solve for largest X
	return (-adjustedLin + sqrt(disc))/(2.0f*adjustedQua);
}

float SGRender::SqDistPointAABB(glm::vec3 point, Cluster c)
{
	float sqDist = 0.0f;

	for (int i = 0; i < 3; i++) {
		// for each axis count any excess distance outside box extents
		float v = point[i];
		if (v < c.minPoint[i]) sqDist += (c.minPoint[i] - v) * (c.minPoint[i] - v);
		if (v > c.maxPoint[i]) sqDist += (v - c.maxPoint[i]) * (v - c.maxPoint[i]);
	}
	return sqDist;
}

bool SGRender::TestSphereAABB(glm::vec3 vs_pos, float radius, Cluster& cluster)
{
	float squaredDistance = SqDistPointAABB(vs_pos, cluster);
	return squaredDistance <= (radius * radius);
}

void SGRender::Lighting::checkClusterLightsOP(int light, const glm::mat4& view)
{
	PointLight& p = m_CulledLightList[light];
	m_Octree->markLightGrid(light, p.position, lightRadius(p), view, m_LightGridList, m_LightGridBuffer, m_LightGridMutex);
}

void SGRender::Lighting::checkLightInFrustum(int light, LightTrackID* idArray, std::atomic<int>& index)
{
	if (m_Camera->inFrustum(m_LightList[light].position, m_LightList[light].radius + lightRadius(m_LightList[light])))
	{
		idArray[index] = m_LightIDs[light];
		index++;
	}
}

void SGRender::Lighting::buildCulledList()
{
	//BUILD CULLED LIST
	std::vector<PointLight> culledLights;
	std::vector<LightTrackID> culledIDs;

	MtLib::ThreadPool* tp = MtLib::ThreadPool::Fetch();
	std::function<void(int, LightTrackID*, std::atomic<int>&)> check = std::bind(&Lighting::checkLightInFrustum, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

	//Reserve the space of max lights
	culledIDs.resize(m_LightList.size());

	std::atomic<int> nextIndex = 0;
	for (int i = 0; i < m_LightList.size(); i++)
	{
		tp->Run(check, i, &culledIDs[0], std::ref(nextIndex));
	}
	tp->Wait();

	//Shrink list down and reset index
	culledIDs.resize(nextIndex);
	nextIndex = 0;

	culledLights.resize(culledIDs.size());
	for (int i = 0; i < culledIDs.size(); i++)
	{
		culledLights[nextIndex] = m_LightList[culledIDs[i].index];
		nextIndex++;
	}

	//Update the buffers
	m_CulledLightList = std::move(culledLights);
	m_CulledLightIDs = std::move(culledIDs);
	updateLightBuffer();
}

void SGRender::Lighting::copyTileLightData(void* lights, int offset, int size)
{
	memcpy_s(&m_TileLightIndexList[offset], sizeof(int32_t) * MAX_LIGHTS_PER_CLUSTER, lights, sizeof(int32_t) * size);
}

void SGRender::Lighting::buildSupportLists()
{
	//BUILD LIGHT GRID AND TILE LIGHT INDEX
	MtLib::ThreadPool* tp = MtLib::ThreadPool::Fetch();
	std::function<void(int, glm::mat4&)> check = std::bind(&Lighting::checkClusterLightsOP, this, std::placeholders::_1, std::placeholders::_2);

	//Now build cluster list after culling
	//Work out what lights belong to each cluster by traversing tree
	glm::mat4 v = m_Camera->getView();
	for (int i = 0; i < m_CulledLightList.size(); i++)
	{
		tp->Run(check, i, v);
	}
	tp->Wait();

	//Work out offsets and total size of tile light index lists
	size_t tile_index_size = 0;
	for (auto& c : m_LightGridList)
	{
		c.offset = tile_index_size;
		tile_index_size += c.size;
	}

	//Resize array then copy everything
	m_TileLightIndexList.resize(tile_index_size);
	std::function<void(void*, int, int)> copy = std::bind(&Lighting::copyTileLightData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	for (int c = 0; c < m_LightGridList.size(); c++)
	{
		void* lights = (void*)&m_LightGridBuffer[c];
		int offset = m_LightGridList[c].offset;
		int size = m_LightGridList[c].size;
		if (size)
		{
			tp->Run(copy, lights, offset, size);
		}
	}
	tp->Wait();

	//Prevent buffering data that is empty anyways
	if (!tile_index_size)
	{
		return;
	}

	//Buffer light grid
	m_LightGrid.bind();
	m_LightGrid.bufferData(&m_LightGridList[0], sizeof(LGridElement) * m_LightGridList.size());
	m_LightGrid.unbind();

	//Clear light grid for next buffering
	for (int i = 0; i < m_LightGridList.size(); i++)
	{
		m_LightGridList[i].offset = 0;
		m_LightGridList[i].size = 0;
	}

	//Buffer tile indices
	m_TileLightIndex.bind();
	m_TileLightIndex.bufferData(&m_TileLightIndexList[0], sizeof(int32_t) * m_TileLightIndexList.size());
	m_TileLightIndex.unbind();
}

void SGRender::Lighting::cullLights()
{
	//If no reason, don't cull
	if (!m_ModifiedLights && !m_Camera->hasMoved())
	{
		return;
	}

	//Builds the frustum culled list on the cpu
	buildCulledList();

	//Ensure there are actually lights before proceeding - saves buffering
	if (m_CulledLightList.size() == 0)
	{
		return;
	}

	//Builds the light grid and tile light index list
	//TODO - make able to choose between hardware accelerated
	buildSupportLists();

	m_ModifiedLights = false; //Signal that modified lights are written
}

//Helper functions from Blog: http://www.aortiz.me/2018/12/21/CG.html#building-a-cluster-grid
//Changes a points coordinate system from screen space to view space
static glm::vec4 screen2View(glm::vec4 screen, glm::vec2 screenDimensions, const glm::mat4& invproj) {
	//Convert to NDC
	glm::vec2 texCoord = glm::vec2(screen) / screenDimensions;

	//Convert to clipSpace - original inverted y but that means I have to invert later with my setup so is pointless
	glm::vec2 clipTexCoord = (2.0f * glm::vec2(texCoord.x, texCoord.y)) - 1.0f;
	glm::vec4 clip = glm::vec4(clipTexCoord.x, clipTexCoord.y, screen.z, screen.w);

	//View space transform
	glm::vec4 view = invproj * clip;

	//Perspective projection
	view = view / view.w;

	return view;
}

//Creates a line segment from the eye to the screen point, then finds its intersection
//With a z oriented plane located at the given distance to the origin
static glm::vec3 lineIntersectionToZPlane(glm::vec3 A, glm::vec3 B, float zDistance) {
	//all clusters planes are aligned in the same z direction
	glm::vec3 normal = glm::vec3(0.0, 0.0, 1.0);
	//getting the line from the eye to the tile
	glm::vec3 ab = B - A;
	//Computing the intersection length for the line and the plane
	float t = (zDistance - glm::dot(normal, A)) / glm::dot(normal, ab);
	//Computing the actual xyz position of the point along the line
	glm::vec3 result = A + t * ab;
	return result;
}

static float GetViewSpaceZ(int z, int zSlices, float n, float f)
{
	return -n * pow(f / n, (float)z / (float)zSlices);
}

SGRender::Cluster SGRender::CreateCluster(const glm::vec2& screenDim, const glm::mat4& invproj, int x, int y, int z, float tileX, float tileY, float sszNear, float sszFar)
{
	//Init cluster with SS Values
	Cluster cluster = {
		glm::vec4(
			(float)x * tileX,
			(float)y * tileY,
			-1.0f,
			1.0f
		), //Min

		glm::vec4(
			(float)(x + 1) * tileX,
			(float)(y + 1) * tileY,
			-1.0f,
			1.0f
		)  //Max
	};

	//Get in view space
	glm::vec3 minPointVS = screen2View(cluster.minPoint, screenDim, invproj);
	glm::vec3 maxPointVS = screen2View(cluster.maxPoint, screenDim, invproj);

	//Finding the 4 intersection points made from each point to the cluster near/far plane
	glm::vec3 minPointNear = lineIntersectionToZPlane({ 0, 0, 0 }, minPointVS, sszNear);
	glm::vec3 minPointFar = lineIntersectionToZPlane({ 0, 0, 0 }, minPointVS, sszFar);
	glm::vec3 maxPointNear = lineIntersectionToZPlane({ 0, 0, 0 }, maxPointVS, sszNear);
	glm::vec3 maxPointFar = lineIntersectionToZPlane({ 0, 0, 0 }, maxPointVS, sszFar);

	glm::vec3 minPointAABB = glm::min(glm::min(minPointNear, minPointFar), glm::min(maxPointNear, maxPointFar));
	glm::vec3 maxPointAABB = glm::max(glm::max(minPointNear, minPointFar), glm::max(maxPointNear, maxPointFar));

	cluster.minPoint = glm::vec4(minPointAABB, 0.0);
	cluster.maxPoint = glm::vec4(maxPointAABB, 0.0);

	return cluster;
}

void SGRender::CreateClusterList(std::vector<Cluster>& clusters, const Camera* camera, const glm::vec2& screenDim, int dim, float tileSizeX, float tileSizeY)
{
	//Reserve vector of clusters (are just AABB)
	int size = dim * dim * dim;
	clusters.resize(size);

	//Cam values
	glm::vec3 eyePos = glm::vec3(0.0f);
	float zNear = camera->nearPlane();
	float zFar = camera->farPlane();
	glm::mat4 invproj = glm::inverse(camera->getProj());

	int clusterIndex = 0;
	for (int z = 0; z < dim; z++)
	{
		//1. Get Z Coordinate in SS (from near to far)
		float sszNear = GetViewSpaceZ(z, dim, zNear, zFar);
		float sszFar = GetViewSpaceZ(z + 1, dim, zNear, zFar);
		for (int y = 0; y < dim; y++)
		{
			for (int x = 0; x < dim; x++)
			{
				clusters[clusterIndex] = CreateCluster(screenDim, invproj, x, y, z, tileSizeX, tileSizeY, sszNear, sszFar);
				clusterIndex++;
			}
		}
	}
}

void SGRender::Lighting::buildClusters()
{
	//Define split of clusters (assume 16x9x24 here for now)
	const int clusterDim = 16;

	//Get screen dimensions as vector
	glm::vec2 screenDim = glm::vec2(m_Width, m_Height);
	float tileSizeX = screenDim.x / (float)clusterDim;
	float tileSizeY = screenDim.y / (float)clusterDim;

	//Create Primary Cluster List
	std::vector<Cluster> clusters;
	CreateClusterList(clusters, m_Camera, screenDim, clusterDim, tileSizeX, tileSizeY);

	//Create Octree
	m_Octree = std::shared_ptr<ClusterOctree>(new ClusterOctree(m_Camera, screenDim, clusterDim, tileSizeX, tileSizeY));
	int sizeAtDepth = m_Octree->getBottomCluster().size();

	//Create Uniform buffer
	m_ClusterInfo.create();
	m_ClusterInfo.bind();
	m_ClusterInfo.resize((8 * sizeof(int32_t)));
	m_ClusterInfo.bufferData((void*)&clusterDim,0 * sizeof(int32_t), sizeof(int32_t));
	m_ClusterInfo.bufferData((void*)&tileSizeX, 1 * sizeof(int32_t), sizeof(int32_t));
	m_ClusterInfo.bufferData((void*)&tileSizeY, 2 * sizeof(int32_t), sizeof(int32_t));
	m_ClusterInfo.unbind();

	//Initialise light grid and bind to SSBO
	m_LightGridList.resize(sizeAtDepth);
	m_LightGrid.create(1);
	m_LightGrid.bind();
	m_LightGrid.bufferData(&m_LightGridList[0], sizeof(LGridElement) * m_LightGridList.size());
	m_LightGrid.unbind();

	//Init tile light index list
	m_TileLightIndex.create(1);

	//Initialize light grid CPU Buffer
	m_LightGridBuffer.resize(sizeAtDepth);

	//Init light grid mutexes
	std::vector<std::shared_mutex> tmpmut(sizeAtDepth);
	m_LightGridMutex.swap(tmpmut);

	EngineLog("Clusters bound to: ", m_ClusterInfo.bindingPoint());
	EngineLog("Light Grid bound to: ", m_LightGrid.bindingPoint());
	EngineLog("Tile Light Index bound to: ", m_TileLightIndex.bindingPoint());
}