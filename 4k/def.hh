/* Defines */

#pragma once

#include <windows.h>
#include <inttypes.h>


/* Error messages */


#ifdef DEBUG_BUILD

// Indices //
#define ERR_UNDEFINED	0
#define ERR_OPEN_WIN	1
#define ERR_INIT_WINAPI	2
#define ERR_INIT_SOUND	3
#define ERR_INIT_WR		4
#define ERR_SHADER_CMP	5
#define ERR_SHADER_LNK	6
#define ERR_INIT_SYNC	7
#define ERR_INIT_BASS	8

static const char* error_msg[] =
{
	// Undefined error
	"An undefined error occured",
	// Open window error
	"Unable to open window",
	// Init WinApi error
	"Unable to initialize WinApi",
	// Init sound playback error
	"Unable to initialize sound playback",
	// Init wrangler error
	"Unable to init opengl extension wrangler",
	// Shader compile error
	"Unable to compile shaders",
	// Shader link error
	"Unable to link shaders",
	// Rocket creation error
	"Unable to create rocket device",
	// Init bass error
	"Unable to init bass",
};

#endif


/* Demo defines */


#define WIDTH	800
#define HEIGHT	600


// Uniform bindings
#define UNIF_FLOAT_TIME 0

namespace DEMO
{
	void __fastcall Loop();
#ifdef DEBUG_BUILD
	void Die(int8_t cause = -1);
#else
	__forceinline void __fastcall Die();
#endif

	float time = 0.0;
};


#ifdef DEBUG_BUILD

#define EXPORT_TRACK_NAME "track.wav"

namespace ROCKET
{
	// Sync device
	sync_device* rocket;
	// Sync callback
	sync_cb cb;
	// Sync tracks
	const struct sync_track *r;
	const struct sync_track *g;
	const struct sync_track *b;
};

namespace BASS
{
	// Bass stream
	HSTREAM stream;
};

#endif


/* Window defines */


namespace WINDOW
{
	HWND win_handle;
	HGLRC hRC;
	PIXELFORMATDESCRIPTOR pfd;
	int pf_handle;
	HDC hDC;
};


/* Render defines */


namespace RENDER
{
	unsigned short hVS; //vertex shader handle
	unsigned short hPX; //pixel shader handle
	unsigned short hPr; //shader program handle
	// Uniforms
	unsigned int uLoc[2]; //Uniform locations
};


/* Global namespace functions */


LRESULT CALLBACK MainWProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
__forceinline void Init();
void __fastcall render_gl();

#ifdef DEBUG_BUILD
void __fastcall init_gl();
#else
__forceinline void __fastcall init_gl();
#endif