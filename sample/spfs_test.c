//Splitted files system for ePSP by GUIDOBOT
//sample

#include <pspkernel.h>
#include <pspdebug.h>
#include "../spfs/spfs_hooks.h"
#define SPLITTED_FILE "ms0:/test/splitted_file.dat"

PSP_MODULE_INFO("SPFS_TESTS", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);

void test_seek(SceUID fd, SceOff value, int whence)
{
	SceOff offset = spfs_sceIoLseek(fd, value, whence);
	
	unsigned data = spfs_debug_readu32(fd);
	pspDebugScreenPrintf("seek(%08llx, %d): %llX -> %08X\n", value, whence, offset, data);
};

int main(int argc, char ** argv)
{
	//Start debug screen
	pspDebugScreenInit();

	//Start SPFS system
	pspDebugScreenPrintf("Starting SPFS\n");
	if(spfs_start() >= 0)
	{
		pspDebugScreenPrintf("SPFS Started\n");
	
		//list files in folder
		pspDebugScreenPrintf("Listing ms0:/test\n");
		SceUID directory = sceIoDopen("ms0:/test/");
		if(directory >= 0)
		{
			SceIoDirent entry;
			while(spfs_sceIoDread(directory, &entry) > 0)
				pspDebugScreenPrintf("%s\n", entry.d_name);
			
			sceIoDclose(directory);
		};
	
		//get splitted file info
		pspDebugScreenPrintf("Calling sceIoGetStat()\n");
		SceIoStat status;

		if(spfs_sceIoGetstat(SPLITTED_FILE, &status) >= 0)
			pspDebugScreenPrintf("File Size %llX\n", status.st_size);

		//open splitted file
		pspDebugScreenPrintf("Calling sceIoOpen()\n");
		SceUID fd = spfs_sceIoOpen(SPLITTED_FILE, PSP_O_RDONLY, 0777);
		if(fd >= 0)
		{
			pspDebugScreenPrintf("Opened, returned %08X\n", fd);
			
			//seek tests
			pspDebugScreenPrintf("Testing seeks\n");
			test_seek(fd, 0x00000004, PSP_SEEK_SET); //y
			test_seek(fd, 0x00200000, PSP_SEEK_SET); //y
			test_seek(fd, 0x00000000, PSP_SEEK_SET); //y
			test_seek(fd, 0x00000100, PSP_SEEK_CUR);
			test_seek(fd, -16, PSP_SEEK_CUR);
			test_seek(fd, 0x00000100, PSP_SEEK_END);
			test_seek(fd, 0x00100000, PSP_SEEK_END);
			
			/*
			//copy file
			char buffer[0x3E5400];
			int bytes_read = spfs_sceIoRead(fd, buffer, sizeof(buffer));
			
			pspDebugScreenPrintf("Read %08X bytes\n", bytes_read);
			SceUID output = sceIoOpen("ms0:/test/out.dat", PSP_O_WRONLY | PSP_O_TRUNC | PSP_O_CREAT, 0777);
			sceIoWrite(output, buffer, bytes_read);
			sceIoClose(output);
			*/
		};
		
		//close file
		pspDebugScreenPrintf("Calling sceIoClose()\n");
		spfs_sceIoClose(fd);
		
		/*
		//rename file
		pspDebugScreenPrintf("Calling sceIoRename()\n");
		if(spfs_sceIoRename(SPLITTED_FILE, "ms0:/test/renamed.dat") >= 0)
			pspDebugScreenPrintf("Renamed\n");
	
		//delete file
		pspDebugScreenPrintf("Calling sceIoRemove()\n");
		if(spfs_sceIoRemove("ms0:/test/renamed.dat") >= 0)
			pspDebugScreenPrintf("Removed\n");
		*/
	};

	sceKernelSleepThread();
	sceKernelExitGame();
	return 1;
};