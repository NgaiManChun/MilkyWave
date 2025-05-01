#ifndef _STAR_EFFECT_H
#define _STAR_EFFECT_H

#include "gameObject.h"
#include "progress.h"
using namespace MG;


class StarEffect : public GameObject
{
public:
	/*struct Star
	{
		const GameObject* Target {};
		Texture* Texture {};
		F3 Position = {};
		F3 Velocity = {};
		float Lifetime = 0;
		float TimeNow = 0;
	};*/
	struct Star
	{
		F3 startPosition = {};
		F3 endPosition = {};
		Progress t;
	};
private:
	std::list<Star> stars;

	Texture* texture;
	float duration;
	float maxRadius;
	Progress spanT;
	bool started = false;
public:
	/*StarEffect(
		const GameObject* target,
		F3 position = {},
		F3 velocity = { (rand() % 100 - 50) / 50.0f,(rand() % 100 - 50) / 50.0f ,(rand() % 100 - 50) / 50.0f },
		float lifetime = 0.0f, float timeNow = 0.0f);*/
	
	StarEffect(F3 position = {}, float span = 30.0f, float duration = 700.0f, float maxRadius = 1.5f);

	~StarEffect() {}


	void Update() override;
	void Draw() override;

	void SetStarted(bool started);
};



#endif 

