#pragma once

// namespace
namespace ge {

class IScene;

////////////////////////////////////////////////////////////////////////////////
/// GraphicsEngine manages and displays the graphical world.
/// No descrption yet.
////////////////////////////////////////////////////////////////////////////////

class IGraphicsEngine
{
public:
	virtual void release() = 0;

	virtual IScene* createScene() = 0;
};










} // namespace