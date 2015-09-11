//SPFS Splitter by GUIDOBOT

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include "../spfs/spfs.h"

int process_file(const char * path)
{
    //try to open file
    FILE * input = fopen(path, "rb");
    if(!input)
        return 0;

	//get file size
	fseek(input, 0x0, SEEK_END);
	unsigned long long total_size = ftell(input);
	
	//check if there is no need to split the file
	if(total_size <= PART_SIZE)
	{
		fclose(input);
		return 3;
	};
	
    printf("Processing %s\n", path);

    //split into parts
    unsigned part = 0;
    char data[PART_SIZE];
	
    //split file into parts
    fseek(input, 0x0, SEEK_SET);
    while(!feof(input))
    {
        //create the name for this part file
        char part_name[0x1000];
        sprintf(part_name, "%s.part%03X", path, part);

        //open output file
        FILE * output = fopen(part_name, "wb");

        //read from input and write to output
        unsigned bytes = fread(data, 1, sizeof(data), input);
        fwrite(data, bytes, 1, output);

        //close output file
        fclose(output);

        part++;
    };

    fclose(input);

    //generate main file info
    spfs_info_t header = {
        SPFS_MAGIC,
        part,
		total_size
    };

    //create main file
    FILE * main = fopen(path, "wb");
    fwrite(&header, sizeof(spfs_info_t), 1, main);
    fclose(main);

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
        //skip current, parent and this file
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
