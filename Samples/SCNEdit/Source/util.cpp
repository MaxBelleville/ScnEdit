#include "pch.h"
#include "util.h"
#include <stdarg.h>
#include <stdlib.h>


core::stringw SAY(const char* message, ...)
{
    core::stringw str;
    va_list args;
    va_start(args, message);
    str= _print(message, args);
    va_end(args);
    return str;
}
core::stringw WARN(const char* message, ...)
{
    core::stringw str;
    va_list args;
    printf("WARN: ");
    va_start(args, message);
    str=_print(message, args);
    va_end(args);
    return str;
}



void error(bool fatal, const char * message,...)
{
	va_list args;

   printf("ERROR: ");

    va_start(args, message);
    _print(message, args);
    va_end(args);

	printf("\n");
   
    if (fatal) {
        system("pause");
        exit(1);
    }
}
char* _print(const char* message, va_list args)
{
    std::string str="";
    char* buff;
    int len;
    len = _vscprintf(message, args)+1;
    buff = (char*)malloc(len * sizeof(char));
    if (NULL != buff)
    {
        vsprintf_s(buff, len, message, args);
        fputs(buff,stdout);
    }
    return buff;
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
int str_nequals(char *one, const char *two,int n){
  int i;
  for (i = 0;i<n; i++) {
    if (two[i] != one[i]) return 0;
    if (! one[i]) return 1;
  }
  return 1;
}
int str_equals(char *one, const char *two) {
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
    //error(true,"read_generic: Error reading %s, %i bytes",file->getFileName(),nbytes);
    error(true,"read_generic: Error reading %i bytes",nbytes);
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
    //error(true,"read_generic: Error reading %s, %i bytes",file->getFileName(),nbytes);
    error(true, "read_generic: Error reading %i bytes", nbytes);
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

ITexture* convert_image(io::path file) {
    CBaseApp* app = getApplication();
    ITexture* t = app->getDriver()->findTexture(file);
    if (t) return t;

    video::IImage* image = app->getDriver()->createImageFromFile(file);
    if (image) {
        if (image->getColorFormat() == video::ECF_A8R8G8B8) {
            // Convert BGRA to RGBA
            video::IImage* converted = app->getDriver()->createImage(video::ECF_R8G8B8, image->getDimension());
            if (converted) {
                image->copyTo(converted);
                app->getDriver()->addTexture(file, converted);
                converted->drop();
            }
        }
        image->drop();
    }
    return app->getDriver()->findTexture(file);
}
