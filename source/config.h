#ifndef _CONFIG_H
#define _CONFIG_H

#include <string>
#include "CSVResource.h"

#define TABLE_FLOAT_VALUE(table, key, default_value) \
((table.count(key)) ? std::stof(table.at(key).at("value")) : default_value)

#define TABLE_INT_VALUE(table, key, default_value) \
((table.count(key)) ? std::stoi(table.at(key).at("value")) : default_value)

#define TABLE_STR_VALUE(table, key, default_value) \
((table.count(key)) ? table.at(key).at("value") : default_value)

template<typename T>
static T LoadConfig(const char* file, T(*function)(const D_KVTABLE& table))
{
	// CSV�t�@�C������ݒ��ǂݍ���
	D_TABLE table;
	D_KVTABLE keyValuePair;
	ReadCSVFromPath(file, table);

	// Table�f�[�^����Key-Value-Pair�֕ϊ�
	TableToKeyValuePair("key", table, keyValuePair);

	return function(keyValuePair);
}

#endif