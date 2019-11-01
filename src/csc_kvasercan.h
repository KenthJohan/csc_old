#pragma once
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <canlib.h>

#include "csc_debug.h"

#define XCAN_MSGFARGS(x) (x) [0], (x) [1], (x) [2], (x) [3], (x) [4], (x) [5], (x) [6], (x) [7]
#define XCAN_MSGFORMAT "%02x %02x %02x %02x %02x %02x %02x %02x"

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

enum xcanbps
{
	XCANBPS_1M,
	XCANBPS_500K,
	XCANBPS_250K,
	XCANBPS_125K,
	XCANBPS_100K,
	XCANBPS_62K,
	XCANBPS_50K,
	XCANBPS_83K,
	XCANBPS_10K
};


static char const * xcanbps_name [] = {"1M", "500k", "250k", "125k", "100k", "62k", "50k", "83k", "10k"};


int xcanbps_convert (enum xcanbps bps)
{
	switch (bps)
	{
	case XCANBPS_1M:
	return canBITRATE_1M;
	case XCANBPS_500K:
	return canBITRATE_500K;
	case XCANBPS_250K:
	return canBITRATE_250K;
	case XCANBPS_125K:
	return canBITRATE_125K;
	case XCANBPS_100K:
	return canBITRATE_100K;
	case XCANBPS_62K:
	return canBITRATE_62K;
	case XCANBPS_50K:
	return canBITRATE_50K;
	case XCANBPS_83K:
	return canBITRATE_83K;
	case XCANBPS_10K:
	return canBITRATE_10K;
	}
	assert (0);
	return -1;
}


static void csc_kvasercan_build_name (char * name, int n, int step)
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
		assert (r == canOK);
		r = canGetChannelData (n, canCHANNELDATA_DEVDESCR_ASCII, buffer, sizeof (buffer));
		assert (r == canOK);
		r = canGetChannelData (n, canCHANNELDATA_CARD_SERIAL_NO, serial, sizeof(serial));
		assert (r == canOK);
		r = canGetChannelData (n, canCHANNELDATA_CARD_FIRMWARE_REV, fw, sizeof(fw));
		assert (r == canOK);
		r = canGetChannelData (n, canCHANNELDATA_CHANNEL_FLAGS, &flags, sizeof(flags));
		assert (r == canOK);
		TRACE_F ("flags %x", flags);
		snprintf (name, step, "%02i: %s SN%i v%i.%i.%i %s", n, buffer, serial [0], fw[1] >> 16, fw[1] & 0xffff, fw[0] & 0xffff, (flags & canCHANNEL_IS_OPEN) ? "(Opened)" : "(Available)");
		name += step;
	}
}


