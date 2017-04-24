#include "ObjectLoader.h"

using namespace µTec;

ObjectLoader::ObjectLoader(void)
{
}

ObjectLoader::~ObjectLoader(void)
{
}

bool ObjectLoader::loadOBJ( const char* name, const char * path, std::vector<glm::vec3> & out_vertices, std::vector<glm::vec2> & out_uvs, std::vector<glm::vec3> & out_normals )
{
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE * file = fopen(path, "r");
	if( file == NULL ){
		printf("Impossible to open the file ! Are you in the right path ?");
		getchar();
		return false;
	}

	while( 1 ){

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader
		if ( strcmp( lineHeader, "g" ) == 0 ){
			fscanf(file, "%s\n", name );
		}
		else if ( strcmp( lineHeader, "v" ) == 0 ){
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
			temp_vertices.push_back(vertex);
		}else if ( strcmp( lineHeader, "vt" ) == 0 ){
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y );
			//uv.y = -uv.y; 
			temp_uvs.push_back(uv);
		}else if ( strcmp( lineHeader, "vn" ) == 0 ){
			glm::vec3 normal;
			fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
			temp_normals.push_back(normal);
		}else if ( strcmp( lineHeader, "f" ) == 0 ){
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
			if (matches != 9){
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices    .push_back(uvIndex[0]);
			uvIndices    .push_back(uvIndex[1]);
			uvIndices    .push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}else{
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for( unsigned int i=0; i<vertexIndices.size(); i++ ){

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[ vertexIndex-1 ];
		glm::vec2 uv = temp_uvs[ uvIndex-1 ];
		glm::vec3 normal = temp_normals[ normalIndex-1 ];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs     .push_back(uv);
		out_normals .push_back(normal);

	}

	return true;
}

bool ObjectLoader::getSimilarVertexIndex_fast( Vertex & packed, std::map<Vertex,unsigned int> & VertexToOutIndex, unsigned int & result )
{
	std::map<Vertex,unsigned int>::iterator it = VertexToOutIndex.find(packed);
	if ( it == VertexToOutIndex.end() ){
		return false;
	}else{
		result = it->second;
		return true;
	}
}

void ObjectLoader::indexVBO( std::vector<glm::vec3> & in_vertices, std::vector<glm::vec2> & in_uvs, std::vector<glm::vec3> & in_normals, std::vector<unsigned int> & out_indices, std::vector<glm::vec3> & out_vertices, std::vector<glm::vec2> & out_uvs, std::vector<glm::vec3> & out_normals )
{
	std::map<Vertex,unsigned int> VertexToOutIndex;

	// For each input vertex
	for ( unsigned int i=0; i<in_vertices.size(); i++ ){

		//Vertex packed = {in_vertices[i], in_uvs[i], in_normals[i]};
		Vertex packed = Vertex(in_vertices[i], in_uvs[i], in_normals[i]);

		// Try to find a similar vertex in out_XXXX
		unsigned int index;
		bool found = getSimilarVertexIndex_fast( packed, VertexToOutIndex, index);

		if ( found ){ // A similar vertex is already in the VBO, use it instead !
			out_indices.push_back( index );
		}else{ // If not, it needs to be added in the output data.
			out_vertices.push_back( in_vertices[i]);
			out_uvs     .push_back( in_uvs[i]);
			out_normals .push_back( in_normals[i]);
			unsigned int newindex = (unsigned int)out_vertices.size() - 1;
			out_indices .push_back( newindex );
			VertexToOutIndex[ packed ] = newindex;
		}
	}
}

std::vector<Renderable*> ObjectLoader::loadObjectsFromDir( const char* objPath )
{
	std::vector<Renderable*> renderables;
	DIR *dir;
	struct dirent *ent;

	dir = opendir (objPath);
	if (dir != NULL) {

		/* Print all files and directories within the directory */
		while ((ent = readdir (dir)) != NULL) {
			switch (ent->d_type) {
			case DT_REG:
				{
					std::string filename=ent->d_name;
					int dotPos;
					std::string fileType;
					if(( dotPos = filename.find('.'))!= -1)
					{
						fileType=filename.substr(dotPos+1,3);
					}
					std::string fullPath= objPath;
					fullPath+='\\'+filename;
					if(fileType=="obj" ) renderables.push_back(loadObjectfromFile(fullPath.c_str()));
					break;
				}
			default:
				break;
			}
		}

		closedir (dir);
		return renderables;
	} else {
		std::cout<<"Cannot open directory "<<objPath<<std::endl;
		return renderables;
	}
}

void CaculateTangentsBitangents( VertexLayout* def ) 
{
	for ( unsigned i=0; i<def->indexCount; i+=3)
	{
		// Shortcuts for vertices
		glm::vec3 & v0 = def->vertices[def->indices[i]].position;
		glm::vec3 & v1 = def->vertices[def->indices[i+1]].position;
		glm::vec3 & v2 = def->vertices[def->indices[i+2]].position;

		// Shortcuts for UVs
		glm::vec2 & uv0 = def->vertices[def->indices[i]].textureCoordinates;
		glm::vec2 & uv1 = def->vertices[def->indices[i+1]].textureCoordinates;
		glm::vec2 & uv2 = def->vertices[def->indices[i+2]].textureCoordinates;

		// Edges of the triangle : postion delta
		glm::vec3 deltaPos1 = v1-v0;
		glm::vec3 deltaPos2 = v2-v0;

		// UV delta
		glm::vec2 deltaUV1 = uv1-uv0;
		glm::vec2 deltaUV2 = uv2-uv0;

		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		glm::vec3 tangent = (deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y)*r;
		glm::vec3 bitangent = (deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x)*r;
		def->vertices[def->indices[i]].tangent=tangent;
		def->vertices[def->indices[i+1]].tangent=tangent;
		def->vertices[def->indices[i+2]].tangent=tangent;
		def->vertices[def->indices[i]].cotangent=bitangent;
		def->vertices[def->indices[i+1]].cotangent=bitangent;
		def->vertices[def->indices[i+2]].cotangent=bitangent;

	}
}

Renderable* ObjectLoader::loadObjectfromFile( const char* objPath )
{
	std::vector<glm::vec3> raw_vertices;
	std::vector<glm::vec2> raw_uvs;
	std::vector<glm::vec3> raw_normals;
	std::vector<unsigned int> indices;
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	const char * name= new char[128]();
	loadOBJ(name,objPath,raw_vertices,raw_uvs,raw_normals);
	indexVBO(raw_vertices,raw_uvs,raw_normals,indices,vertices,uvs,normals);


	Renderable* renderable = new Renderable();
	VertexLayout* def= new VertexLayout(indices.size(),vertices.size());

	for(unsigned i=0; i < def->vertexCount; ++i)
	{
		def->vertices[i].normal=normals[i];
		def->vertices[i].position= vertices[i];
		def->vertices[i].textureCoordinates= uvs[i];
	}

	for(unsigned i=0,j=0; i < indices.size();i++)
	{
			def->indices[i]=indices[i];
	}
	renderable->defaultLayout=def;

	//FindAdjacencies(renderable);

	CaculateTangentsBitangents(def);

	renderable->name=name;
	return renderable;
}

