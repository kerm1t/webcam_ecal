#include "opencv2/opencv.hpp"

using namespace cv;

// <------- eCAL ---------->
#define eCAL_ON
#ifdef eCAL_ON
// eCAL
#include <ecal/ecal.h>
#include <ecal/msg/protobuf/subscriber.h>
// proto i/o
#include "webcam.pb.h"

#include <vector>
#include <mutex>

eCAL::protobuf::CSubscriber<pb::webcam::webcam_raw> subscriber_wcamr;

int frameid;

std::mutex callback_mutex; // Declare a global mutex

void webcam_jpg_callback(const pb::webcam::webcam_raw& wcamj)
{
   // Lock the mutex to ensure thread safety
   std::lock_guard<std::mutex> lock(callback_mutex);

   printf("webcam_jpg_callback: frame %d --> size=%d\n", wcamj.frame_id(), wcamj.size());
   // Convert the JPEG payload back to a cv::Mat
   std::vector<uchar> jpegBuffer(wcamj.payload().begin(), wcamj.payload().end());
   printf("1\n");
   cv::Mat frame = cv::imdecode(jpegBuffer, cv::IMREAD_COLOR);
   printf("2\n");
   if (frame.empty()) {
      printf("Failed to decode JPEG image\n");
      return;
   } else {
      printf("frame %d --> rows=%d, cols=%d, chan's=%d\n",frameid, frame.rows, frame.cols, frame.channels());
   }
   // Display the frame
   imshow("webcam->eCAL", frame);
   printf("3\n");
   waitKey(1); // Allow OpenCV to process the window events
}

void init_eCAL(int argc, char** argv) {
  // Initialize eCAL
  eCAL::Initialize(argc, argv, "eCAL jpg-viewer");
  // create subscriber
  subscriber_wcamr = eCAL::protobuf::CSubscriber<pb::webcam::webcam_raw>("webcam_jpg");
  printf("Ecal subscriber created\n");
  // Set the Callback
  subscriber_wcamr.AddReceiveCallback(std::bind(&webcam_jpg_callback, std::placeholders::_2));

  // set eCAL state to healthy (--> eCAL Monitor)
  eCAL::Process::SetState(proc_sev_healthy, proc_sev_level1, "Webcam eCAL sub-node initialized");
}

void exit_eCAL() {
  eCAL::Finalize(); // finalize eCAL API
}

#endif // eCAL_ON
// <------- eCAL ---------->


int main(int argc, char** argv)
{
    printf("show webcam\n");

    init_eCAL(argc, argv);

    namedWindow("eCAL jpg-viewer",1);

    // Just don't exit
    while (eCAL::Ok()) {
        if(waitKey(30) >= 0) break;
    }

    exit_eCAL(); // finalize eCAL API

    return 0;
}
