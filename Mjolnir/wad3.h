#ifndef WAD3_H
#define WAD3_H

typedef struct
{
	char		identification[4];
	int			numlumps;
	int			infotableofs;
} wadinfo_t;

typedef struct
{
	int			filepos;
	int			disksize;
	int			size;					// uncompressed
	char		type;
	char		compression;
	char		pad1, pad2;
	char		name[16];				// must be null terminated
} lumpinfo_t;

#define	MIPLEVELS	4
typedef struct miptex_s
{
	char		name[16];
	unsigned	width, height;
	unsigned	offsets[MIPLEVELS];		// four mip maps stored
} miptex_t;

typedef struct
{
	std::string name;
	wxImage guiImage;
} Texture;

void LoadWadFile(const char* wadfile);

extern std::vector<Texture> textures;

#endif