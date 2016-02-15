// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>  
#include <fstream>  
using namespace std;
#include "heroskin.pb.h"



int main(int args, char* argv[]) {
	fstream input("heroskin_c_new.dat", ios::in | ios::binary);
	fstream input_old("heroskin_c_old.dat", ios::in | ios::binary);
	fstream output("heroskin_c.dat", ios::out | ios::trunc | ios::binary);
	heroskin_c skins, skins_old;
	skins.ParseFromIstream(&input);
	skins_old.ParseFromIstream(&input_old);


	for (int i = skins_old.skins_size(); i < skins.skins_size(); i++) {
		heroskin* skin = skins_old.add_skins();

		char temp[256];
		strcpy_s(temp, skins.skins(i).resource().c_str());
		char* pos = strstr(temp, "_.png");
		if (pos) {
			strcpy_s(pos, sizeof(temp) - (pos - (char *)&temp), ".dds");
		}

		skins.mutable_skins(i)->set_resource(temp);

		skin->CopyFrom(skins.skins(i));

		
		cout << skin->DebugString() << endl;

		if (skin->hero_index() == 183)
		{
			skin->set_box_pos("290;85");
			skin->set_box_wh("83;82");
			skin->set_head_img_pos("259;45");
			skin->set_head_img_wh("275;490");
			skin->set_card_pos("259;45");
			skin->set_card_wh("275;490");
		}

		if (skin->hero_index() == 159)
		{
			skin->set_box_pos("351;72");
			skin->set_box_wh("128;128");
			skin->set_head_img_pos("250;30");
			skin->set_head_img_wh("300;500");
			skin->set_card_pos("250;30");
			skin->set_card_wh("300;500");
		}

		if (skin->hero_index() == 53)
		{
			skin->set_box_pos("296;124");
			skin->set_box_wh("133;132");
		}

		if (skin->hero_index() == 189)
		{
			skin->set_box_pos("269;87");
			skin->set_box_wh("112;112");
			skin->set_head_img_pos("200;54");
			skin->set_head_img_wh("300;522");
			skin->set_card_pos("200;54");
			skin->set_card_wh("300;522");
		}

		if (skin->hero_index() == 167)
		{
			skin->set_box_pos("342;56");
			skin->set_box_wh("95;95");
			skin->set_head_img_pos("185;17");
			skin->set_head_img_wh("338;547");
			skin->set_card_pos("185;17");
			skin->set_card_wh("338;547");
		}

		//cout << skin->hero_index() << endl;

		//cout << skin->resource() << endl;
	}

	if (!skins_old.SerializeToOstream(&output)) {
		cerr << "Failed to write msg." << endl;
		return -1;
	}

	cout << "Write to file: " << "test.dat" << " successfully" << endl; 

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