#pragma once
#include "../../Module_Interfaces/GraphicsEngine/ILight.h"

class Light : public graphics::ILight
{
public:
	void setColor() override;
	void setPosition() override;
	void setDirection() override;
};