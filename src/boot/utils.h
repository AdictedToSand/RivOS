#pragma once
#include "term/term.h"

#define panic(msg) { clearTerm(); print("(%s) -> (%s) -> (line %i) -> %s", __FILE__, __FUNCTION__, __LINE__, msg); for (;;) ;}
