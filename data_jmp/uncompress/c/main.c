#include <stdio.h>
#include <string.h>
#include <string.h>
#include <memory.h>

#include "zlib.h"

#pragma comment(lib,"zlib.lib")

#pragma pack(1)

struct file_node_s
{
	char f_name[260];
	unsigned int position;
	unsigned int compr_size;
	unsigned int source_size;
	char f_reserved[0x20];
};

struct file_header_s
{
	char header[0x32];
	unsigned int f_count;
};

#pragma pack()


struct file_node_s* f_list = 0;
int f_count = 0;

int mkdir_r(char *path)
{
    int i = 0;
    int result;
    int length = strlen(path);
 
    //在末尾加/
    if (path[length - 1] != '\\' && path[length - 1] != '/')
    {
        path[length] = '/';
        path[length + 1] = '\0';
    }
 
    // 创建目录
    for (i = 0;i <= length;i ++)
    {
        if (path[i] == '\\' || path[i] == '/')
        { 
            path[i] = '\0';
 
            //如果不存在,创建
            result = access(path,0);
            if (result != 0)
            {
                result = mkdir(path);
                if (result != 0)
                {
                    return -1;
                } 
            }
            //支持linux,将所有\换成/
            path[i] = '/';
        } 
    }
    return 0;
}

char* dirname(const char* name){
	static char dir[260];

	memset(dir,0,sizeof(dir));
	char* p = strrchr(name,'\\');
	strncpy(dir,name,p - name);

	return dir;
}


void put_file(const char* path,void* buf,unsigned int length)
{
	char new_root[260];
	FILE* fd;

	printf("%s\n",path);

	char* pos = strstr(path,"\\") + 1;

	sprintf(new_root,"output\\%s",pos);

	mkdir_r(dirname(new_root));

	fd = fopen(new_root,"wb");
	if(fd){
		fwrite(buf,length,1,fd);
		fclose(fd);
	}
}

int main(int argc,char** argv)
{
	FILE* fd;
	int i;
	struct file_header_s header;
	void* compr_buf;
	void* source_buf;
	unsigned int source_out_len;



	fd = fopen("data.jmp","rb");
	if(fd){
		fread(&header,sizeof(struct file_header_s),1,fd);

		f_list = (struct file_node_s *)malloc(sizeof(struct file_node_s) * header.f_count);
		f_count = header.f_count;

		for(i = 0;i < f_count;i++){
			fread(&f_list[i],sizeof(struct file_node_s),1,fd);
		}

		for(i = 0;i < f_count;i++)
		{
			fseek(fd,f_list[i].position,SEEK_SET);

			compr_buf = malloc(f_list[i].compr_size);
			source_buf = malloc(f_list[i].source_size);

			source_out_len = f_list[i].source_size;

			if(f_list[i].source_size > 0){
				fread(compr_buf,f_list[i].compr_size,1,fd);

				if(uncompress(source_buf,&source_out_len,compr_buf,f_list[i].compr_size) != Z_OK){
					fprintf(stderr,"uncompress failed\n");
					getchar();
				}
				put_file(f_list[i].f_name,source_buf,source_out_len);
			}
			else
			{
				put_file(f_list[i].f_name,source_buf,source_out_len);
			}

			

			free(compr_buf);
			free(source_buf);
		}

		fclose(fd);
	}
	return 0;
}