#ifndef SPFS_HOOKS
#define SPFS_HOOKS

#include <string.h>
#include <stdio.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include "spfs.h"

typedef struct
{
	int used;
	
	SceUID fd;
	char path[0x100];
	SceOff offset;
	int flags;
	
	spfs_info_t info;
	unsigned current_part;
} splittedfile_t;

#define SPFS_MAX_OPEN_FILES 16
#define SPFS_ERROR_PART_NOT_FOUND 0x805F5001

//flags
#define SPFS_HIDE_PARTS

//macros
#define SPFS_MASK(A) 		((A) | 0x0F000000)
#define SPFS_UNMASK(A) 		((A) & 0x00FFFFFF)
#define SPFS_IS_SPFS(A) 	((A) & 0x0F000000)

//private
int spfs_find_available();
int spfs_get_info(const char * path, spfs_info_t * info);
int spfs_swap_part(unsigned index, unsigned part);

//public
int spfs_start();
SceUID spfs_sceIoOpen(const char * path, int flags, SceMode mode);
SceOff spfs_sceIoLseek(SceUID fd, SceOff offset, int whence);
int spfs_sceIoLseek32(SceUID fd, int offset, int whence);
int spfs_sceIoRead(SceUID fd, void * data, unsigned size);
int spfs_sceIoWrite(SceUID fd, void * data, unsigned size);
int spfs_sceIoClose(SceUID fd);
int spfs_sceIoRemove(const char * path);
int spfs_sceIoRename(const char * oldname, const char * newname);
int spfs_sceIoGetstat(const char * path, SceIoStat * stat);
int spfs_sceIoDread(SceUID fd, SceIoDirent * dir);

//debug
unsigned spfs_debug_readu32(SceUID fd);

#endif