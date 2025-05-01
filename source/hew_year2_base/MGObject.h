// =======================================================
// MGObject.h
// 
// �t�@�C���o�͗p�\����
// 
// ��ҁF鰕��r�i�K�C�@�}���`�����j�@2024/11/09
// =======================================================
#ifndef _MG_OBJECT_H
#define _MG_OBJECT_H

namespace MG {

	enum MGOBJECT_TYPE {
		MGOBJECT_TYPE_MODEL,
		MGOBJECT_TYPE_ANIMATION,
		MGOBJECT_TYPE_ARRANGEMENT,
		MGOBJECT_TYPE_DATA
	};

	class MGObject {
	public:
		MGOBJECT_TYPE type;
		size_t size;
		char* data;
		void Release();
	};

	MGObject LoadMGO(const char* fileName);

} // namespace MG

#endif