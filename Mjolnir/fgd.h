#ifndef FGD_H
#define FGD_H

#pragma warning(disable : 4786)
#include <map>
#include <string>

#include "vector.h"

class CFgd
{
public:
	void LoadFGD(const char* filename);
	void FGDToList(wxArrayString& array);
private:
	FILE* fgdFile;
};

typedef struct {
	Vector sizeMin;
	Vector sizeMax;
	Vector color;
	char classname[24];
} BaseClass;

typedef struct {
	Vector sizeMin;
	Vector sizeMax;
	Vector color;
	char classname[24];
	char description[64];
} PointClass;

extern std::map<std::string, PointClass> pcMap;

#endif