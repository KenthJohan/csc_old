#pragma once
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>




void xxav_open 
(
	uint32_t n,
	AVFormatContext * fctx [],
	AVCodecContext  * cctx [],
	struct SwsContext * wctx [],
	AVFrame * frame0 [],
	AVFrame * frame1 [],
	int ivid [],
	char const * url []
)
{
	for (uint32_t i = 0; i < n; ++ i)
	{
		fctx [i] = avformat_alloc_context ();
		ASSERT_F (fctx [i] != NULL, "Could not allocate memory for Format Context %p", fctx [i]);
	}
	for (uint32_t i = 0; i < n; ++ i)
	{
		//Open an input stream and read the header.
		//The codecs are not opened.
		//The stream must be closed with avformat_close_input().
		int r = avformat_open_input (fctx + i, url [i], NULL, NULL);
		ASSERT_F (r == 0, "ERROR could not open the file %i", r);
	}
	for (uint32_t i = 0; i < n; ++ i)
	{
		//Read packets of a media file to get stream information. 
		//This is useful for file formats with no headers such as MPEG.
		//This function also computes the real framerate in case of MPEG-2 repeat frame mode.
		//The logical file position is not changed by this function; 
		//examined packets may be buffered for later processing.
		int r = avformat_find_stream_info (fctx [i],  NULL);
		ASSERT_F (r >= 0, "ERROR could not get the stream info %i", r);
		
		TRACE_F ("duration            %lli", (long long int)fctx [i]->duration);
		TRACE_F ("bit_rate            %lli", (long long int)fctx [i]->bit_rate);
		//TRACE_F ("file_size %lli", (long long int)fctx [i]->file_size);
		TRACE_F ("max_index_size      %lli", (long long int)fctx [i]->max_index_size);
		TRACE_F ("packet_size         %lli", (long long int)fctx [i]->packet_size);
		TRACE_F ("start_time_realtime %lli", (long long int)fctx [i]->start_time_realtime);
		TRACE_F ("nb_streams          %lli", (long long int)fctx [i]->nb_streams);
	}
	for (uint32_t i = 0; i < n; ++ i)
	{
		//Print detailed information about the input or output format, 
		//such as duration, bitrate, streams, container, 
		//programs, metadata, side data, codec and time base. 
		av_dump_format (fctx [i], 0, url [i], 0);
	}
	for (uint32_t i = 0; i < n; ++ i)
	{
		//The best stream is determined according to various heuristics 
		//as the most likely to be what the user expects. 
		//If the decoder parameter is non-NULL, 
		//av_find_best_stream will find the default decoder for the stream's codec; 
		//streams for which no decoder can be found are ignored.
		AVCodec * codec = NULL;
		ivid [i] = av_find_best_stream (fctx [i], AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);
		ASSERT (ivid [i] >= 0);
		ASSERT_F (codec != NULL, "ERROR unsupported codec! %s", "");
		AVStream * stream = fctx [i]->streams [ivid [i]];
		cctx [i] = stream->codec;
		//ASSERT (fctx [i]->streams [ivid [i]]->codecpar);
		//AVCodec * codec = avcodec_find_decoder (cctx->codec_id);
		//ASSERT_F (cctx [i] != NULL, "ERROR unsupported codec! %s", "");
		int r = avcodec_open2 (cctx [i], codec, NULL);
		ASSERT_F (r >= 0, "Failed to open codec for stream %i", ivid [i]);
	}
	for (uint32_t i = 0; i < n; ++ i)
	{
		AVPacket packet;
		int eof = av_read_frame (fctx [i], &packet);
		ASSERT_F (eof == 0, "%s", "");
		frame0 [i] = av_frame_alloc ();
		ASSERT (frame0 [i]);
		frame1 [i] = av_frame_alloc ();
		ASSERT (frame0 [i]);
		int finnish;
		avcodec_decode_video2 (cctx [i], frame0 [i], &finnish, &packet);
		ASSERT (finnish != 0);
		TRACE_F ("avcodec_decode_video2 (%i %i)", frame0 [i]->width, frame0 [i]->height);
		av_free_packet (&packet);
	}
	for (uint32_t i = 0; i < n; ++ i)
	{
		frame1 [i]->width = frame0 [i]->width;
		frame1 [i]->height = frame0 [i]->height;
		frame1 [i]->format = AV_PIX_FMT_RGB24;
		int size = avpicture_get_size (frame1 [i]->format, frame1 [i]->width, frame1 [i]->height);
		uint8_t * buf = (uint8_t *) av_malloc (size);
		ASSERT (buf != NULL);
		avpicture_fill ((AVPicture *)frame1 [i], buf, frame1 [i]->format, frame1 [i]->width, frame1 [i]->height);
	}
	for (uint32_t i = 0; i < n; ++ i)
	{
		TRACE_F ("AVPixelFormat: %s", av_get_pix_fmt_name (frame0 [i]->format));
		//Create a anyformat to RGB24 converter.
		//Encoded videos usually uses YUV420p pixelformat.
		//OpenGL does not support YUV420p but it can use RGB24.
		wctx [i] = sws_getContext
		(
			frame0 [i]->width, 
			frame0 [i]->height, 
			frame0 [i]->format, 
			frame1 [i]->width,
			frame1 [i]->height, 
			frame1 [i]->format,
			SWS_FAST_BILINEAR, 
			NULL, 
			NULL, 
			NULL
		);
		ASSERT (wctx [i]);
	}
	for (uint32_t i = 0; i < n; ++ i)
	{
		sws_scale
		(
			wctx [i], 
			(const unsigned char * const*)frame0 [i]->data, 
			frame0 [i]->linesize, 
			0, 
			frame0 [i]->height, 
			frame1 [i]->data, 
			frame1 [i]->linesize
		);
	}
}


void xxav_dump_frame (FILE * f, AVFrame const * frame)
{
	fprintf (f, "pts:%08ld ", frame->pts);
	fprintf (f, "pkt_pts:%08ld ", frame->pkt_pts);
	fprintf (f, "pkt_dts:%08ld ", frame->pkt_dts);
	fprintf (f, "width:%04d ", frame->width);
	fprintf (f, "height:%04d ", frame->height);
	fprintf (f, "pix_fmt:%04d ", frame->format);
	fprintf (f, "pix_fmt:%s ", av_get_pix_fmt_name (frame->format));
	fprintf (f, "\n");
}


double xxav_dts2sec (AVStream * stream, int64_t dts)
{
	double time_base = av_q2d (stream->time_base);
	TRACE_F ("tb %f", time_base);
	double sec = (dts - stream->start_time) * time_base;
    return sec;
}


int64_t xxav_dts2timebase (AVStream * stream, int64_t dts)
{
	double x = xxav_dts2sec (stream, dts) * (double)AV_TIME_BASE;
	return (int64_t)x;
}


int64_t xxav_dts2fnum (AVStream * stream, int64_t dts)
{
	double fps = av_q2d (stream->avg_frame_rate);
	double sec = xxav_dts2sec (stream, dts);
    return (int64_t) floor (sec * fps + 0.5);
}


double xxav_dts2norm (AVStream * stream, int64_t dts)
{
	return stream->duration / xxav_dts2sec (stream, dts);
}


int xxav_next1
(
	AVFormatContext * fctx, 
	AVCodecContext  * cctx,
	int istream,
	struct SwsContext * wctx,
	AVFrame * frame0,
	AVFrame * frame1,
	int64_t * pts
)
{
	while (1)
	{
		AVPacket packet;
		//Return the next frame of a stream
		int eof = av_read_frame (fctx, &packet);
		//Check if error or end of file.
		if (eof != 0) {break;}
		
		(*pts) = packet.pts;
		
		AVStream * stream = fctx->streams [istream];
		/*
		if (tsec) {*tsec = xxav_dts2sec (stream, packet.pts);}
		if (fnum) {*fnum = xxav_dts2fnum (stream, packet.pts);}
		*/
		///*
		//TRACE_F ("sec  %f of %f", (double)xxav_dts2sec (stream, packet.pts), (double)fctx->duration / (double)AV_TIME_BASE);
		//TRACE_F ("fnum %f", (double)xxav_dts2fnum (stream, packet.pts));
		//TRACE_F ("%lli ", (long long int)stream->duration);
		//TRACE_F ("%lli ", (long long int)packet.pts);
		//TRACE_F ("%f ",  (double)packet.pts / (double)fctx->duration);
		//*/
		//Make sure the packet comes from the correct stream.
		if (packet.stream_index != istream) {continue;}
		int finnish;
		avcodec_decode_video2 (cctx, frame0, &finnish, &packet);
		//Check if no frame could be decompressed
		if (finnish == 0) {continue;}
		//xxav_dump_frame (stderr, frame0);
		//Use the pixelformat converter (wctx) to convert the decoded frames to our choosing.
		sws_scale
		(
			wctx, 
			(const unsigned char * const*)frame0->data, 
			frame0->linesize, 
			0, 
			frame0->height, 
			frame1->data, 
			frame1->linesize
		);
		av_packet_unref (&packet);
		break;
	}
	return 0;
}



int xxav_next
(
	uint32_t n,
	AVFormatContext * fctx [], 
	AVCodecContext  * cctx [],
	int istream [],
	struct SwsContext * wctx [],
	AVFrame * frame0 [],
	AVFrame * frame1 [],
	int64_t pts []
)
{
	for (uint32_t i = 0; i < n; ++ i)
	{
		xxav_next1 
		(
			fctx [i], 
			cctx [i], 
			istream [i], 
			wctx [i], 
			frame0 [i], 
			frame1 [i],
			pts + i
		);	
	}
	return 0;
}




float xxav_normtime 
(
	AVFormatContext * fctx,
	uint64_t index,
	uint64_t pts
)
{
	AVStream * stream = fctx->streams [index];
	float t = xxav_dts2sec (stream, pts);
	float d = (float)fctx->duration / (float)AV_TIME_BASE;
	//TRACE_F ("%f %f", (double)t, (double)d);
	return t / d;
}


/*
uint64_t xxav_normtime0 
(
	AVFormatContext * fctx,
	uint64_t index,
	float x1
)
{
	AVStream * stream = fctx->streams [index];
	float t = xxav_dts2sec (stream, pts);
	float d = (float)fctx->duration / (float)AV_TIME_BASE;
	//TRACE_F ("%f %f", (double)t, (double)d);
	return t / d;
}
*/




void xxav_seek 
(
	AVFormatContext * fctx, 
	AVCodecContext  * cctx,
	int istream,
	struct SwsContext * wctx,
	AVFrame * frame0,
	AVFrame * frame1,
	int64_t * pts,
	int64_t ts
)
{
	while (1)
	{
		AVPacket packet;
		//Return the next frame of a stream
		int eof = av_read_frame (fctx, &packet);
		//Check if error or end of file.
		if (eof != 0) {break;}
		(*pts) = packet.pts;
		AVStream * stream = fctx->streams [istream];
		if (packet.stream_index != istream) {continue;}
		int finnish;
		avcodec_decode_video2 (cctx, frame0, &finnish, &packet);
		if (finnish == 0) {continue;}
		sws_scale
		(
			wctx, 
			(const unsigned char * const*)frame0->data, 
			frame0->linesize, 
			0, 
			frame0->height, 
			frame1->data, 
			frame1->linesize
		);
		av_packet_unref (&packet);
		TRACE_F ("%10lli %10lli %10lli", ts, *pts, xxav_dts2timebase (stream, *pts));
		//TRACE_F ("%lli %lli %lli", ts, xxav_dts2sec (stream, packet.pts), AV_TIME_BASE);
		if (ts <= xxav_dts2timebase (stream, *pts))
		{
			TRACE ("BREAK");
			break;
		}
	}
}



