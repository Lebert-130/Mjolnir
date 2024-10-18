#include <Windows.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <string>

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#	include "wx/wx.h"
#endif

#include "wad3.h"

std::vector<lumpinfo_t*> lumps;
std::vector<Texture> textures;

void LoadWadFile(const char* wadfile)
{
	FILE* wad = fopen(wadfile, "rb");
	if (!wad){
		MessageBox(NULL, "Failed to load WAD file", "WAD Error", MB_ICONERROR | MB_OK);
		exit(-1);
	}

	wadinfo_t* wadInfo = new wadinfo_t();

	fread(wadInfo->identification, sizeof(char), 4, wad);

	if (stricmp(wadInfo->identification, "WAD3") != 0){
		MessageBox(NULL, "WAD file doesn't have WAD3 id", "WAD Error", MB_ICONERROR | MB_OK);
		exit(-1);
	}

	fread(&wadInfo->numlumps, sizeof(int), 1, wad);
	fread(&wadInfo->infotableofs, sizeof(int), 1, wad);

	fseek(wad, wadInfo->infotableofs, SEEK_SET);

	for (int i = 0; i < wadInfo->numlumps; i++){
		lumpinfo_t* lumpInfo = new lumpinfo_t();
		fread(lumpInfo, sizeof(lumpinfo_t), 1, wad);

		lumps.push_back(lumpInfo);
	}

	for (int i = 0; i < wadInfo->numlumps; i++){
		fseek(wad, lumps[i]->filepos, SEEK_SET);

		int imageStartPos = ftell(wad);

		miptex_t* miptex = new miptex_t();
		fread(miptex, sizeof(miptex_t), 1, wad);

		char* image = (char*)malloc(miptex->width * miptex->height);
		fread(image, miptex->width * miptex->height, 1, wad);

		fseek(wad, lumps[i]->filepos, SEEK_SET);
		fseek(wad, miptex->offsets[0], SEEK_CUR);

		unsigned int mipmapSizes = miptex->width * miptex->height;
		unsigned int totalSize = mipmapSizes;
		for (int i = 1; i < 4; i++) {
			mipmapSizes /= 4;
			totalSize += mipmapSizes;
		}

		fseek(wad, totalSize, SEEK_CUR);

		short paletteColors;
		fread(&paletteColors, sizeof(short), 1, wad);

		if (paletteColors != 256){
			MessageBox(NULL, "Palette color must be 256", "WAD Error", MB_ICONERROR | MB_OK);
			exit(-1);
		}

		int offset = ftell(wad) - imageStartPos;
		int paletteSize = lumps[i]->size - offset;

		char* palette = (char*)malloc(paletteSize);
		fread(palette, paletteSize, 1, wad);

		wxImage img(miptex->width, miptex->height);
		unsigned char* imageData = img.GetData();

		for (unsigned int y = 0; y < miptex->height; y++){
			for (unsigned int x = 0; x < miptex->width; x++){
				unsigned char pixel = image[y * miptex->width + x];

				imageData[(y * miptex->width + x) * 3] = palette[pixel * 3]; //Red
				imageData[(y * miptex->width + x) * 3 + 1] = palette[pixel * 3 + 1]; //Green
				imageData[(y * miptex->width + x) * 3 + 2] = palette[pixel * 3 + 2]; //Blue
			}
		}

		Texture texture;
		texture.name = lumps[i]->name;
		texture.guiImage = img;

		textures.push_back(texture);

		free(image);
		free(palette);
	}


	//We can't use wadInfo->numlumps because when the vector is getting erased, the size changes, and since numlumps is static,
	//it's going to lose track and then crash.
	for (int i = 0; i < lumps.size(); ){
		delete lumps[i];
		lumps.erase(lumps.begin() + i);
	}
}