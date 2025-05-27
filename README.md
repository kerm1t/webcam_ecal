# Webcam eCAL
This repo holds following apps:
- publisher (command line tool)
- viewer

Dependencies: opencv
```
sudo apt-get install libopencv-dev
```

## Clone, Build and Start
```
git clone git@github.com:kerm1t/webcam_ecal.git

mkdir build
cd build
cmake ..
make

./cam_eCAL [<device> <widht> <height>] // start the publisher
                                       // optional: device = 0, ... , 3
                                       //           img width e.g. 1280
                                       //           img height e.g. 720
```
Open a new Terminal
```
./show_jpg                             // start the viewer
```
## Output
![Screenshot from 2025-05-27 18-06-05](https://github.com/user-attachments/assets/285e8346-18b2-4a5d-bf9f-28fe8bc7d365)

