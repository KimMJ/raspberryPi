#include <highgui.h>
#include <cv.h>

int g_slider_position = 0;
CvCapture* g_capture = NULL;

void onTrackbarSlide(int pos)
{
	cvSetCaptureProperty(
		g_capture,
		CV_CAP_PROP_POS_FRAMES,
		pos
	);
}

int main(int argc, char** argv)
{
	cvNamedWindow("Example", CV_WINDOW_AUTOSIZE);	
	g_capture = cvCreateFileCapture(argv[1]);
	int frameCount = (int) cvGetCaptureProperty(
		g_capture,
		CV_CAP_PROP_FRAME_COUNT
	);

  /*
	if(frameCount != 0)
	{
		cvCreateTrackbar(
			"Position",
			"Example",
			&g_slider_position,
			frameCount,
			onTrackbarSlide
		);
	}
  */
	IplImage* frame;	
	char c;	
	int pos;
	
	//동영상 진행 간 연속 출력
	while(1){
    cvSetCaptureProperty(g_capture, CV_CAP_PROP_POS_FRAMES, pos);
    cvGrabFrame(g_capture);
    printf("%d\n", pos);
    pos += 25;
		frame = cvRetrieveFrame(g_capture);
		//cvSetTrackbarPos("Position", "Example", pos);
		if(!frame) {
			break;
		}
		cvShowImage("Example", frame);	
		//pos = (int)cvGetCaptureProperty(g_capture, CV_CAP_PROP_POS_FRAMES);

		c = cvWaitKey(1);
		if(c == 27){
			break;
		}
	}
	
	cvReleaseCapture(&g_capture);
	cvDestroyWindow("Example");
	return 0;
}

