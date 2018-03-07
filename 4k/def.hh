/* Defines */

#pragma once

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN

#include <windows.h>
#include <inttypes.h>

#ifdef DEBUG_BUILD
#include <cmath>
#endif


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
	"An undefined error occured",
	"Unable to open window",
	"Unable to initialize WinApi",
	"Unable to initialize sound playback",
	"Unable to init opengl extension wrangler",
	"Unable to compile shaders",
	"Unable to link shaders",
	"Unable to create rocket device",
	"Unable to init bass",
};

#endif


/* Demo defines */


#define WIDTH	1366
#define HEIGHT	768


namespace DEMO
{
	void __fastcall Loop();
#ifdef DEBUG_BUILD
	void __fastcall Die(int8_t cause = -1);
#else
	__forceinline void __fastcall Die();
#endif

	float time = 0.0;
	unsigned int row = 0;
};


#ifdef DEBUG_BUILD


#include "librocket/sync.h"
#include "bass/c/bass.h"


#define EXPORT_TRACK_NAME "track.wav"


namespace ROCKET
{
	// Sync device
	sync_device* rocket;
	// Sync callback
	sync_cb cb;
   	// Sync tracks
	const char* trackNames[] = {
		"CamX",
		"CamY",
		"CamZ",
		"Alpha",
		"Scene",
		"LightX",
		"LightY",
		"LightZ",
	};
	const struct sync_track* tracks[sizeof(trackNames) / sizeof(trackNames[0])];

#define NUM_TRACKS sizeof(trackNames) / sizeof(trackNames[0])

	bool up = false;
};

namespace BASS
{
	// Bass stream
	HSTREAM stream;
};

#endif


/* Uniform/sync stuff */


#define NUM_UNIF	9

#define UNIF_UTIME	0
#define UNIF_ALPHA	1
#define UNIF_CAMX	2
#define UNIF_CAMY	3
#define UNIF_CAMZ	4
#define UNIF_LX		5
#define UNIF_LY		6
#define UNIF_LZ		7
#define UNIF_SCENE	8

#define TRACK_CAMX	0
#define TRACK_CAMY	1
#define TRACK_CAMZ	2
#define TRACK_ALPHA 3
#define TRACK_SCENE 4
#define TRACK_LX	5
#define TRACK_LY	6
#define TRACK_LZ	7

#define ADD_UNIFORMS \
	uLoc[UNIF_UTIME] = glGetUniformLocation(hPr, "u_time"); \
	uLoc[UNIF_ALPHA] = glGetUniformLocation(hPr, "u_alpha"); \
	uLoc[UNIF_CAMX] = glGetUniformLocation(hPr, "u_x"); \
	uLoc[UNIF_CAMY] = glGetUniformLocation(hPr, "u_y"); \
	uLoc[UNIF_CAMZ] = glGetUniformLocation(hPr, "u_z"); \
	uLoc[UNIF_SCENE] = glGetUniformLocation(hPr, "u_scene"); \
	uLoc[TRACK_LX] = glGetUniformLocation(hPr, "u_lx"); \
	uLoc[TRACK_LY] = glGetUniformLocation(hPr, "u_ly"); \
	uLoc[TRACK_LZ] = glGetUniformLocation(hPr, "u_lz"); \

#define EVAL_UNIFORMS \
	glUniform1f(RENDER::uLoc[UNIF_UTIME], DEMO::time); \
	glUniform1f(RENDER::uLoc[UNIF_ALPHA], GetSyncValue(TRACK_ALPHA)); \
	glUniform1f(RENDER::uLoc[UNIF_CAMX], GetSyncValue(TRACK_CAMX)); \
	glUniform1f(RENDER::uLoc[UNIF_CAMY], GetSyncValue(TRACK_CAMY)); \
	glUniform1f(RENDER::uLoc[UNIF_CAMZ], GetSyncValue(TRACK_CAMZ)); \
	glUniform1f(RENDER::uLoc[UNIF_LX], GetSyncValue(TRACK_LX)); \
	glUniform1f(RENDER::uLoc[UNIF_LY], GetSyncValue(TRACK_LY)); \
	glUniform1f(RENDER::uLoc[UNIF_LZ], GetSyncValue(TRACK_LZ)); \
	glUniform1f(RENDER::uLoc[UNIF_SCENE], GetSyncValue(TRACK_SCENE)); \

 
/* Window defines */


namespace WINDOW
{
	HWND hWnd;
};


/* Render defines */


namespace RENDER
{
	int uLoc[NUM_UNIF]; //Uniform locations

#ifdef DEBUG_BUILD
	//bool ctrlCam = true;
	float cx, cy, cz;
#endif
};


/* Global namespace functions */


__forceinline void __fastcall Init();
LRESULT CALLBACK MainWProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#ifdef DEBUG_BUILD
void __fastcall Quit();
void __fastcall init_gl();
void __fastcall render_gl();
#else
__forceinline void __fastcall init_gl();
__forceinline void __fastcall render_gl();
#endif
