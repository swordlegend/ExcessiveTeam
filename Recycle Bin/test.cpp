#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "../../Common/src/Factory.h"

#include "SFML/System.hpp"
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"

#include <iostream>
#include <string>
#include <fstream>
using namespace std;

#include "mymath/mymath.h"
using namespace mymath;

const char* vshd =
"#version 440 core \n"
"layout(std140) uniform constant_data \n"
"{ \n"
"  mat4 mvp; \n"
"} cd; \n"
"in vec3 in_vertex; \n"
"in vec2 in_texcoord; \n"
"out vec2 texcoord; \n"
"void main() \n"
"{ \n"
"  texcoord = in_texcoord; \n"
"  gl_Position = cd.mvp * vec4(in_vertex, 1); \n"
"} \n";

const char* pshd =
"#version 440 core \n"
"uniform sampler2D tex; \n"
"in vec2 texcoord; \n"
"out vec4 color; \n"
"void main() \n"
"{ \n"
"  color = texture(tex, vec2(texcoord.x, 1-texcoord.y)); \n"
"} \n";

std::string Get_app_path();
#ifdef WIN_BUILD
char* realpath(const char* path, char** ret);
#endif

typedef IGapi* (*getGapiType)(void);

int main(int argc, char** args)
{
	string app_path = Get_app_path();

	//initialize sfml
	sf::Window w;
	w.create(sf::VideoMode(512, 512), "Graphics Api GL Test");

	if (!w.isOpen())
	{
		cerr << "Couldn't initialize SFML." << endl;
	}

	w.setVerticalSyncEnabled(true);

	//load GL Graphics Api dll
	string dll_path = app_path + "GraphicsApiGL.dll";

	fstream f;
	f.open(dll_path.c_str());

	if (!f.is_open())
		cerr << "couldn't open dll." << endl;
	else
		f.close();

	std::replace(dll_path.begin(), dll_path.end(), '/', '\\');
	auto dll = LoadLibrary(dll_path.c_str());
	getGapiType CreateGraphicsApi = 0;

	if (dll)
	{
		CreateGraphicsApi = (getGapiType)GetProcAddress(dll, "createGraphicsApi");
	}

	//set up the GL Gapi
	//IGapi* gapi = CreateGraphicsApi();
	IGapi* gapi = Factory::CreateGapiGL();

	gapi->SetDebugOutput(true);
	gapi->SetSeamlessCubeMaps(true);
	gapi->SetSyncDebugOutput(true);

	//set up the shader program
	auto sp = gapi->CreateShaderSource(vshd, pshd);
	if (!sp)
		return 0;


	/**/
	/*/example binary shader store/load
	char* data;
	u32 size;
	sp->GetBinary(&data, &size);

	f.open("shader.shaderbin", ios::binary | ios::out);
	f.write((const char*)&size, sizeof(u32));
	f.write(data, size);
	f.close();

	data = 0;
	size = 0;

	f.open("shader.shaderbin", ios::binary | ios::in);
	f.read((char*)&size, sizeof(u32));
	f.read(data = new char[size], size);
	f.close();

	sp->loadFromBinary(data);
	/**/

	//set up texture
	string image_path = app_path + "../Runtime/image.png";
	sf::Image im;
	im.loadFromFile(image_path);

	rTextureGapi texdata;
	texdata.width = im.getSize().x;
	texdata.height = im.getSize().y;
	texdata.depth = 1;
	texdata.format = eTextureFormat::RGBA8;
	texdata.is_cubemap = false;
	texdata.is_layered = false;
	texdata.num_levels = 1;

	auto tex = gapi->CreateTexture(texdata);

	rTextureUpdate texupdata;
	texupdata.data = (char*)im.getPixelsPtr();
	texupdata.depth = texdata.depth;
	texupdata.format = texdata.format;
	texupdata.width = texdata.width;
	texupdata.height = texdata.height;
	texupdata.level = 0;
	texupdata.x_offset = 0;
	texupdata.y_offset = 0;
	texupdata.z_offset = 0;

	gapi->WriteTexture(tex, texupdata);

	rSamplerState smpdata;
	smpdata.is_anisotropic = false;
	smpdata.is_bilinear = true;
	smpdata.is_clamped = true;
	smpdata.is_mipmapped = false;
	gapi->SetSamplerState("tex", smpdata, tex);


	//set up the mesh
	vector<vec3> vertices;
	vector<vec2> texcoords;
	vector<u32> indices;

	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);

	indices.push_back(0);
	indices.push_back(2);
	indices.push_back(3);

	vertices.push_back(vec3(-1, -1, 0));
	vertices.push_back(vec3(1, -1, 0));
	vertices.push_back(vec3(1, 1, 0));
	vertices.push_back(vec3(-1, 1, 0));

	texcoords.push_back(vec2(0, 0));
	texcoords.push_back(vec2(1, 0));
	texcoords.push_back(vec2(1, 1));
	texcoords.push_back(vec2(0, 1));

	rBuffer tex_alloc_data;
	tex_alloc_data.is_persistent = false;
	tex_alloc_data.is_readable = false;
	tex_alloc_data.is_writable = true;
	tex_alloc_data.prefer_cpu_storage = false;
	tex_alloc_data.size = texcoords.size() * sizeof(vec2);

	rBuffer vtx_alloc_data;
	vtx_alloc_data.is_persistent = false;
	vtx_alloc_data.is_readable = false;
	vtx_alloc_data.is_writable = true;
	vtx_alloc_data.prefer_cpu_storage = false;
	vtx_alloc_data.size = vertices.size() * sizeof(vec3);

	rBuffer idx_alloc_data;
	idx_alloc_data.is_persistent = false;
	idx_alloc_data.is_readable = false;
	idx_alloc_data.is_writable = true;
	idx_alloc_data.prefer_cpu_storage = false;
	idx_alloc_data.size = indices.size() * sizeof(u32);

	auto vtx_buf = gapi->CreateVertexBuffer(vtx_alloc_data);
	auto tex_buf = gapi->CreateVertexBuffer(tex_alloc_data);

	auto idx_buf = gapi->CreateIndexBuffer(idx_alloc_data);

	gapi->WriteBuffer(vtx_buf, vertices.data(), vertices.size() * sizeof(vec3), 0);
	gapi->WriteBuffer(tex_buf, texcoords.data(), texcoords.size() * sizeof(vec2), 0);

	gapi->WriteBuffer(idx_buf, indices.data(), indices.size() * sizeof(u32), 0);

	//set up the uniform buffer
	rBuffer ubo_alloc_data;
	ubo_alloc_data.is_persistent = false;
	ubo_alloc_data.is_readable = true;
	ubo_alloc_data.is_writable = true;
	ubo_alloc_data.prefer_cpu_storage = false;
	ubo_alloc_data.size = 1 * sizeof(mat4);

	mat4 mvp = ortographic(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, -1.0f);

	auto ubo_buf = gapi->CreateUniformBuffer(ubo_alloc_data);

	gapi->WriteBuffer(ubo_buf, &mvp[0][0], sizeof(mat4), 0);

	//Draw stuff
	bool run = true;
	sf::Event ev;
	while (run)
	{
		while (w.pollEvent(ev))
		{
			if (ev.type == sf::Event::Closed || (ev.type == sf::Event::KeyPressed && ev.key.code == sf::Keyboard::Escape))
			{
				run = false;
			}

			//handle events
		}

		static vector<IVertexBuffer*> vbos;
		static vector<rVertexAttrib> attribs;

		vbos.clear();
		vbos.push_back(vtx_buf);
		vbos.push_back(tex_buf);

		attribs.clear();

		rVertexAttrib attr;
		attr.nComponent = 3;
		attr.divisor = 0;
		attr.index = sp->GetAttributeIndex("in_vertex");
		attr.offset = 0;
		attr.size = 0;
		attr.type = eVertexAttribType::FLOAT;
		attribs.push_back(attr);

		attr.nComponent = 2;
		attr.index = sp->GetAttributeIndex("in_texcoord");
		attribs.push_back(attr);

		gapi->SetViewport(0, 0, 512, 512);
		gapi->SetShaderProgram(sp);
		gapi->SetRenderTargets(0, 0);
		//gapi->SetTextureView(texview, sp->GetSamplerIndex("tex"));
		gapi->SetTexture(tex, sp->GetSamplerIndex("tex"));
		gapi->SetUniformBuffer(ubo_buf, sp->GetUniformBlockIndex("constant_data"));
		gapi->SetVertexBuffers(&vbos[0], attribs.data(), vbos.size());
		gapi->SetIndexBuffer(idx_buf);
		gapi->Draw(indices.size());

		w.display();
	}

	FreeLibrary(dll);

	w.close();
}

#ifdef WIN_BUILD
char* realpath(const char* path, char** ret)
{
	char* the_ret = 0;

	if (!ret)
	{
		the_ret = new char[MAX_PATH];
	}
	else
	{
		if (!*ret)
		{
			*ret = new char[MAX_PATH];
		}
		else
		{
			size_t s = strlen(*ret);

			if (s < MAX_PATH)
			{
				delete[] * ret;
				*ret = new char[MAX_PATH];

			}

			the_ret = *ret;
		}
	}

	size_t size = GetFullPathNameA(path, MAX_PATH, the_ret, 0);

	if (size > MAX_PATH)
	{
		//too long path
		cerr << "Path too long, truncated." << endl;
		delete[] the_ret;
		return "";
	}

	if (ret)
	{
		*ret = the_ret;
	}

	return the_ret;
}
#endif

std::string Get_app_path()
{

	char fullpath[4096];
	std::string app_path;

	/* /proc/self is a symbolic link to the process-ID subdir
	  * of /proc, e.g. /proc/4323 when the pid of the process
	  * of this program is 4323.
	  *
	  * Inside /proc/<pid> there is a symbolic link to the
	  * executable that is running as this <pid>.  This symbolic
	  * link is called "exe".
	  *
	  * So if we read the path where the symlink /proc/self/exe
	  * points to we have the full path of the executable.
	  */

#ifdef LINUX_BUILD
	int length;
	length = readlink("/proc/self/exe", fullpath, sizeof(fullpath));

	/* Catch some errors: */

	if (length < 0)
	{
		my::log << my::lock << "Couldnt read app path. Error resolving symlink /proc/self/exe." << my::endl << my::unlock;
		loop::get().shutdown();
	}

	if (length >= 4096)
	{
		my::log << my::lock << "Couldnt read app path. Path too long. Truncated." << my::endl << my::unlock;
		loop::get().shutdown();
	}

	/* I don't know why, but the string this readlink() function
	  * returns is appended with a '@'.
	  */
	fullpath[length] = '\0';       /* Strip '@' off the end. */

#endif

#ifdef WIN_BUILD

	if (GetModuleFileName(0, (char*)&fullpath, sizeof(fullpath) - 1) == 0)
	{
		cerr << "Couldn't Get the app path." << endl;
		return "";
	}

#endif

	app_path = fullpath;

#ifdef WIN_BUILD
	app_path = app_path.substr(0, app_path.rfind("\\") + 1);
#endif

#ifdef __unix__
	config::get().app_path = config::get().app_path.substr(0, config::get().app_path.rfind("/") + 1);
#endif

	//app_path += "../";

	char* res = 0;

	res = realpath(app_path.c_str(), 0);

	if (res)
	{
#if WIN_BUILD
		app_path = std::string(res);
		delete[] res;
#endif

#if __unix__
		config::get().app_path = std::string(res) + "/";
		free(res); //the original linux version of realpath uses malloc
#endif
	}

	std::replace(app_path.begin(), app_path.end(), '\\', '/');

	return app_path;
}