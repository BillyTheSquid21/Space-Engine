#pragma once
#ifndef ITEMS_HPP
#define ITEMS_HPP

#include <stdint.h>
#include <string>
#include <array>

#define MAX_ITEM_TYPES 1329

//Struct that defines an item for storage
//Functions that allow items to be used will be on a state by state basis
struct Item
{
	uint16_t id = 0;		//Zero is a null item
	uint8_t pocketId = 0;	//Pocket item resides in
	uint8_t categoryId = 0;	//Type of item
	std::string name = "debugItem";
};

struct ItemSlot
{
	Item item;
	uint16_t count = 0;
};

//Each item has a defined slot based on its ID, and category
struct PlayerBag
{
	std::array<ItemSlot, MAX_ITEM_TYPES> items;
};

#endif