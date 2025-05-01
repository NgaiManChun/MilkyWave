// =======================================================
// MGObject.cpp
// 
// �t�@�C���o�͗p�\����
// 
// ��ҁF鰕��r�i�K�C�@�}���`�����j�@2024/11/09
// =======================================================
#include "MGObject.h"
#include "MGDataType.h"
#include <fstream>

namespace MG {

	
	void MGObject::Release()
	{
		delete data;
		data = nullptr;
		size = 0;
	}

	MGObject LoadMGO(const char* fileName) {
		std::ifstream file(fileName, std::ios::binary);

		MGObject mgo{};
		file.read(reinterpret_cast<char*>(&mgo), sizeof(MGObject));

		char* data = new char[mgo.size];
		file.read(data, mgo.size);
		mgo.data = data;

		file.close();

		return mgo;
	}
}

