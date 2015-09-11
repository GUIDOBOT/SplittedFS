#ifndef SPFS_H
#define SPFS_H

#define PART_SIZE 	0x00100000
#define SPFS_MAGIC 	0x53465053

typedef struct
{
    unsigned magic;
    unsigned part_count;
	unsigned long long total_size;
} spfs_info_t;

#endif
