/**********************************************
 * publish webcam frames to eCAL
 * payload: jpg compressed
 * tested on Ubuntu 20.04
 * v1.0
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


int main(int argc, char** argv)
{
    printf("cam_eCAL <#device>\n");
    int device = 0;
    if (argc > 1) {
        device = atoi(argv[1]);
        printf("#device: %d\n", device);
    } else {
        printf("no #device specified, using default port 0\n");
    }

    init_eCAL(argc, argv);

    VideoCapture cap(device); // open the camera <device>
    if(!cap.isOpened())  // check if we succeeded
        return -1;
//    cap.set(cv::CAP_PROP_FRAME_WIDTH, 1024);
//    cap.set(cv::CAP_PROP_FRAME_HEIGHT,768);

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
