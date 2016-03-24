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
	fstream input_old("item_item_c_o.dat", ios::in | ios::binary);
	fstream output("item_item_c.dat", ios::out | ios::binary | ios::trunc);
	item_item_c item_c_n;
	item_item_c item_c_o;
	item_c_n.ParseFromIstream(&input);
	item_c_o.ParseFromIstream(&input_old);

	std::set<int> arrays;
	

	for(int i=0;i<item_c_n.items_size();i++)
	{
		arrays.insert(item_c_n.items(i).item_id());
	}

	for(int i=0;i<item_c_o.items_size();i++)
	{
		if(arrays.find(item_c_o.items(i).item_id()) == arrays.end()){
			item_item* it = item_c_n.add_items();
			it->CopyFrom(item_c_o.items(i));
		}
	}



	for(int i=0;i<item_c_n.items_size();i++)
	{
		if(item_c_n.items(i).item_id() == 5291){
			hero_tpl.CopyFrom(item_c_n.items(i));
			
		}
	}

	add_hero(item_c_n,5292,"\\UI\\Head\\role\\chara_0192.dds","ÄÁäþºìÀòÆÜ");

	if (!item_c_n.SerializeToOstream(&output)) {
		cerr << "Failed to write msg." << endl;
		return -1;
	}

	cout << "Write to file: " << "item_item_c.dat" << " successfully" << endl; 

	output.flush();
	
}