#include "renderer/Vertex.h"

int SGRender::PropertyStride(VProperties properties)
{
	switch (properties)
	{
	case SGRender::V_HAS_UVS:
		return 2;
	case SGRender::V_HAS_NORMALS:
		return 3;
	case SGRender::V_HAS_COLOR:
		return 4;
	case SGRender::V_HAS_TANGENTS:
		return 3;
	default:
		return -1;
	}
}

int SGRender::VertexStride(int vertex)
{
	switch (vertex)
	{
	case SGRender::V_Vertex:
		return 3;
	case SGRender::V_CVertex:
		return 7;
	case SGRender::V_UVertex:
		return 5;
	case SGRender::V_NVertex:
		return 6;
	case SGRender::V_UCVertex:
		return 9;
	case SGRender::V_UNVertex:
		return 8;
	case SGRender::V_NTVertex:
		return 9;
	case SGRender::V_UNTVertex:
		return 11;
	default:
		return -1;
	}
}

int SGRender::VertexColorOffset(int vertex)
{
	switch (vertex)
	{
	case SGRender::V_CVertex:
		return 3;
	case SGRender::V_UCVertex:
		return 5;
	default:
		return -1;
	}
}

static int SGRender::vertexUVOffset(int vertex)
{
	switch (vertex)
	{
	case SGRender::V_UVertex:
		return 3;
	case SGRender::V_UCVertex:
		return 3;
	case SGRender::V_UNVertex:
		return 3;
	case SGRender::V_UNTVertex:
		return 3;
	default:
		return -1;
	}
}

int SGRender::VertexNormalOffset(int vertex)
{
	switch (vertex)
	{
	case SGRender::V_NVertex:
		return 3;
	case SGRender::V_UNVertex:
		return 5;
	case SGRender::V_NTVertex:
		return 3;
	case SGRender::V_UNTVertex:
		return 5;
	default:
		return -1;
	}
}

int SGRender::VertexTangentOffset(int vertex)
{
	switch (vertex)
	{
	case SGRender::V_NTVertex:
		return 6;
	case SGRender::V_UNTVertex:
		return 8;
	default:
		return -1;
	}
}