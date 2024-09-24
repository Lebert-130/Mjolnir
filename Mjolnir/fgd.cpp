#pragma warning(disable : 4786)
#include <Windows.h>
#include <stdio.h>
#include <map>
#include <string>

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#	include "wx/wx.h"
#endif

#include "vector.h"
#include "fgd.h"

std::map<std::string, PointClass> pcMap;
std::map<std::string, SolidClass> scMap;

int CountColons(const std::string& str)
{
	int count = 0;
	for (char ch : str){
		if (ch == ':'){
			count++;
		}
	}
	return count;
}

void CFgd::LoadFGD(const char* filename)
{
	std::map<std::string, BaseClass> bcMap;

	fgdFile = fopen(filename, "r");
	if(!fgdFile){
		char message[64];
		sprintf(message, "Failed to load %s FGD file.", filename);
		MessageBox(NULL, message, "FGD Error", MB_ICONERROR|MB_OK);
		exit(-1);
	}

	char line[256];
	SolidClass tmp_sc;
	Attribute<std::vector<std::string>> tmp_atr_v;
	std::vector<std::string> choices;

	bool insideAttributes = false, insideChoices = false;
	while(fgets(line, sizeof(line), fgdFile)){
		std::string trimmedLine = line;
		trimmedLine.erase(0, trimmedLine.find_first_not_of(" \t"));

		if (strnicmp(line, "@SolidClass", strlen("@SolidClass")) == 0){
			sscanf(line, "@SolidClass = %s : \"%[^\"]\"", &tmp_sc.classname, &tmp_sc.description);
		}
		if (trimmedLine[0] == '['){
			if (!insideAttributes){
				insideAttributes = true;
				continue;
			}
			else if (!insideChoices){
				insideChoices = true;
				continue;
			}
		}
		if (trimmedLine[0] == ']'){
			if (insideChoices){
				insideChoices = false;
				tmp_atr_v.value = choices;
				tmp_sc.choiceAttributes.push_back(tmp_atr_v);
				continue;
			}
			else if (insideAttributes){
				insideAttributes = false;
				scMap[tmp_sc.classname] = tmp_sc;
				continue;
			}
		}
		if(strnicmp(line, "@BaseClass", strlen("@BaseClass")) == 0){
			BaseClass tmp_bc;
			sscanf(line, "@BaseClass size(%f %f %f, %f %f %f) color(%f %f %f) = %s []", &tmp_bc.sizeMin[0], &tmp_bc.sizeMin[1], &tmp_bc.sizeMin[2],
				&tmp_bc.sizeMax[0], &tmp_bc.sizeMax[1], &tmp_bc.sizeMax[2], &tmp_bc.color[0], &tmp_bc.color[1], &tmp_bc.color[2], tmp_bc.classname);
			bcMap[tmp_bc.classname] = tmp_bc;
		}
		if(strnicmp(line, "@PointClass", strlen("@PointClass")) == 0){
			PointClass tmp_pc;
			char baseclass[24];
			sscanf(line, "@PointClass base(%23[^)]) = %23s : \"%[^\"]\"", baseclass, tmp_pc.classname, tmp_pc.description);

			tmp_pc.sizeMin = bcMap[baseclass].sizeMin;
			tmp_pc.sizeMax = bcMap[baseclass].sizeMax;
			tmp_pc.color = bcMap[baseclass].color;

			pcMap[tmp_pc.classname] = tmp_pc;
		}
		if (insideAttributes && !insideChoices){
			char type[32];
			char name[32];
			char description[128];
			char defaultvalue[16];

			if (CountColons(trimmedLine) == 1){
				sscanf(trimmedLine.c_str(), "%31[^ (](%31[^)]) : \"%127[^\"]\"", name, type, description);

				tmp_sc.allAttributes.push_back(description);

				if (stricmp(type, "choices") != 0){
					Attribute<std::string> tmp_atr;
					tmp_atr.value = name;
					tmp_atr.description = description;
					if (stricmp(type, "integer") == 0)
						tmp_atr.defaultvalue = "0";
					tmp_sc.stringAttributes.push_back(tmp_atr);
				}
			}
			if (CountColons(trimmedLine) == 2){
				sscanf(trimmedLine.c_str(), "%31[^ (](%31[^)]) : \"%127[^\"]\" : %s", name, type, description, defaultvalue);

				tmp_sc.allAttributes.push_back(description);

				if (stricmp(type, "choices") != 0){
					Attribute<std::string> tmp_atr;
					tmp_atr.value = name;
					tmp_atr.description = description;
					tmp_atr.defaultvalue = defaultvalue;
					tmp_sc.stringAttributes.push_back(tmp_atr);
				} else {
					tmp_atr_v.description = description;
				}
			}
		}
		if (insideAttributes && insideChoices){
			char number[16];
			char choice[32];
			sscanf(trimmedLine.c_str(), "%s : \"%127[^\"]\"", number, choice);

			choices.push_back(choice);
		}
	}
}

void CFgd::FGDToList(wxArrayString& array, bool isPointClass)
{
	if (isPointClass){
		for (std::map<std::string, PointClass>::iterator it = pcMap.begin(); it != pcMap.end(); it++)
			array.Add(it->first);
	}
	else {
		for (std::map<std::string, SolidClass>::iterator it = scMap.begin(); it != scMap.end(); it++)
			array.Add(it->first);
	}
}
