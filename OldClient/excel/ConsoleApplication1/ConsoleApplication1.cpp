// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>  
#include <fstream>  
using namespace std;
#include "../c++/Hero.pb.h"


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

bool get_newclient_skin(SHeroSkinDataPool& collection, int hero_id, int skin_id,SHeroSkin& output)
{
	for(int i=0;i<collection.data_size();i++)
	{
		SHeroSkin skin = collection.data(i);
		if(skin.nheroid() == hero_id && skin.nskinid() == skin_id)
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
	SHeroSkinDataPool skins;
	SHeroSkinDataPool skins_oldclient;
	skins.ParseFromIstream(&input);
	skins_oldclient.ParseFromIstream(&input_oldclient);

	//Update To NewClient Info(skill icon and other)
	for(int i =0; i< skins_oldclient.data_size();i++)
	{
		SHeroSkin* o_skin = skins_oldclient.mutable_data(i);
		SHeroSkin n_skin;
		//cout << o_skin->DebugString() << endl;
		if(get_newclient_skin(skins,o_skin->nheroid(),o_skin->nskinid(),n_skin))
		{
			if(n_skin.has_scmapskill()) o_skin->set_scmapskill(n_skin.scmapskill());
			if(n_skin.has_ssmapskill()) o_skin->set_ssmapskill(n_skin.ssmapskill());
			if(n_skin.has_smapotherappearance()) o_skin->set_smapotherappearance(n_skin.smapotherappearance());
			if(n_skin.has_sbischangevoice()) o_skin->set_sbischangevoice(n_skin.sbischangevoice());
			if(n_skin.has_schagenskillid()) o_skin->set_schagenskillid(n_skin.schagenskillid());
			if(n_skin.has_nskinlevel()) o_skin->set_nskinlevel(n_skin.nskinlevel());
			if(n_skin.has_schangebulleteffectid()) o_skin->set_schangebulleteffectid(n_skin.schangebulleteffectid());
			if(n_skin.has_schangestatuseffectid()) o_skin->set_schangestatuseffectid(n_skin.schangestatuseffectid());
			if(n_skin.has_schangevoiceeffect()) o_skin->set_schangevoiceeffect(n_skin.schangevoiceeffect());
			if(n_skin.has_umodelscale()) o_skin->set_umodelscale(n_skin.umodelscale());
			if(n_skin.has_uischangeicon()) o_skin->set_uischangeicon(n_skin.uischangeicon());
		}
	}

	//append new skins to old client skins end
	for(int i =skins_oldclient.data_size(); i < skins.data_size();i++)
	{
		SHeroSkin* add_skin = skins_oldclient.add_data();
		add_skin->CopyFrom(skins.data(i));

		//format to dds path
		char temp[256];
		strcpy_s(temp, add_skin->selectpic().c_str());
		char* pos = strstr(temp, "_.bmp");
		if (pos) {
			strcpy_s(pos, sizeof(temp) - (pos - (char *)&temp), ".dds");
		}else
		{
			continue;
		}

		add_skin->set_selectpic(temp);

		//update image
		add_skin->set_sbeginxy(convert_to_dds(add_skin->sbeginxy()));
		add_skin->set_sskinwideandhight(convert_to_dds(add_skin->sskinwideandhight()));
		add_skin->set_sbackgroundbeginwidehight(add_skin->sbeginxy());
		add_skin->set_sbackgroundendwidehight(add_skin->sskinwideandhight());
		
		//hero head
		if(add_skin->nheroid() == 142 && add_skin->nskinid() == 2)
		{
			add_skin->set_sheadbeginxy("294;57");
			add_skin->set_sheadwideandhight("144;144");
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