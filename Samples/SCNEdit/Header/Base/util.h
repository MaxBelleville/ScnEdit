#define UTIL_H_
#include <iostream>
#include <vector>
#include <fstream>
#include <cstdarg>
#include <format>
#include <deque>
using namespace irr;

#include "pch.h"


core::array<std::string> str_split(const char* charStr, const char* delmChar);
int str_equals_lim(const char* one, const char* two, int lim);
int str_equiv_lim(const char* one, const char* two, int lim);
int str_equals(const char* one, const char* two);
int str_equiv(const char* one, const char* two);
std::string str_trim(const char* str, const char* chars);

void error(bool fatal, const char* message, ...);

void read_generic(void * buffer, std::ifstream* file, int nbytes);

void write_generic(void* buffer, std::ofstream* file, int nbytes);

u32 read_u32(std::ifstream* file);

s32 read_s32(std::ifstream* file);

u16 read_u16(std::ifstream* file);

s16 read_s16(std::ifstream* file);

f32 read_f32(std::ifstream* file);

bool is_number(const char* str);

bool copy_file(const char * existfile, const char * newfile);
bool copy_portion(const char* existfile, const char* newfile, int64_t start, int64_t end);
bool can_open(const char * path);

SColor convert_color(const char* pos);
core::vector3df convert_vec3(const char* pos);
core::vector2df convert_vec2(const char* pos);
std::string vec2_to_str(core::vector2df pos, int decimals);
std::string toLowerStr(const std::string& s);

std::string vec3_to_str(core::vector3df pos, int decimals);

std::string str_join(const core::array<u32> arr);

core::array<std::string> search_dir(const std::string& dir, const std::string& file);
core::array<std::string> search_dir_recursive(const std::string& dir, const std::string& file);
core::array<ITexture*> get_skybox(const char* file);

core::array<ITexture*> get_decals(const char* file);

ITexture* convert_image(io::path file);

video::S3DVertex* get_cube_vertices(core::vector3df start, core::vector3df end, SColor clr);
IMeshBuffer* generate_cube_mesh_buff(core::vector3df start, core::vector3df end, SColor clr);
const u16* get_cube_indices();

bool invert3x3(const double src[3][3], double dst[3][3]);
