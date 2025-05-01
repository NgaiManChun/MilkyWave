#ifndef _UI_PAUSE_H
#define _UI_PAUSE_H
#include "gameObject.h"
#include "gameObjectText.h"
using namespace MG;

class UIPause : public GameObject
{
public:
	enum PAUSE_OPTION {
		PAUSE_OPTION_RESUME,
		PAUSE_OPTION_STAGE,
		PAUSE_OPTION_TITLE,
		PAUSE_OPTION_GYRO,
		PAUSE_OPTION_MAX
	};
private:
	Texture* textureBackground;
	GameObjectText labels[PAUSE_OPTION_MAX];
	GameObjectText hintsLabel;
	PAUSE_OPTION currentOption;
private:
	std::wstring GetLabelText(PAUSE_OPTION option) const;
public:
	UIPause(const std::string& scope = RESOURCE_SCOPE_GOBAL);
	~UIPause();
	void Update() override;
	void Draw()override;
	void SetOption(PAUSE_OPTION option);
	PAUSE_OPTION GetOption() const;
	void Up();
	void Down();

};

#endif