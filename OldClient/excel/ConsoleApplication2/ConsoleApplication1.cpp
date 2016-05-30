// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>  
#include <fstream>  
#include <set>

using namespace std;
#include "../c++/Item.pb.h"

SItem_Item hero_tpl;

void add_hero(SItem_ItemDataPool& heros,int hero_id,std::string hero_ico,std::string heroName)
{
	SItem_Item *new_hero = heros.add_data();
	new_hero->CopyFrom(hero_tpl);

	new_hero->set_nid(hero_id);
	new_hero->set_striconfile(hero_ico);
	new_hero->set_stritemname(heroName);

	cout << new_hero->DebugString() << endl;
}



int main(int args, char* argv[]) {
	fstream input("item_item_c_n.dat", ios::in | ios::binary);
	fstream input_merge("item_item_c_o.dat", ios::in | ios::binary);
	fstream output("item_item_c.dat", ios::out | ios::binary | ios::trunc);
	SItem_ItemDataPool item_c_n;
	SItem_ItemDataPool item_c_o;
	item_c_n.ParseFromIstream(&input);
	item_c_o.ParseFromIstream(&input_merge);

	std::set<int> arrays;
	std::set<string> new_veri_items;


	for(int i=0;i<item_c_n.data_size();i++)
	{
		char temp[128];
		SItem_Item item = item_c_n.data(i);

		sprintf(temp,"%d_%s",item.nid(),item.stritemtype().c_str());

		if(item.has_stritemname() && item.nid() == 5292)
		{
			char debug_name[128] = {0};
			strncpy(debug_name,item.stritemname().c_str(),item.stritemname().length());
			cout << "itemName:" << debug_name << endl;
		}
		
		new_veri_items.insert(temp);
	}


	for(int i=0;i<item_c_o.data_size();i++)
	{
		char temp[128];
		SItem_Item item = item_c_o.data(i);

		sprintf(temp,"%d_%s",item.nid(),item.stritemtype().c_str());

		if(new_veri_items.find(temp) == new_veri_items.end())
		{
			//if(item.has_name())
			//{
				char debug_name[128] = {0};
				strncpy(debug_name,item.stritemname().c_str(),item.stritemname().length());
				cout << "Add => itemName:" << debug_name << ":" <<item.nid() << endl;

				SItem_Item* pp = item_c_n.add_data();
				pp->CopyFrom(item);
				

				//ÃÌº””¢–€≤ø∑÷
				if(item.nid() == 5291)
				{
					//cout << pp->DebugString() << endl;
					hero_tpl = item;
				}
			//}
		}
	}

	//add_hero(item_c_n,5295,"\\UI\\Head\\role\\chara_0195.dds","õgÃÔ∏Ÿº™");

	if (!item_c_n.SerializeToOstream(&output)) {
		cerr << "Failed to write msg." << endl;
		return -1;
	}

	cout << "Write to file: " << "item_item_c.dat" << " successfully" << endl; 

	output.flush();
}