/* Sync stuff */


#pragma once


static const float bpm = 210.0f;
static const int rpb = 8;
static const double row_rate = (double(bpm) / 60) * rpb;


#ifdef DEBUG_BUILD


/* Rocket stuff */


#include "librocket/sync.h"
#include "bass/c/bass.h"

#include "def.hh"


void __fastcall InitRocket();
void PauseRocket(void *d, int flag);
void SetRocketTime(void *d, int row);
int IsRocketRunning(void *d);
void __fastcall Play();
double bass_get_row(HSTREAM h);
void __fastcall UpdateRocket();


void __fastcall InitRocket()
{
	using namespace ROCKET;

	rocket = sync_create_device("s");

	if (!rocket)
		DEMO::Die(ERR_INIT_SYNC);

	if (sync_connect(rocket, "localhost", SYNC_DEFAULT_PORT))
		DEMO::Die(ERR_INIT_SYNC);

	for (int i = 0; i < NUM_TRACKS; i++)
		tracks[i] = sync_get_track(rocket, trackNames[i]);

	ROCKET::cb = {
		PauseRocket,
		SetRocketTime,
		IsRocketRunning,
	};
}

void PauseRocket(void *d, int flag)
{
	HSTREAM h = *((HSTREAM *)d);
	if (flag) BASS_ChannelPause(h);
	else BASS_ChannelPlay(h, false);
}

void SetRocketTime(void *d, int row)
{
	HSTREAM h = *((HSTREAM *)d);
	QWORD pos = BASS_ChannelSeconds2Bytes(h, row / row_rate);
	BASS_ChannelSetPosition(h, pos, BASS_POS_BYTE);
}

int IsRocketRunning(void *d)
{
	HSTREAM h = *((HSTREAM *)d);
   
	ROCKET::up = BASS_ChannelIsActive(h) == BASS_ACTIVE_PLAYING;
	return ROCKET::up;
}

void __fastcall Play()
{
	BASS_Start();
	BASS_ChannelPlay(BASS::stream, false);
}

double bass_get_row(HSTREAM h)
{
	QWORD pos = BASS_ChannelGetPosition(h, BASS_POS_BYTE);
	double time = BASS_ChannelBytes2Seconds(h, pos);
	DEMO::time = time;
	return time * row_rate;
}

void __fastcall UpdateRocket()
{
	using namespace DEMO;

	row = bass_get_row(BASS::stream);
	if (sync_update(ROCKET::rocket, (int)floor(row), &ROCKET::cb, (void *)&BASS::stream))
		Die();
}

__forceinline float __fastcall GetSyncValue(unsigned char index)
{
	return sync_get_val(ROCKET::tracks[index], DEMO::row);
}

#else


namespace SYNC_DATA
{
	float data[NUM_TRACKS][NUM_ROWS];
}


/* Sync functions */


#include "def.hh"
#include "auto_sync_data.h"

#include <cmath>


using namespace SYNC_DATA;


// Precalculate single interpolation //
__forceinline void __fastcall inter_sync(uint16_t index)
{
	//printf("--- Interpolating at index %i ---\n", index);

	int i = index + 1;
	int inter = sync_data[index].inter;
	// If no value found, keep current one
	float next_val = sync_data[index].value;

	// Get next value
	while (i < NUM_EVENTS)
	{
		if (sync_data[i].time > sync_data[index].time && sync_data[i].value != 0 && sync_data[i].track == sync_data[index].track) {
			next_val = sync_data[i].value;
			break;
		}
		i++;
	}

	//printf("Current value: Track(%i), Value(%f), Row(%i)\n", sync_data[index].track, sync_data[index].value, sync_data[index].time);
	//printf("Next value: Track(%i), Value(%f), Row(%i)\n", sync_data[i].track, next_val, sync_data[i].time);

	// Start at current row, but don't override its value
	float it = sync_data[index].time + 1;

	//printf("Interpolating from row %i to row %i, start value is %f, end value is %f\n", it, sync_data[i].time, sync_data[index].value, sync_data[i].value);

	// Interpolate until row of next value reached or end was hit
	while (it < sync_data[i].time && it < NUM_ROWS)
	{
		if (inter == INTER_LINEAR) {
			float t = (it - sync_data[index].time) / (sync_data[i].time  - sync_data[index].time);
			data[sync_data[index].track][(int)it] = sync_data[index].value + (sync_data[i].value - sync_data[index].value) * t;
		}
		else if (inter == INTER_SMOOTH) {
			float t = (it - sync_data[index].time) / (sync_data[i].time - sync_data[index].time);
			t = t * t * (3 - 2 * t);
			data[sync_data[index].track][(int)it] = sync_data[index].value + (sync_data[i].value - sync_data[index].value) * t;
		}
		else if (inter == INTER_RAMP) {
			float t = (it - sync_data[index].time) / (sync_data[i].time - sync_data[index].time);
			t = pow(t, 2.0);
			data[sync_data[index].track][(int)it] = sync_data[index].value + (sync_data[i].value - sync_data[index].value) * t;
		}
		else {
			// No interpolation, keep value until next one
			data[sync_data[index].track][(int)it] = sync_data[index].value;
		}

		//printf(" - Interpolation step at row %i: %f\n", (int)it, data[sync_data[index].track][(int)it]);

		it++;
	}
}

__forceinline void __fastcall PrecalcSyncData()
{
	for (int i = 0; i < NUM_EVENTS; i++)
	{
		data[sync_data[i].track][sync_data[i].time] = sync_data[i].value;
		inter_sync(i);
	}

	for (int r = 0; r < NUM_ROWS; r++)
	{
		for (int t = 0; t < NUM_TRACKS; t++)
		{
			printf("%f  ", data[t][r]);
		}
		printf("\n");
	}
}

__forceinline float __fastcall GetSyncValue(uint16_t index)
{
	if (DEMO::row >= NUM_ROWS)
		return data[index][NUM_ROWS - 1];
	return data[index][DEMO::row];
}

#endif
