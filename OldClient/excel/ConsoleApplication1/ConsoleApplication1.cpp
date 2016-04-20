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
	y *= 0.96f;
	
	sprintf(ss,"%d;%d",x,y);
	
	return ss;

}

bool get_newclient_skin(heroskin_c& collection, int hero_id, int skin_id,heroskin& output)
{
	for(int i=0;i<collection.skins_size();i++)
	{
		heroskin skin = collection.skins(i);
		if(skin.hero_index() == hero_id && skin.skin_index() == skin_id)
		{
			output = skin;
			return true;
		}
	}

	return false;
}
int main(int args, char* argv[]) {
	fstream input("heroskin_c_newclient.dat", ios::in | ios::binary);
	fstream input_oldclient("heroskin_c_oldclient.dat", ios::in | ios::binary);
	fstream output("heroskin_c.dat", ios::out | ios::trunc | ios::binary);
	heroskin_c skins;
	heroskin_c skins_oldclient;
	skins.ParseFromIstream(&input);
	skins_oldclient.ParseFromIstream(&input_oldclient);

	//Update To NewClient Info(skill icon and other)
	for(int i =0; i< skins_oldclient.skins_size();i++)
	{
		heroskin* o_skin = skins_oldclient.mutable_skins(i);
		heroskin n_skin;
		//cout << o_skin->DebugString() << endl;
		if(get_newclient_skin(skins,o_skin->hero_index(),o_skin->skin_index(),n_skin))
		{
			if(n_skin.has_str3()) o_skin->set_str3(n_skin.str3());
			if(n_skin.has_str4()) o_skin->set_str4(n_skin.str4());
			if(n_skin.has_str5()) o_skin->set_str5(n_skin.str5());
			if(n_skin.has_str6()) o_skin->set_str6(n_skin.str6());
			if(n_skin.has_str7()) o_skin->set_str7(n_skin.str7());
			if(n_skin.has_unk()) o_skin->set_unk(n_skin.unk());
		}
	}

	//append new skins to old client skins end
	for(int i =skins_oldclient.skins_size(); i < skins.skins_size();i++)
	{
		heroskin* add_skin = skins_oldclient.add_skins();
		add_skin->CopyFrom(skins.skins(i));

		//format to dds path
		char temp[256];
		strcpy_s(temp, add_skin->resource().c_str());
		char* pos = strstr(temp, "_.png");
		if (pos) {
			strcpy_s(pos, sizeof(temp) - (pos - (char *)&temp), ".dds");
		}else
		{
			continue;
		}

		add_skin->set_resource(temp);

		//update image
		add_skin->set_select_card_pos(convert_to_dds(add_skin->select_card_pos()));
		add_skin->set_select_card_wh(convert_to_dds(add_skin->select_card_wh()));
		add_skin->set_shop_card_pos(add_skin->select_card_pos());
		add_skin->set_shop_card_wh(add_skin->select_card_wh());
		
		//hero head
		if(add_skin->hero_index() == 194 && add_skin->skin_index() == 1)
		{
			add_skin->set_head_box_pos("324;89");
			add_skin->set_head_box_wh("116;116");
		}
		

		cout << add_skin->DebugString() << endl;
	}


	if (!skins_oldclient.SerializeToOstream(&output)) {
		cerr << "Failed to write msg." << endl;
		return -1;
	}

	cout << "Write to file: " << "heroskin_c.dat" << " successfully" << endl; 

	output.flush();

}