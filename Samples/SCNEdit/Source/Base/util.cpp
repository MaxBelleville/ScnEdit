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
//
int str_equals_lim(const char *one, const char *two,int lim){
  int i;
  for (i = 0;i< lim; i++) {
	if (two[i] != one[i]) 
		return 0;
	if (!one[i]) //When reach a point that one > index return 1 (Ie two fully matches)
		return 1;
  }
  return 1;
}

int str_equiv_lim(const char *one,const char *two, int lim) {
  int i;
  for (i = 0; i < lim; i++) {
	if (tolower(two[i]) != tolower(one[i])) 
		return 0;
	if (! one[i]) 
		return 1;
  }
  return 1;
}
int str_equals(const char* one, const char* two) {
	return str_equals_lim(one, two, 2048);
}
int str_equiv(const char* one, const char* two) {
	return str_equiv_lim(one, two, 2048);
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
bool copy_file(const char * existfile, const char * newfile)
{
	std::ifstream ifs(existfile, std::ios::in | std::ios::binary);
	std::ofstream ofs(newfile, std::ios::out | std::ios::trunc | std::ios::binary);

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
		if(file)
			fclose(file);
		return true;
	}

	return false;
}
core::vector2df convert_vec2(const char* pos) {
	core::array<std::string> split = str_split(pos, " ");
	if (split.size() == 2) 
		return core::vector2df(stof(split[0]), stof(split[1]));
}



core::vector3df convert_vec3(const char* pos) {

	core::array<std::string> split= str_split(pos," ");
	
	if (split.size() == 2) 
		return core::vector3df(stof(split[0]), stof(split[1]),0);

	if (split.size() == 3) 
		return core::vector3df(stof(split[0]), stof(split[1]), stof(split[2]));

	if (split.size() == 4) 
		return core::vector3df(stof(split[1]), stof(split[2]), stof(split[3]));// Assume argb

}
std::string vec2_to_str(core::vector2df pos, int decimals) {
	std::string fmt = std::format("{{:.{}f}} {{:.{}f}}", decimals, decimals);
	return std::vformat(fmt, std::make_format_args(pos.X, pos.Y));
}

std::string vec3_to_str(core::vector3df pos, int decimals) {
	std::string fmt = std::format("{{:.{}f}} {{:.{}f}} {{:.{}f}}", decimals, decimals, decimals);
	return std::vformat(fmt, std::make_format_args(pos.X, pos.Y, pos.Z));
}

SColor convert_color(const char* pos) {
	core::array<std::string> split = str_split(pos, " ");
	if (split.size() == 3) 
		return SColor(255,stof(split[0]), stof(split[1]), stof(split[2]));
	
	if (split.size() == 4) 
		return SColor(255,stof(split[1]), stof(split[2]), stof(split[3]));// Assume argb
	
}

ITexture* convert_image(io::path file) {
	ITexture* t = getVideoDriver()->findTexture(file);
	if (t)
		return t;

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

		//Convert each entry in folder to lower case for easy searching.
		std::transform(data.begin(), data.end(), data.begin(),
			[](unsigned char c) { return std::tolower(c); });

		if (data.find(file) != std::string::npos) 
			found.push_back(data);
	}
	return found;
}

core::array<ITexture*> get_skybox(const char* file) {
	core::array<ITexture*> textures;
	core::array<std::string> split = str_split(file, "\\");
	core::array<std::string> found;

	if (split.size() == 1 && !str_equiv(file,"")) {
	   found= search_dir("textures/skybox/", file);
 
		if (found.size()==0) 
			found = search_dir("textures/Skyboxes/", file);
	}
	else 
		printf("other case %s\n",file);


	if (found.size() == 0) 
		os::Printer::log("Warning: using default skybox", irr::ELL_WARNING);
	
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

	if (found.size() == 0) 
		found = search_dir("textures/decals/", file);

	if (found.size() == 0) 
		found = search_dir("textures/decal/", file);

	if (found.size() == 0) 
		found = search_dir("textures/", file);

	for (int i = 0; i < found.size(); i++) {
		ITexture* texture = convert_image(found[i].c_str());
		textures.push_back(texture);
	}

	return textures;
}
IMeshBuffer* generate_cube_mesh_buff(core::vector3df start, core::vector3df end, SColor clr)
{
	IVideoDriver* driver = getVideoDriver();
	IMeshBuffer* MeshBuffer = new CMeshBuffer<S3DVertex>(driver->getVertexDescriptor(EVT_STANDARD), EIT_16BIT);

	IIndexBuffer* ib = MeshBuffer->getIndexBuffer();
	IVertexBuffer* vb = MeshBuffer->getVertexBuffer();
	vb->reallocate(12);

	video::S3DVertex* Vertices = get_cube_vertices(start, end, clr);

	for (u32 i = 0; i < 24; ++i)
		vb->addVertex(&Vertices[i]);

	// cube mesh
	ib->set_used(36);

	for (u32 i = 0; i < 36; ++i)
		ib->setIndex(i, get_cube_indices()[i]);

	// recalc bbox
	MeshBuffer->recalculateBoundingBox();
	return MeshBuffer;
}

video::S3DVertex* get_cube_vertices(core::vector3df start, core::vector3df end, SColor clr)
{
	video::S3DVertex Vertices[] = {
		// back
		video::S3DVertex(start.X, start.Y, start.Z, 0, 0, -1, clr, 0, 1),
		video::S3DVertex(end.X, start.Y, start.Z, 0, 0, -1, clr, 1, 1),
		video::S3DVertex(end.X, end.Y, start.Z, 0, 0, -1, clr, 1, 0),
		video::S3DVertex(start.X, end.Y, start.Z, 0, 0, -1, clr, 0, 0),

		// front
		video::S3DVertex(start.X, start.Y, end.Z, 0, 0, 1, clr, 1, 1),
		video::S3DVertex(end.X, start.Y, end.Z, 0, 0, 1, clr, 0, 1),
		video::S3DVertex(end.X, end.Y, end.Z, 0, 0, 1, clr, 0, 0),
		video::S3DVertex(start.X, end.Y, end.Z, 0, 0, 1, clr, 1, 0),

		// bottom
		video::S3DVertex(start.X, start.Y, start.Z, 0, -1, 0, clr, 0, 1),
		video::S3DVertex(start.X, start.Y, end.Z, 0, -1, 0, clr, 1, 1),
		video::S3DVertex(end.X, start.Y, end.Z, 0, -1, 0, clr, 1, 0),
		video::S3DVertex(end.X, start.Y, start.Z, 0, -1, 0, clr, 0, 0),

		// top
		video::S3DVertex(start.X, end.Y, start.Z, 0, 1, 0, clr, 0, 1),
		video::S3DVertex(start.X, end.Y, end.Z, 0, 1, 0, clr, 1, 1),
		video::S3DVertex(end.X, end.Y, end.Z, 0, 1, 0, clr, 1, 0),
		video::S3DVertex(end.X, end.Y, start.Z, 0, 1, 0, clr, 0, 0),

		// left
		video::S3DVertex(end.X, start.Y, start.Z, 1, 0, 0, clr, 0, 1),
		video::S3DVertex(end.X, start.Y, end.Z, 1, 0, 0, clr, 1, 1),
		video::S3DVertex(end.X, end.Y, end.Z, 1, 0, 0, clr, 1, 0),
		video::S3DVertex(end.X, end.Y, start.Z, 1, 0, 0, clr, 0, 0),

		// right
		video::S3DVertex(start.X, start.Y, start.Z, -1, 0, 0, clr, 1, 1),
		video::S3DVertex(start.X, start.Y, end.Z, -1, 0, 0, clr, 0, 1),
		video::S3DVertex(start.X, end.Y, end.Z, -1, 0, 0, clr, 0, 0),
		video::S3DVertex(start.X, end.Y, start.Z, -1, 0, 0, clr, 1, 0),
	};
	return Vertices;
}

const u16* get_cube_indices()
{
	const u16 cube_index[36] =
	{
		// back
		0,2,1,
		0,3,2,

		// front
		4,5,6,
		4,6,7,

		// bottom
		8,10,9,
		8,11,10,

		// top
		12,13,14,
		12,14,15,

		// left
		16,18,17,
		16,19,18,

		// right
		20,21,22,
		20,22,23
	};
	return cube_index;
}
