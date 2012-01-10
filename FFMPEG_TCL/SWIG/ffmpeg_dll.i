/* File : example.i */
%module ffmpeg_dll

%{
#include "..\\FFMpegInit.h"
%}

/* Let's just grab the original header file here */
%include "..\\FFMpegInit.h"

