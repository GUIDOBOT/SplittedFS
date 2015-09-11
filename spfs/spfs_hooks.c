//Splitted files system for ePSP by GUIDOBOT

#include "spfs_hooks.h"

static splittedfile_t files[SPFS_MAX_OPEN_FILES];

int spfs_start()
{
	memset(files, 0x0, sizeof(files));
	return 1;
};

int spfs_find_available()
{
	for(int i = 0; i < SPFS_MAX_OPEN_FILES; i++)
	{
		if(!files[i].used)
			return i;
	};
	
	return -1;
};

int spfs_get_info(const char * path, spfs_info_t * info)
{
	//try to open file
	SceUID fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
	if(fd < 0)
		return -1;
	
	//read magic
	int ret = 0;
	unsigned magic = 0;
	sceIoRead(fd, &magic, sizeof(unsigned));
	if(magic == SPFS_MAGIC)
	{
		//read info from main file
		sceIoLseek(fd, 0x0, PSP_SEEK_SET);
		sceIoRead(fd, info, sizeof(spfs_info_t));
		
		ret = 1;
	};
	
	sceIoClose(fd);
	return ret;
};

int spfs_swap_part(unsigned index, unsigned part)
{
	//close current part
	if(files[index].fd >= 0)
		sceIoClose(files[index].fd);
	
	//create full path for this part
	char new_part[0x100];
	sprintf(new_part, "%s.part%03X", files[index].path, part);

	//update status and open new part
	files[index].current_part = part;
	files[index].offset = part * PART_SIZE; //0;
	files[index].fd = sceIoOpen(new_part, PSP_O_RDONLY, 0777);
	
	return files[index].fd;
};

SceUID spfs_sceIoOpen(const char * path, int flags, SceMode mode)
{
	//only for read mode
	if(flags & PSP_O_RDONLY)
	{		
		//get info from spfs main file
		spfs_info_t info;
		if(spfs_get_info(path, &info) > 0)
		{
			//add to the open files list
			int index = spfs_find_available();
			if(index < 0)
				return -1;

			//create data for this entry
			files[index].used = 1;
			files[index].fd = -1;
			files[index].flags = flags;
			files[index].info = info;
			strncpy(files[index].path, path, 0x100);
			
			//open first part of the file
			if(spfs_swap_part(index, 0) < 0)
				return SPFS_ERROR_PART_NOT_FOUND;
			
			//mask the index to recognize its a spfs entry
			return SPFS_MASK(index);
		};
	};
	
	return sceIoOpen(path, flags, mode);
};

SceOff spfs_sceIoLseek(SceUID fd, SceOff offset, int whence)
{
	//check if fd is a spfs entry
	if(SPFS_IS_SPFS(fd))
	{
		unsigned index = SPFS_UNMASK(fd);
		
		//calculate absolute offset after the seek
		SceOff absolute_offset = 0;
		if(whence == PSP_SEEK_SET)
			absolute_offset = offset;
		else if(whence == PSP_SEEK_CUR)
			absolute_offset = files[index].offset + offset;
		else if(whence == PSP_SEEK_END)
			absolute_offset = files[index].info.total_size - offset;
			
		//check limits
		if(absolute_offset > files[index].info.total_size || absolute_offset < 0)
			return -1;
			
		//swap to new part
		unsigned wanted_part = absolute_offset / PART_SIZE;
		if(wanted_part != files[index].current_part)
			spfs_swap_part(index, wanted_part);

		//seek inside part
		sceIoLseek(files[index].fd, absolute_offset % PART_SIZE, SEEK_SET);
		files[index].offset = absolute_offset;
		
		return files[index].offset;
	};
	
	//regular file
	return sceIoLseek(fd, offset, whence);
};

int spfs_sceIoLseek32(SceUID fd, int offset, int whence)
{
	return (int)spfs_sceIoLseek(fd, (SceOff)offset, whence);
}; 

int spfs_sceIoRead(SceUID fd, void * data, unsigned size)
{
	//check if fd is a spfs entry
	if(SPFS_IS_SPFS(fd))
	{
		unsigned index = SPFS_UNMASK(fd);
		int total_read = 0;
	
		while(size)
		{
			//calculate how many you can/must read from current part
			unsigned bytes_left_for_part = (files[index].current_part + 1) * PART_SIZE - files[index].offset;
			unsigned amount_to_read = size < bytes_left_for_part? size: bytes_left_for_part;

			//read bytes
			int bytes_read = sceIoRead(files[index].fd, data, amount_to_read);
			
			total_read += bytes_read;
			files[index].offset += bytes_read;
			
			//in case of errors (read less than expected)
			if(bytes_read != amount_to_read)
				break;
			
			//load next part
			if(bytes_left_for_part == amount_to_read)
				spfs_swap_part(index, files[index].current_part + 1);
			
			//increase pointer and reduce the remaining size to read
			data += amount_to_read;
			size -= amount_to_read;
		};
	
		return total_read;
	};
	
	//regular file
	return sceIoRead(fd, data, size);
};

int spfs_sceIoWrite(SceUID fd, void * data, unsigned size)
{
	if(SPFS_IS_SPFS(fd))
	{
		//TODO: this
	
		return -1;
	};
	
	return sceIoWrite(fd, data, size);
};

int spfs_sceIoClose(SceUID fd)
{
	//check if fd is a spfs entry
	if(SPFS_IS_SPFS(fd))
	{
		unsigned index = SPFS_UNMASK(fd);
		files[index].used = 0;
		return sceIoClose(files[index].fd);
	};
	
	//regular file
	return sceIoClose(fd);
};

int spfs_sceIoRemove(const char * path)
{
	//get info from spfs main file
	spfs_info_t info;
	if(spfs_get_info(path, &info) > 0)
	{
		//remove all parts
		unsigned part = 0;
		
		for(int i = 0; i < info.part_count; i++)
		{
			char part_path[0x100];
			sprintf(part_path, "%s.part%03X", path, part);
			
			sceIoRemove(part_path);
			part++;
		};
	};
	
	//remove file
	return sceIoRemove(path);
};

int spfs_sceIoRename(const char * oldname, const char * newname)
{
	//get info from spfs main file
	spfs_info_t info;
	if(spfs_get_info(oldname, &info) > 0)
	{
		//rename all parts
		unsigned part = 0;
		
		for(int i = 0; i < info.part_count; i++)
		{
			char old_path_part[0x100];
			char new_path_part[0x100];
			sprintf(old_path_part, "%s.part%03X", oldname, part);
			sprintf(new_path_part, "%s.part%03X", newname, part);
			
			sceIoRename(old_path_part, new_path_part);
			part++;
		};
	};
	
	//rename file
	return sceIoRename(oldname, newname);
};

int spfs_sceIoGetstat(const char * path, SceIoStat * stat)
{
	//get status
	int ret = sceIoGetstat(path, stat);
	
	if(ret >= 0)
	{
		//get info from spfs main file
		spfs_info_t info;
		if(spfs_get_info(path, &info) > 0)
			stat->st_size = info.total_size; //fake size
	};
	
	return ret;
};

int spfs_sceIoDread(SceUID fd, SceIoDirent * dir)
{
	int ret = sceIoDread(fd, dir);
	
#ifdef SPFS_HIDE_PARTS

	//skip part files
	while(strstr(dir->d_name, ".part"))
		ret = sceIoDread(fd, dir);
		
#endif

	//TODO: fake dir->d_stat.st_size
	
	return ret;
};

unsigned spfs_debug_readu32(SceUID fd)
{
	unsigned read = 0xDEADBEEF;
	
	if(SPFS_IS_SPFS(fd))
	{
		unsigned index = SPFS_UNMASK(fd);
		sceIoRead(files[index].fd, &read, sizeof(unsigned));
		sceIoLseek(files[index].fd, -sizeof(unsigned), PSP_SEEK_CUR);
	};
	
	return read;
};