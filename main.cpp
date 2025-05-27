#include "opencv2/opencv.hpp"

using namespace cv;

// <------- eCAL ---------->
#define eCAL_ON
#ifdef eCAL_ON
// eCAL
#include <ecal/ecal.h>
#include <ecal/msg/protobuf/publisher.h>
#include <ecal/msg/protobuf/subscriber.h>
// proto i/o
#include "webcam.pb.h"

eCAL::protobuf::CPublisher<pb::webcam::webcam_raw> publisher_wcamr;
int frameid;

void init_eCAL(int argc, char** argv) {
  // Initialize eCAL
  eCAL::Initialize(argc, argv, "Webcam->eCAL");
  // create publisher
  publisher_wcamr = eCAL::protobuf::CPublisher<pb::webcam::webcam_raw>("WEBCAM_RAW"); // mono or stereo-left
  printf("Ecal publisher created\n");
  // set eCAL state to healthy (--> eCAL Monitor)
  eCAL::Process::SetState(proc_sev_healthy, proc_sev_level1, "Webcam eCAL publisher initialized");
}

void exit_eCAL() {
  eCAL::Finalize(); // finalize eCAL API
}

void publish_cam(const cv::Mat &frame) {
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

    Mat edges;
    namedWindow("webcam->eCAL",1);
    for(;;)
    {
        Mat frame;
        cap >> frame; // get a new frame from camera
//        cvtColor(frame, edges, COLOR_BGR2GRAY);
//        GaussianBlur(edges, edges, Size(7,7), 1.5, 1.5);
//        Canny(edges, edges, 0, 30, 3);
//        imshow("edges", edges);
        imshow("webcam->eCAL", frame);

        publish_cam(frame);

        if(waitKey(30) >= 0) break;
    }
    // the camera will be deinitialized automatically in VideoCapture destructor

    exit_eCAL(); // finalize eCAL API

    return 0;
}
