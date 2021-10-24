#pragma once

#include "d2structs.h"

#define ArraySize(x) (sizeof(x) / sizeof(x[0]))

void d2MapInit(const char *dir);

Level *__fastcall getLevel(ActMisc *misc, uint32_t levelno);
void D2CLIENT_InitGameMisc();
uint32_t D2ClientInterface();
