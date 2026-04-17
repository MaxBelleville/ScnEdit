#ifndef UTIL_H_
#define UTIL_H_
#include <iostream>
#include <fstream>
#include <cstdarg>
#include <format>
#include <deque>

using namespace irr;

core::array<std::string> str_split(const char* charStr, const char* delmChar);
int str_equals_lim(const char* one, const char* two, int lim);
int str_equiv_lim(const char* one, const char* two, int lim);
int str_equals(const char* one, const char* two);
int str_equiv(const char* one, const char* two);
std::string str_trim(const char* str, const char* chars);

void error(bool fatal, const char* message, ...);

void read_generic(std::ifstream* file, void* buffer, int nbytes);

void write_generic(std::ofstream* file, void* buffer, int nbytes);

bool is_number(const char* str);

bool copy_file(const char* existfile, const char* newfile);
bool copy_portion(const char* existfile, const char* newfile, int64_t start, int64_t end);
bool can_open(const char* path);

SColor convert_color(const char* pos);
core::vector3df convert_vec3(const char* pos);
core::vector2df convert_vec2(const char* pos);
std::string vec2_to_str(core::vector2df pos, int decimals);
std::string toLowerStr(const std::string& s);

std::string vec3_to_str(core::vector3df pos, int decimals);

std::string str_join(const core::array<u32> arr);

core::array<std::string> search_dir(std::string dir, const char* file);
core::array<std::string> search_dir_recursive(const std::string& dir, const std::string& file);
core::array<ITexture*> get_skybox(const char* file);

core::array<ITexture*> get_decals(const char* file);

ITexture* convert_image(io::path file);

video::S3DVertex* get_cube_vertices(core::vector3df start, core::vector3df end, SColor clr);
IMeshBuffer* generate_cube_mesh_buff(core::vector3df start, core::vector3df end, SColor clr);
const u16* get_cube_indices();

bool invert3x3(const double src[3][3], double dst[3][3]);

const char* getMaterialName(u8 material);

template <typename T>
inline void read_type(std::ifstream* file, T& target) {
	read_generic(file, &target, sizeof(T));
}

template <typename T>
inline void write_type(std::ofstream* file, const T& value) {
	write_generic(file, (void*)&value, sizeof(T));
}

template <typename T, typename Container>
inline void read_pointers(std::ifstream* file, Container& target, u32 count) {
	target.set_used(count);
	for (u32 i = 0; i < count; ++i) {
		target[i] = new T;
		// 2. Read the data into the newly allocated memory
		read_generic(file, target[i], sizeof(T));
	}
}

template <typename T, typename Container>
inline void write_pointers(std::ofstream* file, const Container& target, u32 count) {
	for (u32 i = 0; i < count; ++i)
		write_generic(file, (void*)target[i], sizeof(T));
}
#endif