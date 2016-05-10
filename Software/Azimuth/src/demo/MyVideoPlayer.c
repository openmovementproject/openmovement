// MyVideoPlayer - Stub handler for video.dll
// by Daniel Jackson, 2006

#include "MyVideoPlayer.h"


#ifdef WIN32
	#if 1
		#include <windows.h>
		#define hmodule_t HMODULE
	#else
		// Avoids including windows.h
		struct hmodule_s { int unused; }; 
		typedef struct hmodule_s *hmodule_t;
		extern hmodule_t LoadLibraryA(const char *lpLibFilename);
		extern int FreeLibrary(hmodule_t hModule);
		extern void *GetProcAddressA(hmodule_t hModule, const char *lpProcName); 
		#define GetProcAddress GetProcAddressA
	#endif
	#define VIDEO_ENABLED
#endif

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

// Should be in header
#ifndef MYVIDEOPLAYER_H
	typedef void Video;
	Video *video_new();
	void video_delete(Video *video);
	int video_open(Video *video, const char *filename);
	int video_play(Video *video);
	int video_pause(Video *video);
	int video_seek(Video *video, unsigned long time);
	int video_getSampleData(Video *video, unsigned char *buffer);
	int video_getSampleSize(Video *video);
	int video_getSampleWidth(Video *video);
	int video_getSampleHeight(Video *video);
	double video_getSampleTime(Video *video);
#endif



#ifdef VIDEO_ENABLED
	// external function types
	typedef Video * (* video_t_newVideo       )(void);
	typedef void    (* video_t_deleteVideo    )(Video *);
	typedef int     (* video_t_openVideo      )(Video *, const char *);
	typedef int     (* video_t_playVideo      )(Video *);
	typedef int     (* video_t_pauseVideo     )(Video *);
	typedef int     (* video_t_seekVideo      )(Video *, unsigned long);
	typedef int     (* video_t_getSampleData  )(Video *, unsigned char *);
	typedef int     (* video_t_getSampleSize  )(Video *);
	typedef int     (* video_t_getSampleWidth )(Video *);
	typedef int     (* video_t_getSampleHeight)(Video *);
	typedef double  (* video_t_getSampleTime  )(Video *);

	// MyVideoPlayer
	static hmodule_t hVideoModule = NULL;

	// external function pointers
	static video_t_newVideo        video_x_newVideo        = NULL;
	static video_t_deleteVideo     video_x_deleteVideo     = NULL;
	static video_t_openVideo       video_x_openVideo       = NULL;
	static video_t_playVideo       video_x_playVideo       = NULL;
	static video_t_pauseVideo      video_x_pauseVideo      = NULL;
	static video_t_seekVideo       video_x_seekVideo       = NULL;
	static video_t_getSampleData   video_x_getSampleData   = NULL;
	static video_t_getSampleSize   video_x_getSampleSize   = NULL;
	static video_t_getSampleWidth  video_x_getSampleWidth  = NULL;
	static video_t_getSampleHeight video_x_getSampleHeight = NULL;
	static video_t_getSampleTime   video_x_getSampleTime   = NULL;
#endif


Video *video_new()
{
#ifdef VIDEO_ENABLED
	int errors = 0;
	hVideoModule = (hmodule_t)LoadLibraryA("VideoDll.dll");
	if (hVideoModule == NULL)
	{
		fprintf(stderr, "VideoDll.dll not found\n");
		return NULL;
	}
	
	if ((video_x_newVideo        = (video_t_newVideo       )GetProcAddress(hVideoModule, "newVideo"       )) == NULL) { errors++; };
	if ((video_x_deleteVideo     = (video_t_deleteVideo    )GetProcAddress(hVideoModule, "deleteVideo"    )) == NULL) { errors++; };
	if ((video_x_openVideo       = (video_t_openVideo      )GetProcAddress(hVideoModule, "openVideo"      )) == NULL) { errors++; };
	if ((video_x_playVideo       = (video_t_playVideo      )GetProcAddress(hVideoModule, "playVideo"      )) == NULL) { errors++; };
	if ((video_x_pauseVideo      = (video_t_pauseVideo     )GetProcAddress(hVideoModule, "pauseVideo"     )) == NULL) { errors++; };
	if ((video_x_seekVideo       = (video_t_seekVideo      )GetProcAddress(hVideoModule, "seekVideo"      )) == NULL) { errors++; };
	if ((video_x_getSampleData   = (video_t_getSampleData  )GetProcAddress(hVideoModule, "getSampleData"  )) == NULL) { errors++; };
	if ((video_x_getSampleSize   = (video_t_getSampleSize  )GetProcAddress(hVideoModule, "getSampleSize"  )) == NULL) { errors++; };
	if ((video_x_getSampleWidth  = (video_t_getSampleWidth )GetProcAddress(hVideoModule, "getSampleWidth" )) == NULL) { errors++; };
	if ((video_x_getSampleHeight = (video_t_getSampleHeight)GetProcAddress(hVideoModule, "getSampleHeight")) == NULL) { errors++; };
	if ((video_x_getSampleTime   = (video_t_getSampleTime  )GetProcAddress(hVideoModule, "getSampleTime"  )) == NULL) { errors++; };

	if (errors > 0)
	{
		fprintf(stderr, "%d functions not found while loading VideoDll.dll\n", errors);
		return NULL;
	}
	
	return video_x_newVideo();
#else
	return 0;
#endif
}

void video_delete(Video *video)
{
#ifdef VIDEO_ENABLED
	if (video_x_deleteVideo != NULL && video != NULL) { video_x_deleteVideo(video); }
	FreeLibrary(hVideoModule);
#endif
	return;
}

int video_open(Video *video, const char *filename)
{
#ifdef VIDEO_ENABLED
	if (video_x_openVideo != NULL && video != NULL) { return video_x_openVideo(video, filename); }
#endif
	return 0;
}

int video_play(Video *video)
{
#ifdef VIDEO_ENABLED
	if (video_x_playVideo != NULL && video != NULL) { return video_x_playVideo(video); }
#endif
	return 0;
}

int video_pause(Video *video)
{
#ifdef VIDEO_ENABLED
	if (video_x_pauseVideo != NULL && video != NULL) { return video_x_pauseVideo(video); }
#endif
	return 0;
}

int video_seek(Video *video, unsigned long time)
{
#ifdef VIDEO_ENABLED
	if (video_x_seekVideo != NULL && video != NULL) { return video_x_seekVideo(video, time); }
#endif
	return 0;
}

int video_getSampleData(Video *video, unsigned char *buffer)
{
#ifdef VIDEO_ENABLED
	if (video_x_getSampleData != NULL && video != NULL) { return video_x_getSampleData(video, buffer); }
#endif
	return 0;
}

int video_getSampleSize(Video *video)
{
#ifdef VIDEO_ENABLED
	if (video_x_getSampleSize != NULL && video != NULL) { return video_x_getSampleSize(video); }
#endif
	return 0;
}

int video_getSampleWidth(Video *video)
{
#ifdef VIDEO_ENABLED
	if (video_x_getSampleWidth != NULL && video != NULL) { return video_x_getSampleWidth(video); }
#endif
	return 0;
}

int video_getSampleHeight(Video *video)
{
#ifdef VIDEO_ENABLED
	if (video_x_getSampleHeight != NULL && video != NULL) { return video_x_getSampleHeight(video); }
#endif
	return 0;
}

double video_getSampleTime(Video *video)
{
#ifdef VIDEO_ENABLED
	if (video_x_getSampleTime != NULL && video != NULL) { return video_x_getSampleTime(video); }
#endif
	return 0;
}

