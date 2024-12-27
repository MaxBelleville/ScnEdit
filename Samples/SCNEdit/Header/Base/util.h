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
int str_equals(const char *one, const char *two);
int str_nequals(const char *one, const char *two,int n);
int str_equiv(const char *one, const char *two);

void error(bool fatal, const char* message, ...);

void read_generic(void * buffer, std::ifstream* file, int nbytes);

void write_generic(void* buffer, std::ofstream* file, int nbytes);

u32 read_u32(std::ifstream* file);

s32 read_s32(std::ifstream* file);

u16 read_u16(std::ifstream* file);

s16 read_s16(std::ifstream* file);

f32 read_f32(std::ifstream* file);

bool is_number(const char* str);

bool CopyFile_(const char * existfile, const char * newfile);

bool can_open(const char * path);

SColor convert_color(const char* pos);
core::vector3df convert_vec3(const char* pos);
core::vector2df convert_vec2(const char* pos);

core::array<std::string> search_dir(std::string dir, const char* file);

core::array<ITexture*> get_skybox(const char* file);

core::array<ITexture*> get_decals(const char* file);

ITexture* convert_image(io::path file);


