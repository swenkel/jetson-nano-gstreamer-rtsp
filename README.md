## GStreamer RTSP Server Script for NVIDIA JETSON NANO


This is a modified script of `test-launch.c` from [`gst-rtsp-server`](https://github.com/GStreamer/gst-rtsp-server) made for easy usage with MIPI connected cameras such as "arducam" and others. Launching more complicated GStreamer pipelines on a Jetson can be quite tricky. Using a proper binary takes care of this.


It can be compiled using gcc:
```
gcc -o stream rtsp_stream_arducam.c `pkg-config --cflags --libs gstreamer-rtsp-server-1.0`
```


The argument should be self-explanatory

```sh
./stream -h
Usage:
  stream [OPTION?]

Help Options:
  -h, --help                        Show help options
  --help-all                        Show all help options
  --help-gst                        Show GStreamer Options

Application Options:
  -p, --port=PORT                   Port to listen on (default: 8554)
  -f, --flip=FLIP                   Flip input image (default: 0 (none)); 0:none,1:90ccw,2:180,3:90cw,4:horizontal flip,6: vertial flip
  -s, --sensor_id=SID               Camera sensor ID (default: 0)
  -r, --sensor_res=SRES             Sensor res: 0: 4032x3040@30, 1:3840x2160@30, 2:1920x1080@60
  -o, --ouput_fps=OFPS              Desired output FPS (default: 30)
  -b, --enc_bitrate=ENCBIT          Desired encoding bitrate (default: 10000000)
  -e, --encover=ENC                 encoder {0: h264,1: h265} (default: 0)
```


Using `./stream -e 0` will use the h264 encoder which is the default option wheras using `./stream -e 1` will use the h265 encoder.




