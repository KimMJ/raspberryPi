#include    <iostream>
#include    <string>
#include    <opencv/cv.hpp>
 
using namespace std;
using namespace cv;
 
#define INPUT "/Users/kimmj/Dropbox/cctv.avi"
#define OUTPUT_PREFIX "img/output_"
#define OUTPUT_POSTFIX ".jpg"
 
int main()
{
 
    int iCurrentFrame = 0;
 
    VideoCapture vc = VideoCapture(INPUT);
 
    if (!vc.isOpened())
    {
        cerr << "fail to open the video" << endl;
        return EXIT_FAILURE;
    }
 
    double fps = vc.get(CV_CAP_PROP_FPS);
    cout << vc.get(CV_CAP_PROP_FRAME_COUNT) << endl;

    while (true)
    {
        //vc.set(CV_CAP_PROP_POS_FRAMES, iCurrentFrame);
        int posFrame = vc.get(CV_CAP_PROP_POS_FRAMES);

        //cout << posFrame << endl;
 
        Mat frame;
        vc >> frame;
        if (frame.empty())
            break;
 
        if (posFrame % 100 == 0) {
          imshow("image", frame);
          stringstream ss;
          ss << posFrame;
          string filename = OUTPUT_PREFIX + ss.str() + OUTPUT_POSTFIX;
          imwrite(filename.c_str(), frame);
        }
        waitKey(1);
    }
 
 
    return EXIT_SUCCESS;
}

