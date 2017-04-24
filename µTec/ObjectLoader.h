#pragma once
#include <vector>
#include <stdio.h>
#include <string>
#include <cstring>
#include <map>
#include "Vertex.h"
#include "Renderable.h"
#include "dirent.h"
#include "glm/glm.hpp"
#include <iostream>
#include <map>

namespace µTec
{

class ObjectLoader
{
public:
	ObjectLoader(void);
	~ObjectLoader(void);

	std::vector<Renderable*> loadObjectsFromDir(const char* objPath);

	Renderable* loadObjectfromFile(const char* objPath);

	bool loadOBJ(const char* name, const char * path, std::vector<glm::vec3> & out_vertices, std::vector<glm::vec2> & out_uvs, std::vector<glm::vec3> & out_normals );


	bool getSimilarVertexIndex_fast(
		Vertex & packed,
		std::map<Vertex,unsigned int> & VertexToOutIndex,
		unsigned int & result
		);

	void indexVBO(
		std::vector<glm::vec3> & in_vertices,
		std::vector<glm::vec2> & in_uvs,
		std::vector<glm::vec3> & in_normals,

		std::vector<unsigned int> & out_indices,
		std::vector<glm::vec3> & out_vertices,
		std::vector<glm::vec2> & out_uvs,
		std::vector<glm::vec3> & out_normals
		);
	Renderable* FindAdjacencies(Renderable* renderable);


};

}