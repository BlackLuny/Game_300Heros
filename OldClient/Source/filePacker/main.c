#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include "filesystem.h"


void add_file(const char *file)
{
    char newname[256];
    sprintf(newname,"..%s",file+7);
    fs_putfile(file,newname);
}

void enumfiles(char *path)
{
	struct _finddata_t filefind;
	char curr[260];
	char filename[260];
	sprintf(curr,"%s\\*.*",path);
	int done = 0, i, handle;
	if((handle = _findfirst(curr, &filefind)) != -1)
	{
		while(!(done = _findnext(handle, &filefind)))
		{
			if(strcmp(filefind.name, "..") == 0)
				continue;
			if((_A_SUBDIR == filefind.attrib))
			{
				sprintf(filename,"%s\\%s",path,filefind.name);
				enumfiles(filename);
			}
			else
			{
			    sprintf(filename,"%s\\%s",path,filefind.name);
			    add_file(filename);
			}
		}
		_findclose(handle);
	}
}


int main()
{
    printf("enum\n");
    fs_create("data.jmp");
    enumfiles("Z:\\data");
printf("pack...\n");
    fs_save();
    return 0;
}
