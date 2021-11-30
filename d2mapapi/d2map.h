#pragma once

#include "d2structs.h"

namespace d2mapapi {

#define ArraySize(x) (sizeof(x) / sizeof(x[0]))

/* return NULL if no error,
 * otherwise return error message */
const char *d2Init(const wchar_t *dir);

Level *__fastcall getLevel(ActMisc *misc, uint32_t levelno);
void D2CLIENT_InitGameMisc();
uint32_t D2ClientInterface();

}
