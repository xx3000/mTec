#include "TextureLoader.h"

using namespace µTec;

TextureManager::TextureManager() : textureMap( new std::map<std::string,DynamicImage>())
{
}

TextureManager::TextureManager( const TextureManager& other )
{
	this->textureMap =other.textureMap;
}


TextureManager::~TextureManager(void)
{
	delete textureMap;
}

bool TextureManager::loadFromDir( const char* dirName )
{
	DIR *dir;
	struct dirent *ent;

	dir = opendir (dirName);
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
					std::string fullPath= dirName;
					fullPath+='\\'+filename;
					if(fileType=="png" ) loadFromFile(fullPath.c_str());
					break;
				}
			default:
				break;
			}
		}

		closedir (dir);
		return true;
	} else {
		std::cout<<"Cannot open directory "<<dirName<<std::endl;
		return false;
	}
}

bool TextureManager::loadFromFile( const char* file )
{
	// Load file and decode DynamicImage.
	std::vector<unsigned char> dynamicImage;
	unsigned width, height;
	unsigned error = lodepng::decode(dynamicImage, width, height, file);

	// If there's an error, display it.
	if(error != 0)
	{
		std::cout << "error " << error << ": " << lodepng_error_text(error) << std::endl;
		return false;
	}
	else
	{
		std::string fileName= file;
		unsigned found = fileName.find_last_of("/\\");
		if(found != std::string::npos) fileName=fileName.substr(found+1);
		DynamicImage img;
		img.rawData=dynamicImage;
		img.width=width;
		img.height=height;
		img.format=RGBA;
		img.size=dynamicImage.size();
		(*textureMap)[fileName]=img;
	}
	return true;
}

const std::map<std::string,DynamicImage>* TextureManager::getTextures() const
{
	return textureMap;
}

DynamicImage* TextureManager::getTextureByName( const char* name ) const
{
	if(textureMap->find(name) != textureMap->end()) return &(*textureMap)[name];
	else return nullptr;
}