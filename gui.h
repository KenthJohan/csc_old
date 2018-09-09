#pragma once










struct Rectangle_SWNE
{
	struct v2f32_xy sw;
	struct v2f32_xy ne;
};

void Rectangle_SWNE_SE (struct v2f32_xy * se, struct Rectangle_SWNE * r)
{
	se->x = r->ne.x;
	se->y = r->sw.y;
}

void Rectangle_SWNE_NW (struct v2f32_xy * nw, struct Rectangle_SWNE * r)
{
	nw->x = r->sw.x;
	nw->y = r->ne.y;
}
