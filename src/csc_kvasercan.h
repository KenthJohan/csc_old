/*
MIT License

Copyright (c) 2019 CSC Project

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <canlib.h>

#include "csc_debug.h"

#define CSC_KVASERCAN_MSGFARGS(x) (x) [0], (x) [1], (x) [2], (x) [3], (x) [4], (x) [5], (x) [6], (x) [7]
#define CSC_KVASERCAN_MSGFORMAT "%02x %02x %02x %02x %02x %02x %02x %02x"

#define CSC_KVASERCAN_ASSERT(exp) csc_kvasercan_assert(exp, __COUNTER__, __RELATIVE_FILE__, __LINE__, __func__, #exp)

void csc_kvasercan_assert
(
	canStatus status,
	int id,
	char const * file,
	int line,
	char const * fn,
	char const * exp
)
{
	if (status < 0)
	{
		char msg [128] = {'\0'};
		canStatus r = canGetErrorText (status, msg, sizeof (msg));
		ASSERT (r == canOK);
		fprintf (stderr, ASSERT_TCOL0 "ASSERT " ASSERT_TCOL1 "[%04i]" TCOL_RST " ", id);
		fprintf (stderr, ASSERT_TCOL2 "%s" TCOL_RST ":", file);
		fprintf (stderr, ASSERT_TCOL3 "%04i" TCOL_RST " in ", line);
		fprintf (stderr, ASSERT_TCOL4 "%s" TCOL_RST " () ", fn);
		fprintf (stderr, ASSERT_TCOL5 "[%s]" TCOL_RST " ", exp);
		fprintf (stderr, "[%04i:" ASSERT_TCOL6 "%s" TCOL_RST "]: ", status, msg);
		fprintf (stderr, "\n");
		fflush (stderr);
		exit (EXIT_FAILURE);
	}
}

enum csc_kvasercan_bps
{
	CSC_KVASERCAN_BPS_1M,
	CSC_KVASERCAN_BPS_500K,
	CSC_KVASERCAN_BPS_250K,
	CSC_KVASERCAN_BPS_125K,
	CSC_KVASERCAN_BPS_100K,
	CSC_KVASERCAN_BPS_62K,
	CSC_KVASERCAN_BPS_50K,
	CSC_KVASERCAN_BPS_83K,
	CSC_KVASERCAN_BPS_10K
};


static char const * csc_kvaser_bps_name [] = {"1M", "500k", "250k", "125k", "100k", "62k", "50k", "83k", "10k"};


int csc_kvasercan_bps_convert (enum csc_kvasercan_bps bps)
{
	switch (bps)
	{
	case CSC_KVASERCAN_BPS_1M:
	return canBITRATE_1M;
	case CSC_KVASERCAN_BPS_500K:
	return canBITRATE_500K;
	case CSC_KVASERCAN_BPS_250K:
	return canBITRATE_250K;
	case CSC_KVASERCAN_BPS_125K:
	return canBITRATE_125K;
	case CSC_KVASERCAN_BPS_100K:
	return canBITRATE_100K;
	case CSC_KVASERCAN_BPS_62K:
	return canBITRATE_62K;
	case CSC_KVASERCAN_BPS_50K:
	return canBITRATE_50K;
	case CSC_KVASERCAN_BPS_83K:
	return canBITRATE_83K;
	case CSC_KVASERCAN_BPS_10K:
	return canBITRATE_10K;
	}
	assert (0);
	return -1;
}


static void csc_kvasercan_build_name (char * name, int n, size_t step)
{
	canStatus r;
	while (n--)
	{
		char buffer [100];
		unsigned int serial [2];
		unsigned int fw [2];
		uint32_t ui;
		uint32_t flags;
		r = canGetChannelData (n, canCHANNELDATA_UI_NUMBER, &ui, sizeof (ui));
		ASSERT (r == canOK);
		r = canGetChannelData (n, canCHANNELDATA_DEVDESCR_ASCII, buffer, sizeof (buffer));
		ASSERT (r == canOK);
		r = canGetChannelData (n, canCHANNELDATA_CARD_SERIAL_NO, serial, sizeof(serial));
		ASSERT (r == canOK);
		r = canGetChannelData (n, canCHANNELDATA_CARD_FIRMWARE_REV, fw, sizeof(fw));
		ASSERT (r == canOK);
		r = canGetChannelData (n, canCHANNELDATA_CHANNEL_FLAGS, &flags, sizeof(flags));
		ASSERT (r == canOK);
		TRACE_F ("flags %x", flags);
		snprintf (name, step, "%02i: %s SN%i v%i.%i.%i %s", n, buffer, serial [0], fw[1] >> 16, fw[1] & 0xffff, fw[0] & 0xffff, (flags & canCHANNEL_IS_OPEN) ? "(Opened)" : "(Available)");
		name += step;
	}
}


