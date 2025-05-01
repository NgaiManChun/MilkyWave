// =======================================================
// main.cpp
// 
// FBXからMGM（モデル）とMGA（アニメーション）
// の変換プログラム
// 
// 作者：魏文俊（ガイ　マンチュン）　2024/12/23
// =======================================================
#define NOMINMAX
#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/matrix4x4.h"
#include "assimp/Importer.hpp"
#pragma comment (lib, "assimp-vc143-mt.lib")
#include <assert.h>
#include <fstream>
#include <iostream>
#include "MGCommon.h"
using namespace MG;

namespace FBXDATA {

	PRIMITIVE_TYPE GetPrimitiveType(unsigned int type) {
		if (type == aiPrimitiveType_POINT) {
			return PRIMITIVE_TYPE_POINT;
		}
		else if (type == aiPrimitiveType_LINE) {
			return PRIMITIVE_TYPE_LINE;
		}
		else if (type == aiPrimitiveType_TRIANGLE) {
			return PRIMITIVE_TYPE_TRIANGLE;
		}
		return PRIMITIVE_TYPE_NONE;
	}

	void LoadNode(MODEL_NODE* node, aiNode* ainode) {
		node->childrenNum = ainode->mNumChildren;
		node->children = new MODEL_NODE[ainode->mNumChildren]{};
		node->meshNum = ainode->mNumMeshes;
		node->meshIndexes = new unsigned int[ainode->mNumMeshes]();
		memcpy(node->meshIndexes, ainode->mMeshes, sizeof(unsigned int) * ainode->mNumMeshes);
		aiVector3D scale;
		aiVector3D position;
		aiQuaternion rotate;
		ainode->mTransformation.Decompose(scale, rotate, position);
		node->scale = { scale.x, scale.y, scale.z };
		node->position = { position.x, position.y, position.z };
		node->rotate = { rotate.x, rotate.y, rotate.z, rotate.w };
		node->name = ainode->mName.C_Str();
		node->instance = "";
		if (ainode->mMetaData) {
			for (int i = 0; i < ainode->mMetaData->mNumProperties; i++) {
				aiString key = ainode->mMetaData->mKeys[i];
				aiMetadataType type = ainode->mMetaData->mValues[i].mType;
				if (!strcmp(key.C_Str(), "instance") && type == AI_AISTRING) {
					aiString* value = (aiString*)ainode->mMetaData->mValues[i].mData;
					node->instance = value->C_Str();
					break;
				}
				
			}
		}
		
		for (unsigned int i = 0; i < ainode->mNumChildren; i++) {
			LoadNode((node->children + i), ainode->mChildren[i]);
		}
	}

	void UnloadNode(MODEL_NODE* node) {
		for (unsigned int i = 0; i < node->childrenNum; i++) {
			UnloadNode((node->children + i));
		}
		delete[] node->children;
		delete[] node->meshIndexes;
		node->children = nullptr;
		node->meshIndexes = nullptr;
	}

	void GetModelNodeList(MODEL_NODE* node, std::list<MODEL_NODE*>& list) {
		for (unsigned int i = 0; i < node->childrenNum; i++) {
			list.push_back((node->children + i));
		}
		for (unsigned int i = 0; i < node->childrenNum; i++) {
			GetModelNodeList((node->children + i), list);
		}
	}

	char* GetModelData(MODEL& model, size_t& size) {

		// サイズ計算
		size = sizeof(MODEL);
		size += sizeof(MESH) * model.meshNum;
		for (int i = 0; i < model.meshNum; i++)
		{
			MESH& mesh = model.meshes[i];
			size += sizeof(VERTEX) * mesh.vertexNum;
			size += sizeof(unsigned int) * mesh.vertexIndexNum;
			size += std::strlen(mesh.textureStr) + 1;

			size += sizeof(BONE) * mesh.boneNum;
			for (int b = 0; b < mesh.boneNum; b++) {
				size += std::strlen(mesh.bones[b].name) + 1;
			}
			if (mesh.boneNum > 0) {
				size += sizeof(VERTEX_BONE_WEIGHT) * mesh.vertexNum;
			}
		}

		size += sizeof(TEXTURE) * model.textureNum;
		for (int i = 0; i < model.textureNum; i++)
		{
			TEXTURE& texture = model.textures[i];
			size += (texture.height) ? sizeof(unsigned char) * texture.width * texture.height * 4 : texture.width;
			size += std::strlen(texture.textureStr) + 1;
		}

		std::list<MODEL_NODE*> nodeList;
		nodeList.push_back(model.rootNode);
		GetModelNodeList(model.rootNode, nodeList);
		size += sizeof(MODEL_NODE) * nodeList.size();
		for (auto itr = nodeList.begin(); itr != nodeList.end(); itr++) {

			MODEL_NODE* node = *itr;
			size += sizeof(unsigned int) * node->meshNum;
			size += std::strlen(node->name) + 1;
			size += std::strlen(node->instance) + 1;
		}

		// データ写す
		char* data = new char[size];
		char* current = data;
		size_t length = sizeof(MODEL);
		memcpy(current, &model, length);
		MODEL* distModel = (MODEL*)current;
		distModel->meshes = nullptr;
		distModel->textures = nullptr;
		distModel->rootNode = nullptr;
		current += length;

		// メッシュ
		length = sizeof(MESH) * model.meshNum;
		memcpy(current, model.meshes, length);
		MESH* distMeshes = (MESH*)current;
		current += length;
		for (unsigned int i = 0; i < model.meshNum; i++) {
			MESH& mesh = model.meshes[i];

			// 頂点
			length = sizeof(VERTEX) * mesh.vertexNum;
			memcpy(current, mesh.vertices, length);
			current += length;
			if (mesh.boneNum > 0) {
				length = sizeof(VERTEX_BONE_WEIGHT) * mesh.vertexNum;
				memcpy(current, mesh.boneWeights, length);
				current += length;
			}

			// インデックス
			length = sizeof(unsigned int) * mesh.vertexIndexNum;
			memcpy(current, mesh.vertexIndexes, length);
			current += length;

			// テクスチャ文字
			length = std::strlen(mesh.textureStr) + 1;
			strcpy_s(current, length, mesh.textureStr);
			current += length;

			// ボーン
			length = sizeof(BONE) * mesh.boneNum;
			memcpy(current, mesh.bones, length);
			BONE* distBones = (BONE*)current;
			current += length;
			for (int b = 0; b < mesh.boneNum; b++) {
				length = std::strlen(mesh.bones[b].name) + 1;
				strcpy_s(current, length, mesh.bones[b].name);
				current += length;
				distBones[b].name = nullptr;
			}

			distMeshes[i].vertices = nullptr;
			distMeshes[i].boneWeights = nullptr;
			distMeshes[i].vertexIndexes = nullptr;
			distMeshes[i].textureStr = nullptr;
			distMeshes[i].bones = nullptr;
		}
		
		// テクスチャ
		length = sizeof(TEXTURE) * model.textureNum;
		memcpy(current, model.textures, length);
		TEXTURE* distTextures = (TEXTURE*)current;
		current += length;
		for (int i = 0; i < model.textureNum; i++)
		{
			TEXTURE& texture = model.textures[i];

			// 画像データ
			length = (texture.height) ? sizeof(unsigned char) * texture.width * texture.height * 4 : texture.width;
			memcpy(current, texture.data, length);
			current += length;

			// テクスチャ文字
			length = std::strlen(texture.textureStr) + 1;
			strcpy_s(current, length, texture.textureStr);
			current += length;

			distTextures[i].data = nullptr;
			distTextures[i].textureStr = nullptr;
		}

		// ノード
		/*length = sizeof(MODEL_NODE) * nodeList.size();
		memcpy(current, *nodeList.data(), length);
		MODEL_NODE* distModelNodes = (MODEL_NODE*)current;
		current += length;*/

		for (auto itr = nodeList.begin(); itr != nodeList.end(); itr++) {

			length = sizeof(MODEL_NODE);
			memcpy(current, *itr, length);
			MODEL_NODE* distModelNode = (MODEL_NODE*)current;
			current += length;

			distModelNode->children = nullptr;
			distModelNode->meshIndexes = nullptr;
		}

		for (auto itr = nodeList.begin(); itr != nodeList.end(); itr++) {
			MODEL_NODE* node = *itr;

			// メッシュインデックス
			length = sizeof(unsigned int) * node->meshNum;
			memcpy(current, node->meshIndexes, length);
			current += length;

			// ノード名
			length = std::strlen(node->name) + 1;
			strcpy_s(current, length, node->name);
			current += length;

			length = std::strlen(node->instance) + 1;
			strcpy_s(current, length, node->instance);
			current += length;
		}
		return data;
	}

	char* GetAnimationData(ANIMATION& animation, size_t& size) {

		// サイズ計算
		size = sizeof(ANIMATION);
		size += sizeof(ANIMATION_CHANNEL) * animation.channelNum;
		size += std::strlen(animation.name) + 1;
		for (int a = 0; a < animation.channelNum; a++) {
			ANIMATION_CHANNEL& animationChannel = animation.channels[a];
			size += sizeof(VECTOR_KEY) * animationChannel.positionKeyNum;
			size += sizeof(VECTOR_KEY) * animationChannel.scalingKeyNum;
			size += sizeof(QUATERNION_KEY) * animationChannel.rotationKeyNum;
			size += strlen(animationChannel.nodeName) + 1;
		}

		// データ写す
		char* data = new char[size];
		char* current = data;
		size_t length = sizeof(ANIMATION);
		memcpy(current, &animation, length);
		ANIMATION* distAnimation = (ANIMATION*)current;
		current += length;

		length = strlen(animation.name) + 1;
		strcpy_s(current, length, animation.name);
		current += length;

		length = sizeof(ANIMATION_CHANNEL) * animation.channelNum;
		memcpy(current, animation.channels, length);
		ANIMATION_CHANNEL* distChannel = (ANIMATION_CHANNEL*)current;
		current += length;

		distAnimation->name = nullptr;
		distAnimation->channels = nullptr;

		for (int a = 0; a < animation.channelNum; a++) {
			ANIMATION_CHANNEL& animationChannel = animation.channels[a];

			length = sizeof(VECTOR_KEY) * animationChannel.positionKeyNum;
			memcpy(current, animationChannel.positionKeys, length);
			current += length;

			length = sizeof(VECTOR_KEY) * animationChannel.scalingKeyNum;
			memcpy(current, animationChannel.scalingKeys, length);
			current += length;

			length = sizeof(QUATERNION_KEY) * animationChannel.rotationKeyNum;
			memcpy(current, animationChannel.rotationKeys, length);
			current += length;

			length = strlen(animationChannel.nodeName) + 1;
			strcpy_s(current, length, animationChannel.nodeName);
			current += length;

			/*distChannel->positionKeys = nullptr;
			distChannel->scalingKeys = nullptr;
			distChannel->rotationKeys = nullptr;
			distChannel->nodeName = nullptr;*/
		}

		return data;
	}

	void ReadModel(const char* fileName, const char* outputName = nullptr) {

		Assimp::Importer importer;
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_OPTIMIZE_EMPTY_ANIMATION_CURVES, false);
		const aiScene* scene = importer.ReadFile(fileName
			, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded);

		//const aiScene* scene = aiImportFile(fileName, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);
		assert(scene);
		
		MODEL model;
		model.meshNum = scene->mNumMeshes;
		model.textureNum = scene->mNumTextures;
		model.meshes = new MESH[scene->mNumMeshes]{};
		model.textures = new TEXTURE[scene->mNumTextures]{};
		model.rootNode = new MODEL_NODE{};
		// メッシュ読み込み
		for (int i = 0; i < scene->mNumMeshes; i++)
		{
			aiMesh* aimesh = scene->mMeshes[i];
			MESH& mesh = model.meshes[i];
			mesh.primitiveType = GetPrimitiveType(aimesh->mPrimitiveTypes);
			

			int faceIndexNum = 0;
			if (mesh.primitiveType == PRIMITIVE_TYPE_TRIANGLE) {
				faceIndexNum = 3;
			}
			else if (mesh.primitiveType == PRIMITIVE_TYPE_LINE) {
				faceIndexNum = 2;
			}
			else if (mesh.primitiveType == PRIMITIVE_TYPE_POINT) {
				faceIndexNum = 1;
			}

			mesh.vertexIndexNum = aimesh->mNumFaces * faceIndexNum;
			mesh.vertexIndexes = new unsigned int[mesh.vertexIndexNum]{};
			mesh.boneNum = aimesh->mNumBones;
			mesh.vertexNum = aimesh->mNumVertices;
			mesh.vertices = new VERTEX[aimesh->mNumVertices]{};

			// 頂点バッファ生成
			{
				for (unsigned int v = 0; v < aimesh->mNumVertices; v++)
				{
					mesh.vertices[v].position = { aimesh->mVertices[v].x, aimesh->mVertices[v].y, aimesh->mVertices[v].z };
					if (aimesh->mColors[0]) {
						mesh.vertices[v].diffuse = { aimesh->mColors[0]->r, aimesh->mColors[0]->g, aimesh->mColors[0]->b, aimesh->mColors[0]->a };
					}
					else if(aimesh->mMaterialIndex != -1) {
						mesh.vertices[v].diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
						aiMaterial* material = scene->mMaterials[aimesh->mMaterialIndex];
						aiColor4D color;
						//$raw.Maya|baseColor
						if (material->Get(AI_MATKEY_BASE_COLOR, color) == AI_SUCCESS) {
							mesh.vertices[v].diffuse.x = color.r;
							mesh.vertices[v].diffuse.y = color.g;
							mesh.vertices[v].diffuse.z = color.b;
						}
						else if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
							mesh.vertices[v].diffuse.x = color.r;
							mesh.vertices[v].diffuse.y = color.g;
							mesh.vertices[v].diffuse.z = color.b;
							
						}
						ai_real opacity;
						if (material->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS) {
							mesh.vertices[v].diffuse.w = opacity;
						}
					}
					else {
						mesh.vertices[v].diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
					}
					if (aimesh->mTextureCoords[0]) {
						mesh.vertices[v].texCoord = { aimesh->mTextureCoords[0][v].x, aimesh->mTextureCoords[0][v].y };
					}
					else {
						mesh.vertices[v].texCoord = { };
					}
					if (aimesh->mNormals) {
						mesh.vertices[v].normal = { aimesh->mNormals[v].x, aimesh->mNormals[v].y, aimesh->mNormals[v].z };
					}
					else {
						mesh.vertices[v].normal = { };
					}
					
				}
			}

			// ボーン情報
			if (aimesh->mNumBones > 0) {
				mesh.bones = new BONE[aimesh->mNumBones]{};

				static struct BONE_WEIGHT {
					int boneId = 0;
					float weight = 0.0f;
				};
				std::vector<std::vector<BONE_WEIGHT>> vertexWeights(aimesh->mNumVertices);
				for (int b = 0; b < aimesh->mNumBones; b++) {
					mesh.bones[b].transform = {
						aimesh->mBones[b]->mOffsetMatrix.a1, aimesh->mBones[b]->mOffsetMatrix.a2, aimesh->mBones[b]->mOffsetMatrix.a3, aimesh->mBones[b]->mOffsetMatrix.a4,
						aimesh->mBones[b]->mOffsetMatrix.b1, aimesh->mBones[b]->mOffsetMatrix.b2, aimesh->mBones[b]->mOffsetMatrix.b3, aimesh->mBones[b]->mOffsetMatrix.b4,
						aimesh->mBones[b]->mOffsetMatrix.c1, aimesh->mBones[b]->mOffsetMatrix.c2, aimesh->mBones[b]->mOffsetMatrix.c3, aimesh->mBones[b]->mOffsetMatrix.c4,
						aimesh->mBones[b]->mOffsetMatrix.d1, aimesh->mBones[b]->mOffsetMatrix.d2, aimesh->mBones[b]->mOffsetMatrix.d3, aimesh->mBones[b]->mOffsetMatrix.d4
					};
					mesh.bones[b].name = aimesh->mBones[b]->mName.C_Str();
					for (unsigned int w = 0; w < aimesh->mBones[b]->mNumWeights; w++) {
						vertexWeights[aimesh->mBones[b]->mWeights[w].mVertexId].push_back({ b, aimesh->mBones[b]->mWeights[w].mWeight });
					}
				}

				mesh.boneWeights = new VERTEX_BONE_WEIGHT[aimesh->mNumVertices]{};
				for (unsigned int v = 0; v < aimesh->mNumVertices; v++)
				{
					std::sort(vertexWeights[v].begin(), vertexWeights[v].end(), [](BONE_WEIGHT& a, BONE_WEIGHT& b) {
						return a.weight > b.weight;
					});
					for (int w = 0; w < 4; w++) {
						if (w < vertexWeights[v].size()) {
							mesh.boneWeights[v].boneIndexes[w] = vertexWeights[v][w].boneId;
							mesh.boneWeights[v].weights[w] = vertexWeights[v][w].weight;
						}
						else {
							mesh.boneWeights[v].boneIndexes[w] = 0;
							mesh.boneWeights[v].weights[w] = 0.0f;
						}

					}
				}
			}

			// インデックスバッファ生成
			{
				for (unsigned int f = 0; f < aimesh->mNumFaces; f++)
				{
					const aiFace* face = &aimesh->mFaces[f];
					for (int fi = 0; fi < faceIndexNum; fi++) {
						mesh.vertexIndexes[f * faceIndexNum + fi] = face->mIndices[fi];
					}
				}
			}
			
			// テクスチャ
			aiString texture;
			aiMaterial* aimaterial = scene->mMaterials[aimesh->mMaterialIndex];
			aimaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texture);
			char* str = new char[texture.length + 1];
			strncpy(str, texture.C_Str(), texture.length + 1);
			mesh.textureStr = str;
		}

		// テクスチャ読み込み
		for (int i = 0; i < scene->mNumTextures; i++)
		{
			aiTexture* aitexture = scene->mTextures[i];
			TEXTURE& texture = model.textures[i];
			texture.textureStr = aitexture->mFilename.C_Str();
			texture.width = aitexture->mWidth;
			texture.height = aitexture->mHeight;
			texture.data = (unsigned char*)aitexture->pcData;
		}
		
		
		LoadNode(model.rootNode, scene->mRootNode);

		// アウトプット
		size_t size;
		char* data = GetModelData(model, size);

		MGObject mgo{
			MGOBJECT_TYPE_MODEL,
			size,
			0
		};

		std::string ouputFileName;
		if (outputName) {
			ouputFileName = outputName;
		}
		else {
			ouputFileName = fileName;
			ouputFileName += ".mgm";
		}
		
		std::ofstream file(ouputFileName, std::ios::binary);

		file.write((const char*)&mgo, sizeof(MGObject));
		file.write((const char*)data, size);
		file.close();

		// 以下後始末
		UnloadNode(model.rootNode);
		for (int i = 0; i < model.meshNum; i++)
		{
			MESH& mesh = model.meshes[i];
			delete[] mesh.vertices;
			delete[] mesh.boneWeights;
			delete[] mesh.vertexIndexes;
			delete[] mesh.textureStr;
			delete[] mesh.bones;
			mesh.vertices = nullptr;
			mesh.boneWeights = nullptr;
			mesh.vertexIndexes = nullptr;
			mesh.textureStr = nullptr;
			mesh.bones = nullptr;
		}
		delete[] model.meshes;
		delete[] model.textures;
		delete model.rootNode;
		model.meshes = nullptr;
		model.textures = nullptr;
		model.rootNode = nullptr;

		//aiReleaseImport(scene);
		importer.FreeScene();
	}

	void ReadAnimation(const char* fileName, const char* outputName = nullptr) {

		Assimp::Importer importer;
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_OPTIMIZE_EMPTY_ANIMATION_CURVES, false);
		//importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_ALL_GEOMETRY_LAYERS, false);
		const aiScene* scene = importer.ReadFile(fileName
			, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded);
		//const aiScene* scene = aiImportFile(fileName, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded);
		assert(scene);

		// アニメーション読み込み
		for (int i = 0; i < scene->mNumAnimations; i++)
		{
			aiAnimation* aianimation = scene->mAnimations[i];
			ANIMATION animation{};
			animation.name = aianimation->mName.C_Str();
			animation.frameRate = aianimation->mTicksPerSecond;
			animation.frames = aianimation->mDuration;
			animation.channelNum = aianimation->mNumChannels;
			animation.channels = new ANIMATION_CHANNEL[aianimation->mNumChannels]{};
			for (int a = 0; a < aianimation->mNumChannels; a++) {
				// アニメーションノード、適応オブジェクトごと
				aiNodeAnim* ainodeanim = aianimation->mChannels[a];
				ANIMATION_CHANNEL& animationChannel = animation.channels[a];
				animationChannel.nodeName = ainodeanim->mNodeName.C_Str();
				animationChannel.positionKeyNum = ainodeanim->mNumPositionKeys;
				animationChannel.rotationKeyNum = ainodeanim->mNumRotationKeys;
				animationChannel.scalingKeyNum = ainodeanim->mNumScalingKeys;
				animationChannel.positionKeys = new VECTOR_KEY[ainodeanim->mNumPositionKeys]{};
				animationChannel.scalingKeys = new VECTOR_KEY[ainodeanim->mNumScalingKeys]{};
				animationChannel.rotationKeys = new QUATERNION_KEY[ainodeanim->mNumRotationKeys]{};

				// スケール
				for (int k = 0; k < ainodeanim->mNumScalingKeys; k++) {
					VECTOR_KEY& scalingKey = animationChannel.scalingKeys[k];
					aiVector3D vector = ainodeanim->mScalingKeys[k].mValue;
					scalingKey.frame = ainodeanim->mScalingKeys[k].mTime;
					scalingKey.vector = { vector.x, vector.y, vector.z };
				}

				// 回転
				for (int k = 0; k < ainodeanim->mNumRotationKeys; k++) {
					QUATERNION_KEY& rotationKey = animationChannel.rotationKeys[k];
					aiQuaternion quaternion = ainodeanim->mRotationKeys[k].mValue;
					
					rotationKey.frame = ainodeanim->mRotationKeys[k].mTime;
					rotationKey.rotate = { quaternion.x, quaternion.y, quaternion.z, quaternion.w };
				}

				// 移動
				for (int k = 0; k < ainodeanim->mNumPositionKeys; k++) {
					VECTOR_KEY& positionKey = animationChannel.positionKeys[k];
					aiVector3D vector = ainodeanim->mPositionKeys[k].mValue;
					positionKey.frame = ainodeanim->mPositionKeys[k].mTime;
					positionKey.vector = { vector.x, vector.y, vector.z };
				}
			}


			// アウトプット
			size_t size;
			char* data = GetAnimationData(animation, size);

			MGObject mgo{
				MGOBJECT_TYPE_ANIMATION,
				size,
				0
			};

			std::string ouputFileName;
			if (outputName) {
				ouputFileName = outputName;
			}
			else {
				ouputFileName = fileName;
				if (scene->mNumAnimations > 1) {
					ouputFileName += "_" + std::to_string(i);
				}
				ouputFileName += ".mga";
			}
			std::ofstream file(ouputFileName, std::ios::binary);

			file.write((const char*)&mgo, sizeof(MGObject));
			file.write((const char*)data, size);
			file.close();


			// 以下後始末
			for (int a = 0; a < animation.channelNum; a++) {
				ANIMATION_CHANNEL& animationChannel = animation.channels[a];
				delete[] animationChannel.positionKeys;
				delete[] animationChannel.scalingKeys;
				delete[] animationChannel.rotationKeys;
				animationChannel.positionKeys = nullptr;
				animationChannel.scalingKeys = nullptr;
				animationChannel.rotationKeys = nullptr;
			}
			delete[] animation.channels;
			animation.channels = nullptr;
		}

		//aiReleaseImport(scene);
		importer.FreeScene();
	}

	void GetInstanceList(aiNode* ainode, std::list<MODEL_INSTANCE>& list) {

		if (ainode->mMetaData) {
			for (int i = 0; i < ainode->mMetaData->mNumProperties; i++) {
				aiString key = ainode->mMetaData->mKeys[i];
				aiMetadataType type = ainode->mMetaData->mValues[i].mType;
				if (!strcmp(key.C_Str(), "instance") && type == AI_AISTRING) {
					aiString* value = (aiString*)ainode->mMetaData->mValues[i].mData;
					aiVector3D scale;
					aiVector3D position;
					aiQuaternion rotate;
					ainode->mTransformation.Decompose(scale, rotate, position);
					MODEL_INSTANCE instance = {
						ainode->mName.C_Str(),
						value->C_Str(),
						{ scale.x, scale.y, scale.z },
						{ position.x, position.y, position.z },
						{ rotate.x, rotate.y, rotate.z, rotate.w }
					};
					list.push_back(instance);
					break;
				}

			}
		}

		for (unsigned int i = 0; i < ainode->mNumChildren; i++) {
			GetInstanceList(ainode->mChildren[i], list);
		}
	}

	char* GetArrangementData(ARRANGEMENT& arrangement, size_t& size) {

		// サイズ計算
		size = sizeof(ARRANGEMENT);
		size += sizeof(MODEL_INSTANCE) * arrangement.instanceNum;
		for (int i = 0; i < arrangement.instanceNum; i++) {
			size += std::strlen(arrangement.instances[i].name) + 1;
			size += std::strlen(arrangement.instances[i].instance) + 1;
		}

		// データ写す
		char* data = new char[size];
		char* current = data;
		size_t length = sizeof(ARRANGEMENT);
		memcpy(current, &arrangement, length);
		((ARRANGEMENT*)current)->instances = nullptr;
		current += length;

		length = sizeof(MODEL_INSTANCE) * arrangement.instanceNum;
		memcpy(current, arrangement.instances, length);
		MODEL_INSTANCE* instances = (MODEL_INSTANCE*)current;
		current += length;
		for (int i = 0; i < arrangement.instanceNum; i++) {
			
			length = strlen(arrangement.instances[i].name) + 1;
			strcpy_s(current, length, arrangement.instances[i].name);
			current += length;

			length = strlen(arrangement.instances[i].instance) + 1;
			strcpy_s(current, length, arrangement.instances[i].instance);
			current += length;

			instances[i].name = nullptr;
			instances[i].instance = nullptr;
		}
		return data;
	}

	void ReadInstanceArrangement(const char* fileName, const char* outputName = nullptr) {

		Assimp::Importer importer;
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_OPTIMIZE_EMPTY_ANIMATION_CURVES, false);
		const aiScene* scene = importer.ReadFile(fileName
			, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded);

		//const aiScene* scene = aiImportFile(fileName, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded);
		assert(scene);

		std::list<MODEL_INSTANCE> list;
		GetInstanceList(scene->mRootNode, list);

		if (list.size() == 0) return;

		ARRANGEMENT arrangement = {
			list.size(),
			new MODEL_INSTANCE[list.size()]
		};
		int index = 0;
		for (auto& instance : list) {
			arrangement.instances[index] = instance;
			index++;
		}

		size_t size;
		char* data = GetArrangementData(arrangement, size);

		MGObject mgo{
			MGOBJECT_TYPE_ARRANGEMENT,
			size,
			0
		};

		std::string ouputFileName;
		if (outputName) {
			ouputFileName = outputName;
		}
		else {
			ouputFileName = fileName;
			ouputFileName += ".mgi";
		}

		std::ofstream file(ouputFileName, std::ios::binary);

		file.write((const char*)&mgo, sizeof(MGObject));
		file.write((const char*)data, size);
		file.close();

		delete[] arrangement.instances;
		//aiReleaseImport(scene);
		importer.FreeScene();
	}

}


int main(int argc, char* argv[]) {

	bool readModel = true;
	bool readAnimation = true;
	bool readArrangement = true;
	bool renameModel = false;
	bool renameAnimation = false;
	bool renameArrangement = false;
	char* modelOutputName = nullptr;
	char* animationOutputName = nullptr;
	char* arrangementOutputName = nullptr;
	char* inFile = nullptr;

	for (int i = 1; i < argc; i++) {
		if (renameModel) {
			renameModel = false;
			modelOutputName = argv[i];
		}
		else if (renameAnimation) {
			renameAnimation = false;
			animationOutputName = argv[i];
		}
		else if (renameArrangement) {
			renameArrangement = false;
			arrangementOutputName = argv[i];
		}
		else if (!strcmp(argv[i], "--without-model")){
			readModel = false;
		}
		else if (!strcmp(argv[i], "--without-animation")) {
			readAnimation = false;
		}
		else if (!strcmp(argv[i], "--without-arrangement")) {
			readArrangement = false;
		}
		else if (!strcmp(argv[i], "--output")) {
			renameModel = true;
		}
		else if (!strcmp(argv[i], "--output-animation")) {
			renameAnimation = true;
		}
		else if (!strcmp(argv[i], "--output-arrangement")) {
			renameArrangement = true;
		}
		else {
			inFile = argv[i];
		}
	}

	if (readModel && inFile) {
		FBXDATA::ReadModel(inFile, modelOutputName);
	}
	if (readAnimation && inFile) {
		FBXDATA::ReadAnimation(inFile, animationOutputName);
	}
	if (readArrangement && inFile) {
		FBXDATA::ReadInstanceArrangement(inFile, arrangementOutputName);
	}

	return 0;
}