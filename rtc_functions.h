#ifndef RTC_H
#define RTC_H

#include "types.h"

void InitializeRTC(void);
void FetchTime(i16 *,i16 *,i16 *);
void PrintTime(ui32,ui32,ui32);
void FetchDate(i16 *,i16 *,i16 *);
void PrintDate(ui32,ui32,ui32);
void PrintONTime(ui32, ui32,ui32);
void PrintOFFTime(ui32,ui32,ui32);

void UpdateRTCTime(ui32,ui32,ui32);
void UpdateRTCDate(ui32,ui32,ui32);

void FetchDay(i16 *);
void PrintDay(ui32);
void UpdateRTCDay(ui32);

#endif
