//SPFS Joiner by GUIDOBOT

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include "../spfs/spfs.h"

int process_file(const char * path)
{
	//try to open file
    FILE * output = fopen(path, "rb");
    if(!output)
        return 0;

    //check if file is a spfs main file
    unsigned magic = 0;
    fread(&magic, sizeof(unsigned), 1, output);
    if(magic != SPFS_MAGIC)
    {
        fclose(output);
        return 2;
    };

	//read info
	spfs_info_t header;
	fseek(output, 0x0, SEEK_SET);
	fread(&header, sizeof(spfs_info_t), 1, output);
	fclose(output);

	//change to write mode
	output = fopen(path, "wb");
	for(unsigned i = 0; i < header.part_count; i++)
	{
		//generate the part path
		char part_path[0x100];
		sprintf(part_path, "%s.part%03X", path, i);

		//open file and read
		char data[PART_SIZE];
		FILE * input = fopen(part_path, "rb");
		unsigned bytes = fread(data, 1, sizeof(data), input);
		fclose(input);

        //remove this part
        remove(part_path);

		//write to final file
		fwrite(data, bytes, 1, output);
	};

	fclose(output);
	return 1;
};

int process_directory(const char * path)
{
    printf("Processing %s\n", path);

    //try to open directory
    DIR * directory = opendir(path);
    if(!directory)
        return 0;

    dirent * entry;
    while((entry = readdir(directory)) > 0)
    {
        //skip current, parent, this file and parts
        if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..") || strstr(entry->d_name, ".part"))
            continue;

        //create full path for the entry
        char * fullpath = new char[strlen(path) + entry->d_namlen + 2];
        strcpy(fullpath, path);
        strcat(fullpath, entry->d_name);

        //process files
        if(!process_file(fullpath))
        {
            //process subdirs
            strcat(fullpath, "/");
            process_directory(fullpath);
        };

        delete [] fullpath;
    };

    //close directory
    closedir(directory);
    return 1;
};

int main(int argc, char ** argv)
{
    //use the current path as default
    char * path = argv[0];

    //use the path in the second arg if it was provided
    if(argc > 1)
        path = argv[1];

    //fix path string
    char * slash = strrchr(path, '/');
    if(!slash)
        slash = strrchr(path, '\\');

    slash[1] = 0x0;

    return process_directory(path);
};
