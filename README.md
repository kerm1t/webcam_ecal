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

./cam_eCAL <device> // start the publisher, device = 0,...,3
./show_jpg          // start the viewer
```
## Output
![Screenshot from 2025-05-27 17-07-44](https://github.com/user-attachments/assets/79ef4032-7974-47e9-953a-7ebcf6aed8ee)
