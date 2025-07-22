/**
 * The file is licensed under the GNU GPL v3
 * Copyright (C) Guanyuming He 2025
 *
 * The file implements the helpers in utility.h
 * @author Guanyuming He
 */

#include "utility.h"
#include "url2html.h"
#include "date_util.h"

void global_init()
{
	scraper::	global_init();
	url2html::	global_init();
	date_global_init();
}

void global_uninit()
{
	url2html::	global_uninit();
}
