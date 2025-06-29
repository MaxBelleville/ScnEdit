#include "pch.h"
#include "Header/Base/util.h"
#include <stdarg.h>
#include <stdlib.h>
#include <filesystem>

void error(bool fatal, const char* message, ...)
{
	va_list args;

	printf("ERROR: ");

	va_start(args, message);
	std::vprintf(message, args);
	va_end(args);

	printf("\n");
	if (fatal)
		exit(1);
}

core::array<std::string> str_split(const char* charStr, const char * delmChar) {
	std::string str(charStr);
	std::string delm(delmChar);
	size_t pos = 0;
	core::array<std::string> arr;
	arr.clear();
	std::string token;
	while ((pos = str.find(delm)) != std::string::npos) {
		token = str.substr(0, pos);
		arr.push_back(token);
		str.erase(0, pos + delm.length());
	}
	arr.push_back(str);
	return arr;
}


//stolen from furrycat
int str_nequals(const char *one, const char *two,int n){
  int i;
  for (i = 0;i<n; i++) {
	if (two[i] != one[i]) return 0;
	if (! one[i]) return 1;
  }
  return 1;
}
int str_equals(const char *one, const char *two) {
  int i;
  for (i = 0; ; i++) {
	if (two[i] != one[i]) return 0;
	if (! one[i]) return 1;
  }
}

int str_equiv(const char *one,const char *two) {
  int i;
  for (i = 0; ; i++) {
	if (tolower(two[i]) != tolower(one[i])) return 0;
	if (! one[i]) return 1;
  }
}
//Reads files characters
void read_generic(void * buffer, std::ifstream* file, int nbytes)
{
	if (file->is_open())
	{
		file->read((char*)buffer,nbytes);
		if (!file->fail())
			return;
	}
	//else
	//error(true,"read_generic: Error reading {}, {} bytes",file->getFileName(),nbytes);
	error(true,"read_generic: Error reading {} bytes",nbytes);
}

//Reads files characters
void write_generic(void* buffer, std::ofstream* file, int nbytes)
{
	if (file->is_open())
	{
		file->write((char*)buffer, nbytes);
		if (!file->fail())
			return;
	}
	//else
	//error(true,"read_generic: Error reading {}, {} bytes",file->getFileName(),nbytes);
	error(true, "read_generic: Error reading {} bytes", nbytes);
}


u32 read_u32(std::ifstream* file){
	u32 ret;
	read_generic(&ret,file,sizeof(u32));
	return ret;
}

s32 read_s32(std::ifstream* file){
	s32 ret;
	read_generic(&ret,file,sizeof(s32));
	return ret;
}


u16 read_u16(std::ifstream* file){
	u16 ret;
	read_generic(&ret,file,sizeof(u16));
	return ret;
}

s16 read_s16(std::ifstream* file){
	s16 ret;
	read_generic(&ret,file,sizeof(s16));
	return ret;
}

f32 read_f32(std::ifstream* file){
	f32 ret;
	read_generic(&ret,file,sizeof(f32));
	return ret;
}

bool is_number(const char* str)
{
	char* end = nullptr;
	double val = strtod(str, &end);
	return end != str && *end == '\0' && val != HUGE_VAL;
}

using namespace std;

//create new file (overwritting if exists) and copy contents from existfile
bool CopyFile_(const char * existfile, const char * newfile)
{
	ifstream ifs(existfile,ios::in | ios::binary);
	ofstream ofs(newfile,ios::out| ios::trunc | ios::binary);

	if (!ifs.is_open() || !ofs.is_open())
		return false;

	ofs << ifs.rdbuf();
	ifs.close();
	ofs.close();
	return true;
}

bool can_open(const char* path) {
	FILE* file;
	errno_t err;
	err = fopen_s(&file, path, "r");
	if (err==0) {
		if(file)fclose(file);
		return true;
	}
	else {
		return false;
	}
}
core::vector2df convert_vec2(const char* pos) {
	core::array<std::string> split = str_split(pos, " ");
	if (split.size() == 2) {
		return core::vector2df(stof(split[0]), stof(split[1]));
	}
}



core::vector3df convert_vec3(const char* pos) {

	
	core::array<std::string> split= str_split(pos," ");
	
	if (split.size() == 2) {
		return core::vector3df(stof(split[0]), stof(split[1]),0);
	}
	if (split.size() == 3) {
		return core::vector3df(stof(split[0]), stof(split[1]), stof(split[2]));
	}
	if (split.size() == 4) {
		return core::vector3df(stof(split[1]), stof(split[2]), stof(split[3]));// Assume argb
	}

}

SColor convert_color(const char* pos) {
	core::array<std::string> split = str_split(pos, " ");
	if (split.size() == 3) {
		return SColor(255,stof(split[0]), stof(split[1]), stof(split[2]));
	}
	if (split.size() == 4) {
		return SColor(255,stof(split[1]), stof(split[2]), stof(split[3]));// Assume argb
	}
}

ITexture* convert_image(io::path file) {
	ITexture* t = getVideoDriver()->findTexture(file);
	if (t) return t;

	video::IImage* image = getVideoDriver()->createImageFromFile(file);
	if (image) {
		if (image->getColorFormat() != video::ECF_A8R8G8B8) {
			video::IImage* converted = getVideoDriver()->createImage(video::ECF_R8G8B8, image->getDimension());
			if (converted) {
				image->copyTo(converted);
				image->drop();
				getVideoDriver()->addTexture(file, converted);
				converted->drop();
			}
		}
		return  getVideoDriver()->getTexture(file);
	}
	return nullptr;
}

core::array<std::string> search_dir(std::string dir, const char* file) {
	core::array<std::string> found;
	for (const auto& entry : std::filesystem::directory_iterator(dir)) {
		std::string data = entry.path().string();
		std::transform(data.begin(), data.end(), data.begin(),
			[](unsigned char c) { return std::tolower(c); });
		if (data.find(file) != std::string::npos) {
			found.push_back(data);
		}
	}
	return found;
}

core::array<ITexture*> get_skybox(const char* file) {
	core::array<ITexture*> textures;
	core::array<std::string> split = str_split(file, "\\");
	core::array<std::string> found;
	if (split.size() == 1 && !str_equiv(file,"")) {
	   found= search_dir("textures/skybox/", file);
 
		if (found.size()==0) found = search_dir("textures/Skyboxes/", file);
	}
	else {
		printf("other case %s\n",file);
	}

	if (found.size() == 0) {
		os::Printer::log("Warning: using default skybox", irr::ELL_WARNING);
	}
	for (int i = 0; i < found.size(); i++) {
		ITexture* texture = convert_image(found[i].c_str());
		textures.push_back(texture);
	}

	return textures;
 }
core::array<ITexture*> get_decals(const char* file) {
	core::array<ITexture*> textures;
	core::array<std::string> found;

	found = search_dir("textures/sprites/", file);

	if (found.size() == 0) found = search_dir("textures/decals/", file);

	if (found.size() == 0) found = search_dir("textures/decal/", file);

	if (found.size() == 0) found = search_dir("textures/", file);

	for (int i = 0; i < found.size(); i++) {
		ITexture* texture = convert_image(found[i].c_str());
		textures.push_back(texture);
	}

	return textures;
}
