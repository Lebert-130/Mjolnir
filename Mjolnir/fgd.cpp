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
	while(fgets(line, sizeof(line), fgdFile)){
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
	}
}

void CFgd::FGDToList(wxArrayString& array)
{
	for(std::map<std::string, PointClass>::iterator it = pcMap.begin(); it != pcMap.end(); it++)
		array.Add(it->first);
}
