/**********************************************
 * publish webcam frames to eCAL
 * payload: jpg compressed
 * tested on Ubuntu 20.04, 22.04
 * v1.1
 * 10-24-2026 - dramatic speedup using cv::CAP_V4L2 + MJPG encoding
 * 5-27-2025
*/

#include "opencv2/opencv.hpp"

using namespace cv;

// <------- eCAL ---------->
#define eCAL_ON
#ifdef eCAL_ON
// eCAL
#include <ecal/ecal.h>
#include <ecal/msg/protobuf/publisher.h>
// proto i/o
#include "webcam.pb.h"

#include <vector> // For std::vector

eCAL::protobuf::CPublisher<pb::webcam::webcam_raw> publisher_wcamr;
int frameid;

void init_eCAL(int argc, char** argv) {
  // Initialize eCAL
  eCAL::Initialize(argc, argv, "Webcam->eCAL");
  // create publisher
  // put all jpg into the raw payload, jpg contains all info as w, h, channels etc.
  publisher_wcamr = eCAL::protobuf::CPublisher<pb::webcam::webcam_raw>("webcam_jpg");
  printf("Ecal publisher created\n");
  // set eCAL state to healthy (--> eCAL Monitor)
  eCAL::Process::SetState(proc_sev_healthy, proc_sev_level1, "Webcam eCAL pub-node initialized");
}

void exit_eCAL() {
  eCAL::Finalize(); // finalize eCAL API
}

void convertToJpeg(const cv::Mat& frame, std::vector<uchar>& jpegBuffer) {
    // Encode the image to JPEG format
    std::vector<int> compressionParams = {cv::IMWRITE_JPEG_QUALITY, 90}; // Set JPEG quality to 90
    cv::imencode(".jpg", frame, jpegBuffer, compressionParams);
}

void publish_camjpg(const cv::Mat &frame) {
    printf("frame %d --> rows=%d, cols=%d, chan's=%d\n",frameid, frame.rows, frame.cols, frame.channels());
    pb::webcam::webcam_raw wcamj;

    // Convert frame to JPEG
    std::vector<uchar> jpegBuffer;
    convertToJpeg(frame, jpegBuffer);
    int size = jpegBuffer.size();
 
    wcamj.set_payload(reinterpret_cast<char*>(jpegBuffer.data()), jpegBuffer.size());
    wcamj.set_frame_id(++frameid);
    wcamj.set_size(size); // w x h
    wcamj.set_timestamp(eCAL::Time::GetMicroSeconds());
    publisher_wcamr.Send(wcamj);
};

void publish_camraw(const cv::Mat &frame) {
    printf("frame %d --> rows=%d, cols=%d, chan's=%d\n",frameid, frame.rows, frame.cols, frame.channels());
    int size = frame.rows*frame.cols;
    pb::webcam::webcam_raw wcamr;
    wcamr.set_payload(reinterpret_cast<char*>(frame.data), size * frame.elemSize());
    wcamr.set_frame_id(++frameid);
    wcamr.set_size(size); // w x h
    publisher_wcamr.Send(wcamr);
};

#endif // eCAL_ON
// <------- eCAL ---------->


/* ------------------------------------------------
   How To Run:
 1) capture + publisher: build
 2) visualizer (incl. subscriber): build/show_jpg
 
   How to get available resolutions for USB camera:
   > sudo apt install v4l-utils
   > v4l2-ctl -d /dev/video0 --list formats-ext
   ------------------------------------------------ */
int main(int argc, char** argv)
{
    printf("cam_eCAL <#device> <width> <height>\n");
    int device = 0;
    int w = 0;
    int h = 0;
    int fps = 10;
    if (argc > 1) {
        device = atoi(argv[1]);
        printf("#device: %d\n", device);
    } else {
        printf("no #device specified, using default port 0\n");
    }
    if (argc > 2) {
        w = atoi(argv[2]);
    }
    if (argc > 3) {
        h = atoi(argv[3]);
    }
    if (argc > 4) {
        fps = atoi(argv[4]);
    }

    printf("#width: %d\n", w);
    printf("#height: %d\n", h);
    printf("fps: %d\n", fps);

    init_eCAL(argc, argv);

//    VideoCapture cap(device); // open the camera <device>
    VideoCapture cap(device, cv::CAP_V4L2); // (5x) faster access + readout
    if(!cap.isOpened())  // check if we succeeded
        return -1;
    cap.set(cv::CAP_PROP_FRAME_WIDTH,  w);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, h);
    // YUYV readout is too slow
    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M','J','P','G')); // fast readout
//    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M','P','G','1')); // fast readout
    cap.set(cv::CAP_PROP_FPS, fps);
//    cap.set(cv::CAP_PROP_FPS, 15); // 3840 x 1080 gets to ~12 fps
//    cap.set(cv::CAP_PROP_FPS, 10); // 1600 x 600, ne das geht auch mit 3840 x 1080
//    cap.set(cv::CAP_PROP_FPS, 5); //  2560 x 720
//                                      3840 x 1080, 5 FPS with CAP_V4L2 + MJPG
//                                                   1 FPS with CAP_V4L2 + MPG1
//                                                   1 FPS with CAP_V4L2 + YUYV
//                                                   1 FPS no CAP_V4L2
// ------------------------------
// benchmark for china stereocam:
// 3840   12 fps
// 2560   18 fps
// 1600   23 fps
// ------------------------------

    Mat edges;
    for(;;)
    {
        Mat frame;
        cap >> frame; // get a new frame from camera

        publish_camjpg(frame);

        if(waitKey(30) >= 0) break;
    }
    // the camera will be deinitialized automatically in VideoCapture destructor

    exit_eCAL(); // finalize eCAL API

    return 0;
}
