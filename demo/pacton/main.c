#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>

#include <iup.h>
#include <iupcontrols.h>

#include <canlib.h>

#include "csc_kvasercan.h"
#include "csc_basic.h"
#include "csc_iup.h"
#include "csc_pacton.h"
#include "csc_str.h"




#define CAN_NAME_STEP 64
#define CAN_NAME_COUNT 10



enum main_column
{
	MAIN_COLUMN_NUMBER,
	MAIN_COLUMN_BLOCK,
	MAIN_COLUMN_BLOCKSIZE,
	MAIN_COLUMN_INDEX,
	MAIN_COLUMN_SUBINDEX,
	MAIN_COLUMN_DATA,
	MAIN_COLUMN_NAME,
	MAIN_COLUMN_BYTEOFFSET,
	MAIN_COLUMN_BITOFFSET,
	MAIN_COLUMN_TYPE_BYTEORDER,
	MAIN_COLUMN_TYPE_SIZE,
	MAIN_COLUMN_TYPE_PRIMTYPE,
	MAIN_COLUMN_DIM,
	MAIN_COLUMN_VALUE,
	MAIN_COLUMN__N
};


char const * main_column_tostr (enum main_column col)
{
	switch (col)
	{
	case MAIN_COLUMN_NUMBER: return "Number";
	case MAIN_COLUMN_NAME: return "Name";
	case MAIN_COLUMN_BLOCK: return "Block";
	case MAIN_COLUMN_BLOCKSIZE: return "Size";
	case MAIN_COLUMN_INDEX: return "Index";
	case MAIN_COLUMN_SUBINDEX: return "Subi";
	case MAIN_COLUMN_BYTEOFFSET: return "Boff";
	case MAIN_COLUMN_BITOFFSET: return "boff";
	case MAIN_COLUMN_TYPE_BYTEORDER: return "Order";
	case MAIN_COLUMN_TYPE_SIZE: return "Size";
	case MAIN_COLUMN_TYPE_PRIMTYPE: return "Type";
	case MAIN_COLUMN_DIM: return "Dim";
	case MAIN_COLUMN_DATA: return "Data";
	case MAIN_COLUMN_VALUE: return "Value";
	default:return "";
	}
}


static struct pacton_block allblock = {0};
static struct pacton_value alldata = {0};


/*
Action generated to retrieve the value of a cell.
Called both for common cells and for line and column titles.
*/
static char * callback_matrix_value (Ihandle *self, int lin, int col)
{
	(void)self;
	static char text[1024];
	memset (text, 0, sizeof(text));
	if (lin == 0 && col == 0)
	{
		return "";
	}
	else if (lin == 0)
	{
		snprintf (text, sizeof (text), "%s", main_column_tostr ((enum main_column) col));
		return text;
	}
	else if (col == 0)
	{
		snprintf (text, sizeof (text), "%d", lin - 1);
		return text;
	}
	else if (lin >= 1 && (lin-1) < (int)alldata.n)
	{
		uint32_t i = (uint32_t)lin - 1;
		assert (i < alldata.n);
		uint32_t block = alldata.block[i];
		assert (block < allblock.n);
		char * value_name0 = alldata.names0 + PACTON_VALUE_NAMES0_STEP * i;
		char * block_name0 = allblock.names0 + PACTON_BLOCK_NAMES0_STEP * block;
		//char * block_name1 = allblock.names1 + allblock.names1_step * block;
		uint32_t block_size = allblock.data_size [block];
		uint32_t block_index = allblock.index [block];
		uint32_t block_subindex = allblock.subindex [block];
		uint32_t value_bytepos = alldata.bytepos[i];
		uint32_t value_bitpos = alldata.bitpos[i];
		uint32_t value_type = alldata.type[i];
		uint32_t value_dim = alldata.dim[i];

		switch (col)
		{
		case MAIN_COLUMN_NAME:
			snprintf(text, sizeof (text), "%s", value_name0);
			break;
		case MAIN_COLUMN_BLOCK:
			snprintf(text, sizeof (text), "%s", block_name0);
			break;
		case MAIN_COLUMN_BLOCKSIZE:
			snprintf(text, sizeof (text), "%i", (int)block_size);
			break;
		case MAIN_COLUMN_INDEX:
			snprintf(text, sizeof (text), "0x%x", block_index);
			break;
		case MAIN_COLUMN_SUBINDEX:
			snprintf(text, sizeof (text), "%i", block_subindex);
			break;
		case MAIN_COLUMN_BYTEOFFSET:
			snprintf(text, sizeof (text), "%i", value_bytepos);
			break;
		case MAIN_COLUMN_BITOFFSET:
			snprintf(text, sizeof (text), "%i", value_bitpos);
			break;
		case MAIN_COLUMN_TYPE_BYTEORDER:
			snprintf(text, sizeof (text), "%s", PACTON_TYPE_BYTEORDER(value_type)?"BE":"LE");
			break;
		case MAIN_COLUMN_TYPE_SIZE:
			snprintf(text, sizeof (text), "%i", PACTON_TYPE_SIZE(value_type));
			break;
		case MAIN_COLUMN_TYPE_PRIMTYPE:
			snprintf(text, sizeof (text), "%s", pacton_primtype_tostr (PACTON_TYPE_PRIMTYPE(value_type)));
			break;
		case MAIN_COLUMN_DIM:
			snprintf(text, sizeof (text), "%i", value_dim);
			break;
		case MAIN_COLUMN_DATA:{
			uint8_t * v = allblock.data + block * PACTON_BLOCK_DATA_STEP;
			size_t vn = allblock.data_size [block];
			char fmt [] = "%02X ";
			csc_str_print_hex_array (text, sizeof (text), v, vn, fmt, sizeof (fmt));
			break;}
		case MAIN_COLUMN_VALUE:{
			uint8_t * v = allblock.data + block * PACTON_BLOCK_DATA_STEP;
			v += value_bytepos;
			unsigned int mask = (1 << PACTON_TYPE_SIZE(value_type)) - 1;
			if (value_bitpos || value_dim == 1)
			{
				snprintf (text, sizeof (text), "%02X ", ((*v) >> value_bitpos) & mask);
			}
			//uint8_t v = allblock.data [alldata[i].block][alldata[i].bytepos];
			//snprintf(text, sizeof (text), "%i", v);
			break;}
		}
	}

	return text;
}


/*
Action generated to notify the application that the value of a cell was changed. Never called when READONLY=YES.
This callback is usually set in callback mode, but also works in normal mode.
When in normal mode, it is called after the new value has been internally stored, so to refuse the new value simply reset the cell to the desired value.
When it was interactively changed the temporary attribute CELL_EDITED will be set to Yes during the callback (since 3.13).
*/
static int callback_matrix_value_edit (Ihandle *self, int lin, int col, char* newvalue)
{
	(void)self;
	TRACE_F ("%i:%i = %s", lin, col, newvalue);
	if (col >= 1 && lin >= 1)
	{
		int i = lin - 1;
		if (i < (int)alldata.n)
		{
			uint32_t block = alldata.block[i];
			uint32_t bytepos = alldata.bytepos[i];
			uint32_t bitpos = alldata.bitpos[i];
			uint32_t type = alldata.type[i];
			char * e = NULL;
			errno = 0;
			intmax_t v;
			switch (col)
			{
			case 1:
				break;
			case 2:
				break;
			case MAIN_COLUMN_INDEX:
				v = strtoimax (newvalue, &e, 16);
				allblock.index [block] = (uint32_t)v;
				assert (errno == 0);
				break;
			case MAIN_COLUMN_SUBINDEX:
				v = strtoimax (newvalue, &e, 10);
				allblock.subindex [block] = (uint32_t)v;
				assert (errno == 0);
				break;
			case MAIN_COLUMN_BYTEOFFSET:
				v = strtoimax (newvalue, &e, 10);
				alldata.bytepos[i] = (uint32_t)v;
				assert (errno == 0);
				break;
			case MAIN_COLUMN_BITOFFSET:
				v = strtoimax (newvalue, &e, 10);
				alldata.bitpos[i] = (uint32_t)v;
				assert (errno == 0);
				break;
			case MAIN_COLUMN_DIM:
				v = strtoimax (newvalue, &e, 10);
				alldata.dim[i] = (uint32_t)v;
				assert (errno == 0);
				break;
			case MAIN_COLUMN_TYPE_SIZE:
				v = strtoimax (newvalue, &e, 10);
				alldata.type[i] &= ~((uint32_t)0xFF << 8);
				alldata.type[i] |= PACTON_TYPE (0, v, 0);
				assert (errno == 0);
				break;
			case MAIN_COLUMN_NAME:
				strncpy (alldata.names0 + i * PACTON_VALUE_NAMES0_STEP, newvalue, PACTON_VALUE_NAMES0_STEP);
				break;
			case MAIN_COLUMN_VALUE:{
				v = strtoimax (newvalue, &e, 16);
				unsigned int mask = (1 << PACTON_TYPE_SIZE(type)) - 1;
				uint8_t * data = allblock.data + block * PACTON_BLOCK_DATA_STEP + bytepos;
				data [0] &= ~(mask << bitpos);
				data [0] |= ((v & mask) << bitpos);
				assert (errno == 0);
				break;}
			}
		}
	}
	return IUP_DEFAULT;
}


/*
Action generated before the current cell enters edition mode to determine if a text field or a dropdown list will be shown.
It is called after EDITION_CB. If this action is not registered, a text field will be shown.
Its return determines what type of element will be used in the edition mode.
If the selected type is a dropdown, the values appearing in the dropdown must be fulfilled in this callback,
just like elements are added to any list (the drop parameter is the handle of the dropdown list to be shown).
You should also set the lists current value ("VALUE"), the default is always "1".
The previously cell value can be verified from the given drop Ihandle via the "PREVIOUSVALUE" attribute.
*/
static int callback_drop (Ihandle *self, Ihandle *drop, int lin, int col)
{
	TRACE_F ("%i:%i", lin, col);
	(void)self;
	if (lin == 0) {return IUP_IGNORE;}
	switch (col)
	{
	case MAIN_COLUMN_BLOCK:
		//printf ("lin: %i %i\n", lin-1, alldata [lin-1].block);
		//TODO: Figureout why we need to do +2 and not +1 here:
		IupSetInt (drop, "VALUE", (int)alldata.block[lin-1]+2);
		csc_Iup_set_string_list (drop, allblock.names0, PACTON_BLOCK_NAMES0_STEP, allblock.n);
		return IUP_DEFAULT;
	case MAIN_COLUMN_TYPE_PRIMTYPE:{
		enum pacton_type t = alldata.type[lin-1];
		IupSetInt (drop, "VALUE", (int)PACTON_TYPE_PRIMTYPE(t)+2);
		for (int i = 0; i < PACTON_PRIMTYPE__N; ++i)
		{
			char const * name = pacton_primtype_tostr ((enum pacton_primtype) i);
			TRACE_F ("name: %s", name);
			IupSetAttributeId (drop, "", (int)i+1, name);
		}
		return IUP_DEFAULT;}
	}
	return IUP_IGNORE;
}


/*
Action generated when an element in the dropdown list or the popup menu is selected.
For the dropdown, if returns IUP_CONTINUE the value is accepted as a new value and the matrix leaves edition mode,
else the item is selected and editing remains. For the popup menu the returned value is ignored.
*/
static int callback_dropselect (Ihandle *self, int lin, int col, Ihandle *drop, char *t, int i, int v)
{
	(void)self;
	(void)drop;
	TRACE_F ("%i:%i, %s, i=%d, v=%d", lin, col, t, i, v);
	if (lin == 0 || v == 0) {return IUP_IGNORE;}
	switch (col)
	{
	case MAIN_COLUMN_BLOCK:
		assert (i >= 0);
		alldata.block[lin-1] = (uint32_t)i - 1;
		return IUP_DEFAULT;
	case MAIN_COLUMN_TYPE_PRIMTYPE:
		assert (i >= 0);
		alldata.type[lin-1] &= ~((uint32_t)0xFF << 16);
		alldata.type[lin-1] |= PACTON_TYPE (0, 0, (uint32_t)i - 1);
		return IUP_DEFAULT;
	}
	return IUP_CONTINUE;
	// return IUP_DEFAULT;
}


/*
Action generated before the current cell is redrawn to determine if a dropdown/popup menu feedback or a toggle should be shown.
If this action is not registered, no feedback will be shown.
If the callback is defined and return IUP_DEFAULT for a cell,
to show the dropdown/popup menu the user can simply do a single click in the drop feedback area of that cell.
(Toggle support since 3.9)
*/
static int callback_dropcheck (Ihandle *self, int lin, int col)
{
	(void)self;
	if (lin == 0) {return IUP_IGNORE;}
	switch (col)
	{
	case MAIN_COLUMN_BLOCK:
	case MAIN_COLUMN_TYPE_PRIMTYPE:
		return IUP_DEFAULT;
	}
	return IUP_IGNORE;
}



static int callback_export_block (Ihandle *self)
{
	TRACE_F ("Export block %s", IupGetAttribute (self, "TITLE"));
	pacton_block_tofile (&allblock, "../packaton/block.txt");
	return IUP_DEFAULT;
}


static int callback_export_value (Ihandle *self)
{
	TRACE_F ("Export block %s", IupGetAttribute (self, "TITLE"));
	pacton_value_tofilename (&alldata, "../packaton/value.txt");
	return IUP_DEFAULT;
}


static int callback_select_can_device ()
{
	Ihandle * menu = IupGetHandle ("menu_selectcan");
	assert (menu);
	Ihandle * items = IupGetChild (menu, 0);
	assert (items);
	IupDestroy_children (items);
	canStatus r;
	int n = 0;
	r = canGetNumberOfChannels (&n);
	assert (r == canOK);
	TRACE_F ("canGetNumberOfChannels %i", n);
	assert (n < CAN_NAME_COUNT);
	char buffer [CAN_NAME_COUNT*CAN_NAME_STEP] = {'\0'};
	csc_kvasercan_build_name (buffer, n, CAN_NAME_STEP);
	IupItem_str1_append (items, buffer, n, CAN_NAME_STEP, "KVASER_CAN_CHANNEL", "select_can");
	return IUP_DEFAULT;
}


static int callback_select_can (Ihandle *self)
{
	Ihandle * menu_selectcan = IupGetHandle ("menu_selectcan");
	ASSERT (menu_selectcan);
	int channel = IupGetInt (self, "KVASER_CAN_CHANNEL");
	TRACE_F ("Select CAN channel %i", channel);
	IupSetStrf (menu_selectcan, "TITLE", "Device %02i", channel);
	canHandle handle = canOpenChannel (channel, canOPEN_ACCEPT_VIRTUAL | canOPEN_EXCLUSIVE);
	TRACE_F ("canOpenChannel %i", handle);
	CSC_KVASERCAN_ASSERT (handle);
	canStatus r;
	unsigned int tseg1 = 5;
	unsigned int tseg2 = 2;
	unsigned int sjw = 1;
	r = canSetBusParams (handle, csc_kvasercan_bps_convert (CSC_KVASERCAN_BPS_250K), tseg1, tseg2, sjw, 1, 0);
	CSC_KVASERCAN_ASSERT (r);
	r = canBusOn (handle);
	CSC_KVASERCAN_ASSERT (r);
	return IUP_DEFAULT;
}


int main (int argc, char **argv)
{
	setbuf (stdout, NULL);
	allblock.n = 19;
	allblock.names0 = calloc (allblock.n, PACTON_BLOCK_NAMES0_STEP);
	allblock.names1 = calloc (allblock.n, PACTON_BLOCK_NAMES1_STEP);
	allblock.index = calloc (allblock.n, sizeof (uint32_t));
	allblock.subindex = calloc (allblock.n, sizeof (uint32_t));
	allblock.data_size = calloc (allblock.n, sizeof (uint32_t));
	allblock.data = calloc (allblock.n, PACTON_BLOCK_DATA_STEP);
	pacton_block_fromfile (&allblock, "../pacton/block.txt");

	alldata.n = 20;
	alldata.names0 = calloc (alldata.n, PACTON_VALUE_NAMES0_STEP);
	alldata.block = calloc (alldata.n, sizeof (uint32_t));
	alldata.bytepos = calloc (alldata.n, sizeof (uint32_t));
	alldata.bitpos = calloc (alldata.n, sizeof (uint32_t));
	alldata.dim = calloc (alldata.n, sizeof (uint32_t));
	alldata.type = calloc (alldata.n, sizeof (uint32_t));
	pacton_value_fromfile (&alldata, "../pacton/value.txt");

	canInitializeLibrary ();
	IupOpen (&argc, &argv);
	IupControlsOpen ();

	Ihandle * matrix = IupMatrix (NULL);
	Ihandle * dlg = IupDialog (matrix);
	Ihandle * menu_export_items = IupMenu (IupItem ("Export block","export_block"), IupItem ("Export value","export_value"), NULL);
	Ihandle * menu_export = IupSubmenu ("Export", menu_export_items);
	Ihandle * menu_selectcan_items = IupMenu (NULL);
	Ihandle * menu_selectcan = IupSubmenu ("Select CAN device", menu_selectcan_items);
	Ihandle * menu = IupMenu (menu_export, menu_selectcan, NULL);

	//Set handle names for global access:
	IupSetHandle ("menu_selectcan", menu_selectcan);
	IupSetHandle ("mymenu", menu);
	IupSetHandle ("mymatrix", dlg);

	//Setup callbacks:
	IupSetCallback (matrix, "VALUE_CB", (Icallback)callback_matrix_value);
	IupSetCallback (matrix, "VALUE_EDIT_CB", (Icallback)callback_matrix_value_edit);
	IupSetCallback (matrix, "DROP_CB",(Icallback)callback_drop);
	IupSetCallback (matrix, "DROPCHECK_CB",(Icallback)callback_dropcheck);
	IupSetCallback (matrix, "DROPSELECT_CB",(Icallback)callback_dropselect);
	IupSetCallback (menu_selectcan_items, "OPEN_CB",(Icallback)callback_select_can_device);
	IupSetFunction ("export_block", (Icallback)callback_export_block);
	IupSetFunction ("export_value", (Icallback)callback_export_value);
	IupSetFunction ("select_can", (Icallback)callback_select_can);

	//Matrix configure:
	IupSetAttribute(matrix, "NAME", "MATRIX");
	IupSetInt (matrix, "NUMCOL", MAIN_COLUMN__N);
	IupSetInt (matrix, "NUMLIN", 8);
	IupSetInt (matrix, "NUMCOL_VISIBLE", MAIN_COLUMN__N);
	IupSetInt (matrix, "NUMLIN_VISIBLE", 8);
	IupSetIntId (matrix, "WIDTH", MAIN_COLUMN_NUMBER, 15);
	IupSetIntId (matrix, "WIDTH", MAIN_COLUMN_NAME, 100);
	IupSetIntId (matrix, "WIDTH", MAIN_COLUMN_BLOCK, 100);
	IupSetIntId (matrix, "WIDTH", MAIN_COLUMN_DATA, 100);
	IupSetInt (matrix, "WIDTHDEF", 30);
	IupSetIntId (matrix, "HEIGHT", 0, 8);
	IupSetAttribute (matrix, "RESIZEMATRIX", "Yes");
	IupSetAttributeId2 (matrix, "BGCOLOR", IUP_INVALID_ID, MAIN_COLUMN_BLOCK, "230 230 255");
	IupSetAttributeId2 (matrix, "BGCOLOR", IUP_INVALID_ID, MAIN_COLUMN_INDEX, "230 230 255");
	IupSetAttributeId2 (matrix, "BGCOLOR", IUP_INVALID_ID, MAIN_COLUMN_BLOCKSIZE, "230 230 255");
	IupSetAttributeId2 (matrix, "BGCOLOR", IUP_INVALID_ID, MAIN_COLUMN_SUBINDEX, "230 230 255");
	IupSetAttributeId2 (matrix, "BGCOLOR", IUP_INVALID_ID, MAIN_COLUMN_DATA, "230 230 255");

	//Dialog configure:
	IupSetAttribute (dlg, "TITLE", "Pacton");
	IupSetAttribute (dlg, "SIZE", "800x150");
	IupSetAttribute (dlg, "MENU", "mymenu");

	//Show gui:
	IupShowXY (dlg, IUP_CENTER, IUP_CENTER);
	IupMainLoop ();
	IupClose ();

	return EXIT_SUCCESS;
}
