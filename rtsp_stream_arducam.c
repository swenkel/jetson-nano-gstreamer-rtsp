/* GStreamer
 * Copyright (C) 2023 Simon Wenkel
 * Copyright (C) 2008 Wim Taymans <wim.taymans at gmail.com>
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

#include <stdio.h>

#define DEFAULT_RTSP_PORT "8554"
#define DEFAULT_FLIP "0"
#define DEFAULT_SENSOR_ID "0"
#define DEFAULT_SENSOR_RES "0"
#define DEFAULT_OUT_FPS "30"
#define DEFAULT_BIT_RATE "10000000"
#define DEFAULT_ENCODER "0"

static char *port = (char *) DEFAULT_RTSP_PORT;
static char *flip = (char *) DEFAULT_FLIP;
static char *sensor_id = (char *) DEFAULT_SENSOR_ID;
static char *sres = (char *) DEFAULT_SENSOR_RES;
static char *ofps = (char *) DEFAULT_OUT_FPS;
static char *enc_bitrate = (char *) DEFAULT_BIT_RATE;
static char *enc = (char *) DEFAULT_ENCODER;


static GOptionEntry entries[] = {
    {"port", 'p', 0, G_OPTION_ARG_STRING, &port,
        "Port to listen on (default: " DEFAULT_RTSP_PORT ")", "PORT"},
    {"flip",'f', 0, G_OPTION_ARG_STRING, &flip,
    "Flip input image (default: 0 (none)); 0:none,1:90ccw,2:180,3:90cw,4:horizontal flip,6: vertial flip", "FLIP"},
    {"sensor_id", 's', 0, G_OPTION_ARG_STRING,&sensor_id,
    "Camera sensor ID (default: 0)", "SID"},
    {"sensor_res", 'r', 0, G_OPTION_ARG_STRING, &sres,
    "Sensor res: 0: 4032x3040@30, 1:3840x2160@30, 2:1920x1080@60", "SRES"},
    {"ouput_fps", 'o', 0, G_OPTION_ARG_STRING, &ofps,
    "Desired output FPS (default: 30)", "OFPS"},
    {"enc_bitrate", 'b', 0, G_OPTION_ARG_STRING, &enc_bitrate,
    "Desired encoding bitrate (default: 10000000)","ENCBIT"},
    {"encover", 'e', 0, G_OPTION_ARG_STRING, &enc,
  	"encoder {0: h264,1: h265} (default: 0)","ENC"},
    {NULL}
};

int main (int argc, char *argv[])
{
    GMainLoop *loop;
    GstRTSPServer *server;
    GstRTSPMountPoints *mounts;
    GstRTSPMediaFactory *factory;
    GOptionContext *optctx;
    GError *error = NULL;

    optctx = g_option_context_new ("");
    g_option_context_add_main_entries (optctx, entries, NULL);
    g_option_context_add_group (optctx, gst_init_get_option_group ());
    if (!g_option_context_parse (optctx, &argc, &argv, &error)) {
    g_printerr ("Error parsing options: %s\n", error->message);
    g_option_context_free (optctx);
    g_clear_error (&error);
    return -1;
    }
    g_option_context_free (optctx);

    loop = g_main_loop_new (NULL, FALSE);

    /* create a server instance */
    server = gst_rtsp_server_new ();
    g_object_set (server, "service", port, NULL);

    /* get the mount points for this server, every server has a default object
    * that be used to map uri mount points to media factories */
    mounts = gst_rtsp_server_get_mount_points (server);

    /* make a media factory for a test stream. The default media factory can use
    * gst-launch syntax to create pipelines.
    * any launch line works as long as it contains elements named pay%d. Each
    * element with pay%d names will be a stream */

    // set the options
    static char * video_str;
    // three resolutions that seem to be stable
    if (*sres == '0') {
        video_str = "video/x-raw(memory:NVMM),"
            "format=NV12,width=4032,height=3040,framerate=";
    } else if (*sres == '1') {
        video_str = "video/x-raw(memory:NVMM),"
            "format=NV12,width=3840,height=2160,framerate=";
    } else if (*sres == '2') {
        video_str = "video/x-raw(memory:NVMM),"
            "format=NV12,width=1920,height=1080,framerate=";
    } else {
        video_str =  "video/x-raw(memory:NVMM),"
            "format=NV12,width=4032,height=3040,framerate=";
    }

    static char *gst_cap = "nvarguscamerasrc sensor-id=";
    static char *gst_sep = " ! ";
    static char *gst_frc = "/1";
    static char *gst_flp = "nvvidconv top=0 bottom=2160 left=0 right=3840 !"
                          " nvvidconv flip-method=";
    static char *gst_enc;
    static char *gst_out_enc;
    if (*enc == '0') {
        gst_enc = " ! nvv4l2h264enc preset-level=2 bitrate=";
        gst_out_enc = " h264parse ! rtph264pay";
    } else if (*enc == '1') {
        gst_enc = " ! nvv4l2h265enc preset-level=2 bitrate=";
        gst_out_enc = " h265parse ! rtph265pay";
    } else {
        gst_enc = " ! nvv4l2h264enc preset-level=2 bitrate=";
        gst_out_enc = " h264parse ! rtph264pay";
    }
    static char *gst_out = " profile=0 iframeinterval=10 control-rate=2 "
        "low-latency=1 max-perf-enabled=1 vbv-size=1100000 ! ";
    

     
    static char *gst_out_end = " name=pay0 pt=96";
    char *gst_str = (char *) malloc (strlen(gst_cap) + 
        strlen(sensor_id) + strlen(video_str) + strlen(ofps) + 
        strlen(gst_frc) + strlen(gst_sep) + strlen(gst_flp) + 
        strlen(flip) + strlen(gst_enc) + strlen(enc_bitrate) +
        strlen(gst_out) + strlen(gst_out_enc) + 
        strlen(gst_out_end) + 1);
    
    // create the gstreamer pipeline string
    strcpy(gst_str, gst_cap);
    strcat(gst_str, sensor_id);
    strcat(gst_str, video_str);
    strcat(gst_str, ofps);
    strcat(gst_str, gst_frc);
    strcat(gst_str, gst_sep);
    strcat(gst_str, gst_flp);
    strcat(gst_str, flip);
    strcat(gst_str, gst_enc);
    strcat(gst_str, enc_bitrate);
    strcat(gst_str, gst_out);
    strcat(gst_str, gst_out_enc);
    strcat(gst_str, gst_out_end);

    factory = gst_rtsp_media_factory_new ();
    gst_rtsp_media_factory_set_launch (factory, gst_str);
    gst_rtsp_media_factory_set_shared (factory, TRUE);

    /* attach the test factory to the /test url */
    gst_rtsp_mount_points_add_factory (mounts, "/stream", factory);

    /* don't need the ref to the mapper anymore */
    g_object_unref (mounts);

    /* attach the server to the default maincontext */
    gst_rtsp_server_attach (server, NULL);

    /* start serving */
    g_print ("stream ready at rtsp://127.0.0.1:%s/stream\n", port);
    g_main_loop_run (loop);

    return 0;
}
