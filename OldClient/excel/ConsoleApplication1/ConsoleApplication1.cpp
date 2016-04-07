// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>  
#include <fstream>  
using namespace std;
#include "heroskin.pb.h"

string convert_to_dds(string org)
{
	int x,y;
	char ss[32];
	
	sscanf(org.c_str(),"%d;%d",&x,&y);
	x *= 0.8f;
	y *= 0.8f;
	
	sprintf(ss,"%d;%d",x,y);
	
	return ss;

}

int main(int args, char* argv[]) {
	fstream input("heroskin_c_newclient.dat", ios::in | ios::binary);
	fstream input_oldclient("heroskin_c_oldclient.dat", ios::in | ios::binary);
	fstream output("heroskin_c.dat", ios::out | ios::trunc | ios::binary);
	heroskin_c skins;
	heroskin_c skins_oldclient;
	skins.ParseFromIstream(&input);
	skins_oldclient.ParseFromIstream(&input_oldclient);

	for (int i = skins_oldclient.skins_size(); i < skins.skins_size(); i++) {

		heroskin* skin = skins_oldclient.add_skins();
		skin->CopyFrom(skins.skins(i));

		char temp[256];
		strcpy_s(temp, skin->resource().c_str());
		char* pos = strstr(temp, "_.png");
		if (pos) {
			strcpy_s(pos, sizeof(temp) - (pos - (char *)&temp), ".dds");
		}else
		{
			continue;
		}

		skin->set_resource(temp);
		
		cout << skin->DebugString() << endl;
		//363 60  105 105

		skin->set_select_card_pos(convert_to_dds(skin->select_card_pos()));
		skin->set_select_card_wh(convert_to_dds(skin->select_card_wh()));
		skin->set_head_box_pos(convert_to_dds(skin->head_box_pos()));
		skin->set_head_box_wh(convert_to_dds(skin->head_box_wh()));
		skin->set_shop_card_pos(skin->select_card_pos());
		skin->set_shop_card_wh(skin->select_card_wh());
	}

	for(int i=0;i<skins_oldclient.skins_size();i++)
	{
		heroskin* skin = skins_oldclient.mutable_skins(i);
		if(skin->hero_index() == 192)
		{
			skin->set_head_box_pos("363;60");
			skin->set_head_box_wh("105;105");
		}
	}

	if (!skins_oldclient.SerializeToOstream(&output)) {
		cerr << "Failed to write msg." << endl;
		return -1;
	}

	cout << "Write to file: " << "heroskin_c.dat" << " successfully" << endl; 

	output.flush();

	/*heroskin_c skins;
	fstream input("heroskin_c.dat", ios::in | ios::binary);
	skins.ParseFromIstream(&input);
	printf("%d\n", skins.skins_size());
	for (int i = 0; i < skins.skins_size(); i++)
	{
		char temp[256];
		strcpy_s(temp, skins.skins(i).resource().c_str());
		char* pos = strstr(temp, "_.png");
		if (pos) {
			strcpy_s(pos, sizeof(temp)-(pos - (char *)&temp) , ".dds");
		}

		skins.mutable_skins(i)->set_resource(temp);

		cout << skins.skins(i).resource() << endl;
	}

	fstream output("test.dat", ios::out | ios::trunc | ios::binary);

	if (!skins.SerializeToOstream(&output)) {
		cerr << "Failed to write msg." << endl;
		return -1;
	}

	cout << "Write to file: " << "test.dat" << " successfully" << endl;*/
}