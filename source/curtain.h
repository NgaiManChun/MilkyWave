#ifndef _CURTAIN_H
#define _CURTAIN_H

#include "gameObject.h"
#include "renderer.h"
#include "progress.h"
using namespace MG;

class Curtain : public GameObject {
public:
	struct _ASSET {
		std::string TEXTURE_WHITE;
		std::string TEXTURE_STAR;
	};
private:
	RenderTarget* renderTarget;
	Texture* tex;
	Texture* texStar;
	F3 starSize = {};
	Progress t = { 1000.0f, false };
	bool reverse = false;
public:
	Curtain();
	~Curtain();
	void Update() override;
	void Draw() override;
	void Reset();
	void SetReverse(bool reverse);
	bool GetReverse() const;
	float GetTime() const;
};

#endif
