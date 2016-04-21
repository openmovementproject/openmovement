// MyVideoPlayer - Stub handler for video.dll
// by Daniel Jackson, 2006

#ifndef MYVIDEOPLAYER_H
#define MYVIDEOPLAYER_H

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
