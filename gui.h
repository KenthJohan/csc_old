#pragma once


struct Rectangle_SWNE
{
	struct v2f32_xy sw;
	struct v2f32_xy ne;
};

/*
struct Rectangle_Quad
{
	struct v2f32_xy sw;
	struct v2f32_xy ne;
	struct v2f32_xy se;
	struct v2f32_xy nw;
};

void rect_convert 
(struct Rectangle_Quad * dst, struct Rectangle_SWNE * src)
{
	memcpy (dst, src, sizeof (struct Rectangle_SWNE));
	dst->se.x = src->ne.x;
	dst->se.y = src->sw.y;
	dst->nw.x = src->sw.x;
	dst->nw.y = src->ne.y;
}
*/



void Rectangle_SWNE_SE 
(struct v2f32_xy * se, struct Rectangle_SWNE const * r)
{
	se->x = r->ne.x;
	se->y = r->sw.y;
}

void Rectangle_SWNE_NW 
(struct v2f32_xy * nw, struct Rectangle_SWNE const * r)
{
	nw->x = r->sw.x;
	nw->y = r->ne.y;
}
