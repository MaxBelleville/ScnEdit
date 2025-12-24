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
	if (fatal) {
		exit(1);
	}
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
std::string str_trim(const char* str, const char* chars) {
	std::string result(str);

	// Trim from start
	size_t start = result.find_first_not_of(chars);
	if (start == std::string::npos) {
		return "";
	}
	result = result.substr(start);

	// Trim from end
	size_t end = result.find_last_not_of(chars);
	if (end != std::string::npos) {
		result = result.substr(0, end + 1);
	}

	return result;
}


//Reads files characters
void read_generic(void * buffer, std::ifstream* file, int nbytes)
{
	if (file->is_open())
	{
		file->read((char*)buffer,nbytes);
		if (!file->fail()) {
			return;
		}
		
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

bool copy_portion(const char* existfile, const char* newfile, int64_t start, int64_t end)
{
	if (start < 0 || end <= start)
		return false;

	std::ifstream ifs(existfile, std::ios::in | std::ios::binary);
	std::ofstream ofs(newfile, std::ios::out | std::ios::app | std::ios::binary);

	if (!ifs.is_open() || !ofs.is_open())
		return false;

	// Ensure end does not exceed file size
	ifs.seekg(0, std::ios::end);
	int64_t filesize = ifs.tellg();
	if (start >= filesize || end > filesize)
		return false;

	// Seek to the start position
	ifs.seekg(start, std::ios::beg);

	const size_t buffer_size = 8192;
	char buffer[buffer_size];
	int64_t bytes_to_copy = end - start;

	while (bytes_to_copy > 0) {
		size_t chunk = static_cast<size_t>(std::min<int64_t>(buffer_size, bytes_to_copy));
		read_generic(buffer, &ifs, static_cast<int>(chunk));
		write_generic(buffer, &ofs, static_cast<int>(chunk));
		bytes_to_copy -= chunk;
	}

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

std::string str_join(const core::array<u32> arr) {
	std::string result = "[";
	for (size_t i = 0; i < arr.size(); ++i) {
		result += std::format("{}", arr[i]);
		if (i + 1 < arr.size()) result += ",";
	}
	result += "]";
	return result;
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
	if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) {
		return found;
	}
	for (const auto& entry : std::filesystem::directory_iterator(dir)) {
		std::string data = entry.path().string();

		// Convert each entry in folder to lower case for easy searching.
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
		os::Printer::log("other case %s\n",file);


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

bool solveProjectionFrame(const  core::array <std::pair<core::vector3df, core::vector2df>>& samples,
	core::vector3df& origin_out,
	core::vector3df& u_axis_out,
	core::vector3df& v_axis_out)
{
	int N = samples.size();
	if (N < 3) return false;
	double maxErr = 0.0;
	double rmsErr = 0.0;
	// Build M^T * M and M^T * P efficiently
	// M has rows [1, u, v]
	double MTM[3][3] = { 0 }; // symmetric
	double MTP[3][3] = { 0 }; // 3 x 3, columns for X,Y,Z

	for (int i = 0; i < N; ++i) {
		double u = samples[i].second.X;
		double v = samples[i].second.Y;
		core::vector3df fit = origin_out + u_axis_out * u + v_axis_out * v;
		double err = (samples[i].first - fit).getLength();
		rmsErr += err * err;
		maxErr = max(maxErr, err);
		double row[3] = { 1.0, u, v };
		// MTM += outer(row, row)
		for (int a = 0; a < 3; ++a)
			for (int b = 0; b < 3; ++b)
				MTM[a][b] += row[a] * row[b];

		// MTP += row^T * P_i  (accumulate for X, Y, Z separately)
		MTP[0][0] += row[0] * samples[i].first.X;
		MTP[1][0] += row[1] * samples[i].first.X;
		MTP[2][0] += row[2] * samples[i].first.X;

		MTP[0][1] += row[0] * samples[i].first.Y;
		MTP[1][1] += row[1] * samples[i].first.Y;
		MTP[2][1] += row[2] * samples[i].first.Y;

		MTP[0][2] += row[0] * samples[i].first.Z;
		MTP[1][2] += row[1] * samples[i].first.Z;
		MTP[2][2] += row[2] * samples[i].first.Z;
	}

	// Invert MTM (3x3). Use a robust 3x3 inversion or a small linear algebra helper.
	double invMTM[3][3];
	if (!invert3x3(MTM, invMTM)) return false;
	rmsErr = sqrt(rmsErr / N);
	printf("rmsErr = %f, maxErr = %f\n", rmsErr, maxErr);
	// X = inv(MTM) * MTP  (3x3 * 3x3 -> 3x3)
	double Xmat[3][3] = { 0 };
	for (int r = 0; r < 3; ++r)
		for (int c = 0; c < 3; ++c) {
			double s = 0.0;
			for (int k = 0; k < 3; ++k) s += invMTM[r][k] * MTP[k][c];
			Xmat[r][c] = s;
		}

	// Extract rows
	origin_out = core::vector3df((float)Xmat[0][0], (float)Xmat[0][1], (float)Xmat[0][2]);
	u_axis_out = core::vector3df((float)Xmat[1][0], (float)Xmat[1][1], (float)Xmat[1][2]);
	v_axis_out = core::vector3df((float)Xmat[2][0], (float)Xmat[2][1], (float)Xmat[2][2]);

	return true;
}

// Helper function to invert a 3x3 matrix.
   // Returns true if inversion succeeded, false if the matrix is singular.
bool invert3x3(const double src[3][3], double dst[3][3])
{
	// Compute the determinant
	double det =
		src[0][0] * (src[1][1] * src[2][2] - src[1][2] * src[2][1]) -
		src[0][1] * (src[1][0] * src[2][2] - src[1][2] * src[2][0]) +
		src[0][2] * (src[1][0] * src[2][1] - src[1][1] * src[2][0]);

	if (fabs(det) < 1e-12)
		return false;

	double invDet = 1.0 / det;

	dst[0][0] = (src[1][1] * src[2][2] - src[1][2] * src[2][1]) * invDet;
	dst[0][1] = -(src[0][1] * src[2][2] - src[0][2] * src[2][1]) * invDet;
	dst[0][2] = (src[0][1] * src[1][2] - src[0][2] * src[1][1]) * invDet;

	dst[1][0] = -(src[1][0] * src[2][2] - src[1][2] * src[2][0]) * invDet;
	dst[1][1] = (src[0][0] * src[2][2] - src[0][2] * src[2][0]) * invDet;
	dst[1][2] = -(src[0][0] * src[1][2] - src[0][2] * src[1][0]) * invDet;

	dst[2][0] = (src[1][0] * src[2][1] - src[1][1] * src[2][0]) * invDet;
	dst[2][1] = -(src[0][0] * src[2][1] - src[0][1] * src[2][0]) * invDet;
	dst[2][2] = (src[0][0] * src[1][1] - src[0][1] * src[1][0]) * invDet;

	return true;
}

