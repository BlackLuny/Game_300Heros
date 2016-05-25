// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>  
#include <fstream>  
#include <set>

using namespace std;
#include "../c++/item_item_c.pb.h"

item_item hero_tpl;

void add_hero(item_item_c& heros,int hero_id,std::string hero_ico,std::string heroName)
{
	item_item *new_hero = heros.add_items();
	new_hero->CopyFrom(hero_tpl);

	new_hero->set_item_id(hero_id);
	new_hero->set_ico(hero_ico);
	new_hero->set_name(heroName);

	cout << new_hero->DebugString() << endl;
}



int main(int args, char* argv[]) {
	fstream input("item_item_c_n.dat", ios::in | ios::binary);
	fstream input_merge("item_item_c_o.dat", ios::in | ios::binary);
	fstream output("item_item_c.dat", ios::out | ios::binary | ios::trunc);
	item_item_c item_c_n;
	item_item_c item_c_o;
	item_c_n.ParseFromIstream(&input);
	item_c_o.ParseFromIstream(&input_merge);

	std::set<int> arrays;
	std::set<string> new_veri_items;


	for(int i=0;i<item_c_n.items_size();i++)
	{
		if(item_c_n.items(i).item_type() == "skill")
		{
			if(item_c_n.items(i).name().find("誓约胜利之剑") != string::npos)
			{
				cout << item_c_n.items(i).name() << endl;
				cout << item_c_n.items(i).DebugString() << endl;
			}

		}
	}

	for(int i=0;i<item_c_o.items_size();i++)
	{
		if(item_c_o.items(i).item_type() == "skill")
		{
			if(item_c_o.items(i).name().find("誓约胜利之剑") != string::npos)
			{
				cout << item_c_o.items(i).name() << endl;
				cout << item_c_o.items(i).DebugString() << endl;
			}

		}
	}


#if 0

	for(int i=0;i<item_c_n.items_size();i++)
	{
		char temp[128];
		item_item item = item_c_n.items(i);

		sprintf(temp,"%d_%s",item.item_id(),item.item_type().c_str());

		if(item.has_name() && item.item_id() == 5292)
		{
			char debug_name[128] = {0};
			strncpy(debug_name,item.name().c_str(),item.name().length());
			cout << "itemName:" << debug_name << endl;
		}
		
		new_veri_items.insert(temp);
	}


	for(int i=0;i<item_c_o.items_size();i++)
	{
		char temp[128];
		item_item item = item_c_o.items(i);

		sprintf(temp,"%d_%s",item.item_id(),item.item_type().c_str());

		if(new_veri_items.find(temp) == new_veri_items.end())
		{
			//if(item.has_name())
			//{
				char debug_name[128] = {0};
				strncpy(debug_name,item.name().c_str(),item.name().length());
				cout << "Add => itemName:" << debug_name << ":" <<item.item_id() << endl;

				item_item* pp = item_c_n.add_items();
				pp->CopyFrom(item);
				

				//添加英雄部分
				if(item.item_id() == 5291)
				{
					//cout << pp->DebugString() << endl;
					hero_tpl = item;
				}
			//}
		}
	}

	//add_hero(item_c_n,5295,"\\UI\\Head\\role\\chara_0195.dds","沢田纲吉");

	if (!item_c_n.SerializeToOstream(&output)) {
		cerr << "Failed to write msg." << endl;
		return -1;
	}

	cout << "Write to file: " << "item_item_c.dat" << " successfully" << endl; 

	output.flush();
#endif
}