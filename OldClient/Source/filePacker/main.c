#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include "filesystem.h"

char find_path[256];
int find_strsize;
void add_file(const char *file)
{
    char newname[512];
    sprintf(newname,"..%s",file+find_strsize);
    fs_putfile(file,newname);
}

void enumfiles(char *path)
{
	struct _finddata_t filefind;
	char curr[512];
	char filename[512];
	sprintf(curr,"%s\\*.*",path);
	int done = 0, handle;
	if((handle = _findfirst(curr, &filefind)) != -1)
	{
		while(!(done = _findnext(handle, &filefind)))
		{
			if(strcmp(filefind.name, "..") == 0 || strcmp(filefind.name, ".") == 0)
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
    getcwd(find_path,sizeof(find_path));
    strcat(find_path,"\\output");
    find_strsize = strlen(find_path);
    fs_create("data.jmp");
    printf("enum files.....\n");
    enumfiles(find_path);
    fs_save();
    return 0;
}
