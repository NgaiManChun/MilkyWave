#include "course.h"

void Course::_AddFace(const MODEL& rawModel, const MODEL_NODE& node, const M4x4& world)
{
	M4x4 nodeWorld =
		M4x4::ScalingMatrix(node.scale) *
		M4x4::RotatingMatrix(node.rotate) *
		M4x4::TranslatingMatrix(node.position) * world;	

	for (unsigned int i = 0; i < node.meshNum; i++) {
		unsigned int meshIndex = node.meshIndexes[i];
		MESH& mesh = rawModel.meshes[meshIndex];
		if (mesh.primitiveType != PRIMITIVE_TYPE_TRIANGLE) continue;
		unsigned int faceNum = mesh.vertexIndexNum / 3;
		for (unsigned int f = 0; f < faceNum; f++) {
			FACE* face = new FACE{
				mesh.vertices[mesh.vertexIndexes[f * 3 + 0]].position,
				mesh.vertices[mesh.vertexIndexes[f * 3 + 1]].position,
				mesh.vertices[mesh.vertexIndexes[f * 3 + 2]].position,
				{},
			};

			face->v0 = nodeWorld * face->v0;
			face->v1 = nodeWorld * face->v1;
			face->v2 = nodeWorld * face->v2;

			F3 center = (face->v0 + face->v1 + face->v2) / 3.0f;
			float disSq0 = DistanceSquare(center, face->v0);
			float disSq1 = DistanceSquare(center, face->v1);
			float disSq2 = DistanceSquare(center, face->v2);
			float maxDisSq = std::max(std::max(disSq0, disSq1), disSq2);

			float r = sqrtf(maxDisSq) / 2.0f;
			faces.push_back(face);
			Collision::Sphere collsion(M4x4::ScalingMatrix({ r, r, r }) * M4x4::TranslatingMatrix(center));

			for (const GameObject::COLLISION_UNIT& collisionUnit : GetCollisionUnits()) {
				if (collisionUnit.world->Overlap(collsion)) {
					areaFaces[&collisionUnit].push_back(face);
				}
			}
		}
	}

	for (int i = 0; i < node.childrenNum; i++) {
		_AddFace(rawModel, node.children[i], nodeWorld);
	}
}

Course::Course(Model* surface, Animation* rail, ARRANGEMENT* arrangement) : surface(surface), rail(rail)
{
	AddCollisionUnits(arrangement);
	UpdateWorldCollisionUnits();
	if (surface) {
		_AddFace(*surface->rawModel, *surface->rawModel->rootNode, M4x4::TranslatingMatrix({}));
	}
}

Course::~Course()
{
	for (auto face : faces) {
		delete face;
	}
	faces.clear();
	areaFaces.clear();
}

void Course::Update()
{
	surfaceFlow.IncreaseValue(GetDeltaTime());
	UpdateWorldCollisionUnits();
}

void Course::Draw()
{
	DrawModel(surface, position, size, rotate, color, { 0.0f, surfaceFlow });
}

Course::GRAVITY Course::GetGravity(const F3& position) const
{
	ANIMATION_CHANNEL* railCh = rail->modelNodeChannels["rail"];
	float minDistance = -1.0f;
	unsigned int nearsetPointIndex = 0;
	for (unsigned int i = 0; i < railCh->positionKeyNum; i++) {
		float distance = DistanceSquare(position, railCh->positionKeys[i].vector);
		if (minDistance < 0.0f || distance < minDistance) {
			nearsetPointIndex = i;
			minDistance = distance;
		}
	}
	GRAVITY gravity = { 
		railCh->positionKeys[nearsetPointIndex].vector, 
		railCh->rotationKeys[nearsetPointIndex].rotate, 
		Rotate({ 0.0f, 0.0f, 1.0f }, railCh->rotationKeys[nearsetPointIndex].rotate),
		Rotate({ 0.0f, 1.0f, 0.0f }, railCh->rotationKeys[nearsetPointIndex].rotate),
		(float)nearsetPointIndex / railCh->positionKeyNum
	};

	if (nearsetPointIndex > 0 && nearsetPointIndex < railCh->positionKeyNum) {
		float previousDistance = DistanceSquare(position, railCh->positionKeys[nearsetPointIndex - 1].vector);
		float nextDistance = DistanceSquare(position, railCh->positionKeys[nearsetPointIndex + 1].vector);
		if (previousDistance < nextDistance) {
			// ÅŠñ‚è“_’´‚¦‚Ä‚È‚¢
			float t = minDistance / (minDistance + previousDistance);
			gravity.position = gravity.position * (1.0f - t) + railCh->positionKeys[nearsetPointIndex - 1].vector * t;

			Quaternion nextRotate = railCh->rotationKeys[nearsetPointIndex - 1].rotate;
			F3 forward = Lerp(gravity.forward, Rotate({ 0.0f, 0.0f, 1.0f }, nextRotate), t);
			F3 upper = Lerp(gravity.upper, Rotate({ 0.0f, 1.0f, 0.0f }, nextRotate), t);
			gravity.rotate = Quaternion(forward, upper);

			gravity.t = (nearsetPointIndex - t) / railCh->positionKeyNum;
		}
		else {
			// ÅŠñ‚è“_’´‚¦‚½
			float t = minDistance / (minDistance + nextDistance);
			gravity.position = gravity.position * (1.0f - t) + railCh->positionKeys[nearsetPointIndex + 1].vector * t;

			Quaternion nextRotate = railCh->rotationKeys[nearsetPointIndex + 1].rotate;
			F3 forward = Lerp(gravity.forward, Rotate({ 0.0f, 0.0f, 1.0f }, nextRotate), t);
			F3 upper = Lerp(gravity.upper, Rotate({ 0.0f, 1.0f, 0.0f }, nextRotate), t);
			gravity.rotate = Quaternion(forward, upper);

			gravity.t = (nearsetPointIndex + t) / railCh->positionKeyNum;
		}
	}

	return gravity;
}

Course::GRAVITY Course::GetGravity(const float t) const
{
	F3 size, position;
	Quaternion rotate;
	rail->Apply("rail", rail->rawAnimation->frames * t, size, position, rotate);
	return {
		position,
		rotate,
		Rotate({ 0.0f, 0.0f, 1.0f }, rotate),
		Rotate({ 0.0f, 1.0f, 0.0f }, rotate),
		t
	};
}

Course::SURFACE_ALIGN Course::GetSurfaceAlign(const F3& position, const Quaternion& gravityRotate) const
{
	Quaternion gravityInverse = gravityRotate.Inverse();
	for (const auto& area : areaFaces) {
		if (area.first->world->Overlap(position)) {
			for (const auto face : area.second) {

				F3 p0 = Rotate(face->v0 - position, gravityInverse);
				F3 p1 = Rotate(face->v1 - position, gravityInverse);
				F3 p2 = Rotate(face->v2 - position, gravityInverse);

				float s0 = p0.x * p1.z - p0.z * p1.x;
				float s1 = p1.x * p2.z - p1.z * p2.x;
				float s2 = p2.x * p0.z - p2.z * p0.x;
				if((s0 > 0 && s1 > 0 && s2 > 0) || (s0 < 0 && s1 < 0 && s2 < 0)){
					F3 normal = Normalize(Cross(p1 - p0, p2 - p0));
					return {
							((Dot(p0, normal) > 0) ? 1.0f : -1.0f) * fabsf((normal.x * p0.x + normal.z * p0.z) / normal.y + p0.y),
							normal,
							gravityRotate * Quaternion::Quaternion(Normalize(Cross(normal, { -1.0f, 0.0f, 0.0f })), normal)
					};
				}
			}
		}
	}

	return {
		0.0f,
		{ 0.0f, 1.0f, 0.0f },
		Quaternion::Identity()
	};
}
