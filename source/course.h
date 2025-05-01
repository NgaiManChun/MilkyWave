#ifndef _COURSE_H
#define _COURSE_H

#include "gameObject.h"
#include "collision.h"
#include "progress.h"
using namespace MG;
using namespace MG::Collision;

class Course : public GameObject {
public:
	struct FACE {
		F3 v0;
		F3 v1;
		F3 v2;
		F3 normal;
	};
	struct SURFACE_ALIGN {
		float depth = 0.0f;
		F3 normal = { 0.0f, 1.0f, 0.0f };
		Quaternion rotate = Quaternion::Identity();
	};
	struct GRAVITY {
		F3 position;
		Quaternion rotate;
		F3 forward;
		F3 upper;
		float t;
	};
private:
	Model* surface;
	Animation* rail;
	std::list<FACE*> faces;
	std::map<const COLLISION_UNIT*, std::list<FACE*>> areaFaces;
	Progress surfaceFlow = Progress(10000.0f, true);

	void _AddFace(const MODEL& rawModel, const MODEL_NODE& node, const M4x4& world);
public:
	Course(Model* surface, Animation* rail, ARRANGEMENT* arrangement);
	~Course();
	void Update() override;
	void Draw() override;
	GRAVITY GetGravity(const F3& position) const;
	GRAVITY GetGravity(const float t) const;
	SURFACE_ALIGN GetSurfaceAlign(const F3& position, const Quaternion& gravityRotate) const;
};

#endif
