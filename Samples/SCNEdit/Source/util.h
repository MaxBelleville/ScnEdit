#define UTIL_H_
#include <iostream>
#include <vector>
#include <fstream>
#include <cstdarg>
#include <format>
using namespace irr;




/*
template<class T>
  for_each(T first, T last, Function f)
  {
    for ( ; first!=last; ++first ) f(*first);
    return f;
  }
  */




//from furrycat's
core::array<std::string> str_split(const char* charStr, const char* delmChar);
int str_equals(char *one, const char *two);
int str_nequals(char *one, const char *two,int n);
int str_equiv(const char *one, const char *two);
core::stringw SAY(const char* c, ...);
core::stringw WARN(const char* c, ...);
char* _print(const char* c, va_list args);

void error(bool fatal, const char * message,...);

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

ITexture* convert_image(io::path file);