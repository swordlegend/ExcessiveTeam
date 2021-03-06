#include "PlatformLibrary/Sys.h"
#include "PlatformLibrary/Window.h"
#include "PlatformLibrary/WindowCommon.h"
#include <vector>
#include "SupportLibrary/Common.h"
#include "PlatformLibrary/Windows/Timer_win.h"
#include <conio.h>
#include "Core/EngineCore.h"
#include "Core/GameWorld.h"
#include "../Games/Excessive-Strike/TestLevelScript.h"

// TODO REMOVE
//#include "../Games/Excessive-Strike/TestLevelScript.h"
//#include "../Games/Excessive-Strike/TestLevelScript.cpp"
//#include "../Games/Excessive-Strike/PlayerScript.cpp"

//static const char vertexShaderCode[] =
//"#version 440 core \n"
//"const vec2 quadVertices[3] = { vec2(-1, -1), vec2(3, -1), vec2(-1, 3) };\n"
//"const vec2 quadTexcoords[3] = { vec2(0, 1), vec2(2, 1), vec2(0, -1) };\n"
//"out vec2 tex0; \n"
//"void main()\n"
//"{\n"
//"	gl_Position = vec4(quadVertices[gl_VertexID], 0.0, 1.0);\n"
//"	tex0 = quadTexcoords[gl_VertexID]; \n"
//"}\n";
//
//static const char pixelShaderCode[] =
//"#version 440 core \n"
//"uniform sampler2D tex; \n"
//"in vec3 pos; \n"
//"in vec2 tex0; \n"
//"out vec4 color; \n"
//"void main() \n"
//"{ \n"
//"   color = texture(tex, tex0);\n"
//"} \n"
//;

class IGuiTexture
{
public:
	virtual mm::uvec2 GetDimension() const = 0;

	virtual u32 GetWidth() const = 0;
	virtual u32 GetHeight() const = 0;

	virtual const Color* GetPixels() const = 0;
};

class IGuiGraphicsEngine
{
public:
	virtual IGuiTexture* CreateTexture(const std::string& filePath) = 0;
};

class GuiImage
{
public:
	GuiImage(IGuiTexture* texture) : texture(texture)
	{

	}

	~GuiImage()
	{
		delete texture;
	}

	u32 GetWidth() const { return texture->GetWidth(); }
	u32 GetHeight() const { return texture->GetHeight(); }
	const mm::uvec2 GetDimension() const { return texture->GetDimension(); }

	const Color* GetPixels() const { return texture->GetPixels(); }

protected:
	IGuiTexture* texture;
};

class GuiTextureCPU : public IGuiTexture
{
public:
	GuiTextureCPU(const std::string& filePath)
	{
		bool b = img.loadFromFile(filePath);
		assert(b);
	}

	mm::uvec2 GetDimension() const
	{
		return mm::uvec2(img.getSize().x, img.getSize().y);
	}

	u32 GetWidth() const
	{
		return img.getSize().x;
	}

	u32 GetHeight() const
	{
		return img.getSize().y;
	}

	const Color* GetPixels() const
	{
		return (Color*)img.getPixelsPtr();
	}

protected:
	sf::Image img;
};

class GuiGraphicsEngineCPU : public IGuiGraphicsEngine
{
	IGuiTexture* CreateTexture(const std::string& filePath)
	{
		return new GuiTextureCPU(filePath);
	}
};

class GuiCore
{
public:
	GuiCore(IGuiGraphicsEngine* g) : guiGraphicsEngine(g)
	{

	}

	inline ~GuiCore()
	{ 
		for (auto a : images)
			delete a;

		delete guiGraphicsEngine;
	}

	inline GuiImage* AddImage(const std::string& filePath)
	{
		IGuiTexture* texture = guiGraphicsEngine->CreateTexture(filePath);
		GuiImage* img = new GuiImage(texture);
		images.push_back(img);
		return img;
	}

protected:
	IGuiGraphicsEngine* guiGraphicsEngine;
	std::vector<GuiImage*> images;

} EditorGui(new GuiGraphicsEngineCPU());

class GuiGraphicsEngineDx11
{
};

class GuiGraphicsEngineOpenGL
{
};

int main()
{
	GuiImage* editorStartupImg = EditorGui.AddImage(GetAssetsPath() + "editor_startup.jpg");

	WindowDesc windowDesc;
		windowDesc.clientSize = editorStartupImg->GetDimension();
		windowDesc.style = eWindowStyle::DEFAULT;
	Window* window = new Window(windowDesc);


	rGraphicsEngineRT graphicsDesc;
		graphicsDesc.targetWindow = window;
	//Core.InitGraphicsEngineRasterZsiros(graphicsDesc); // ...
	Core.InitPhysicsEngineBullet();
	
	Actor* ground = World.AddActor("box.DAE", 0);
	ground->SetKinematic(true);
	ground->SetScale(mm::vec3(100, 100, 1));
	ground->SetName("ground");

	// TMP !!
	//World.AddScript<TestLevelScript>();

	Timer* timer = new Timer();

	while (window->IsOpen())
	{
		WindowEvent evt;
		while (window->PopEvent(evt));

		float deltaTime = timer->GetSecondsPassed();
		timer->Reset();

		Core.Update(deltaTime);
		
		static uint64_t magic = 0;
		magic++;
		if((magic % 100) == 0)
			window->SetTitle("Fps: " + std::to_string(1.f / deltaTime));
	}
	
	getch();

//	delete pixels;
	delete window;
	delete timer;
	return 0;
}