#pragma once
#include "glm/glm.hpp"
#include <vector>
#include <cstring>
#include "RenderingHelperFunctions.h"

namespace µTec
{

enum ImageFormat {RGB=3,RGBA=4};

struct DynamicImage
{
	std::vector<unsigned char> rawData;
	ImageFormat format;
	unsigned width;
	unsigned height;
	unsigned size;
};


// Stores an edge by its vertices and force an order between them
struct Edge
{
	Edge(unsigned int _a, unsigned int _b)
	{
		assert(_a != _b);

		if (_a < _b)
		{
			a = _a;
			b = _b;                   
		}
		else
		{
			a = _b;
			b = _a;
		}
	}

	void Print()
	{
		printf("Edge %d %d\n", a, b);
	}

	unsigned int a;
	unsigned int b;
};

struct Neighbors
{
	unsigned int n1;
	unsigned int n2;

	Neighbors()
	{
		n1 = n2 = (unsigned int)-1;
	}

	void AddNeigbor(unsigned int n)
	{
		if (n1 == -1) {
			n1 = n;
		}
		else if (n2 == -1) {
			n2 = n;
		}
		else {
			assert(0);
		}
	}

	unsigned int GetOther(unsigned int me) const
	{
		if (n1 == me) {
			return n2;
		}
		else if (n2 == me) {
			return n1;
		}
		else {
			assert(0);
		}

		return 0;
	}
};



struct CompareEdges
{
	bool operator()(const Edge& Edge1, const Edge& Edge2)
	{
		if (Edge1.a < Edge2.a) {
			return true;
		}
		else if (Edge1.a == Edge2.a) {
			return (Edge1.b < Edge2.b);
		}        
		else {
			return false;
		}            
	}
};





#pragma warning( push )
#pragma warning( disable : 4351)
 
struct Face
{
	Face(): indices() {}
	unsigned int indices[3];

	unsigned int GetOppositeIndex(const Edge& e) const
	{
		for (unsigned int i = 0 ; i < 3 ; i++) {
			unsigned int Index = indices[i];

			if (Index != e.a && Index != e.b) {
				return Index;
			}
		}

		assert(0);

		return 0;
	}
};

#pragma warning( pop )

struct Vertex
{
	Vertex(glm::vec3 pos,glm::vec2 tex,glm::vec3 nrml) :position(pos), textureCoordinates(tex) , normal(nrml){}
	Vertex(){}
	glm::vec3 position;
	glm::vec2 textureCoordinates;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 cotangent;

	bool operator==(const Vertex& other) const 
	{
		if(this->position.x == other.position.x &&
			this->position.y == other.position.y &&
			this->position.z == other.position.z ) return true;
		return false;
	}

	
	bool operator<(const Vertex& that) const{
		return memcmp((void*)this, (void*)&that, sizeof(Vertex))>0;
	};
};

struct CompareVertices
{
	bool operator()(const Vertex& a, const Vertex& b) const
	{
		if (a.position.x < b.position.x) {
			return true;
		}
		else if (a.position.x == b.position.x) {
			if (a.position.y < b.position.y) return true;
			else if (a.position.y == b.position.y && a.position.z < b.position.z) return true;
		}
		return false;
	}
};

}