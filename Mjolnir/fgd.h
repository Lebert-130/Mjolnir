#ifndef FGD_H
#define FGD_H

#pragma warning(disable : 4786)
#include <map>
#include <string>
#include <vector>

#include "vector.h"

class CFgd
{
public:
	void LoadFGD(const char* filename);
	void FGDToList(wxArrayString& array, bool isPointClass);
private:
	FILE* fgdFile;
};

template<typename T>
struct Attribute {
	std::string description;
	T value;
};

typedef struct {
	char classname[24];
	char description[64];
	std::string defaultvalue;
	std::vector<Attribute<std::vector<std::string>>> choiceAttributes;
	std::vector<Attribute<std::string>> stringAttributes;
	//The reason why this vector exists is to keep consistency. I don't want to put all the choice attributes at the end of the list.
	std::vector<std::string> allAttributes;
} SolidClass;

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
extern std::map<std::string, SolidClass> scMap;

#endif