// ZlibTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "zlib.h"
#include <stdio.h>
#include <stdlib.h>


int main()
{
	FILE* fp;
	long len;
	unsigned long tgtlen;
	
	unsigned char* buf;
	unsigned char* buff;



	if (fopen_s(&fp, "Z:\\test.dat", "rb") == 0)
	{
		fseek(fp, 0, SEEK_END);
		len = ftell(fp);
		buf = (unsigned char*)malloc(len);
		fseek(fp, 0, SEEK_SET);
		fread(buf, len, 1, fp);
		fclose(fp);

		tgtlen = 17418109;//compressBound(len);
		buff = new unsigned char[tgtlen];
		if (uncompress(buff, &tgtlen, buf, len) == Z_OK)
		{
			fopen_s(&fp, "z:\\tem2p.data", "wb");
			fwrite(buff, tgtlen, 1, fp);
			fclose(fp);
		}
	}




    return 0;
}

