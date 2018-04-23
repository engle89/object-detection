#include <iostream>
#include <cuda_runtime.h>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

int main()
{
	cv::VideoCapture camera(0);
	if (!camera.isOpened())
		return -1;

	cv::namedWindow("Source");

	Mat image = imread("D:/engle/image processing/object detection/object detection/small.PNG",-1);
	Mat imageRGBA;
	cvtColor(image, imageRGBA, CV_BGR2RGBA);

	while (1)
	{
		//part one
		//red mask that filters out other colors

		cv::Mat frame;
		bool bSuccess = camera.read(frame);
		if (bSuccess == false)
		{
			cout << "Video camera is disconnected" << endl;
			cin.get(); //Wait for any key press
			break;
		}
		
		cv::Mat origin = frame.clone();
		cv::medianBlur(frame, frame, 3);
		cv::Mat hsv;
		cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);

		//threshold the hsv image, keep only red pixels
		cv::Mat lower_red_hue_range;
		cv::Mat upper_red_hue_range;
		cv::inRange(hsv, cv::Scalar(0, 100, 100), cv::Scalar(10, 255, 255), lower_red_hue_range);
		cv::inRange(hsv, cv::Scalar(160, 100, 100), cv::Scalar(179, 255, 255), upper_red_hue_range);

		cv::Mat red_hue_image;
		cv::addWeighted(lower_red_hue_range, 1.0, upper_red_hue_range, 1.0, 0.0, red_hue_image);

		cv::GaussianBlur(red_hue_image, red_hue_image, cv::Size(9, 9), 2, 2);

		//part two
		//find centroid
		Mat temp;
		compare(red_hue_image, Scalar(50), temp, CMP_GE);
		Moments m = moments(temp, false);
		Point p(m.m10 / m.m00, m.m01 / m.m00);
		int distance = 0;
		bool a = true;
		if (m.m00 == 0)
		{
			a = false;
			distance = 0;
		}
		else
		{
			a = true;
			circle(red_hue_image, p, 5, Scalar(128, 0, 0), -1);

			//distance = sqrt(m.m00);
			vector<vector<Point> > contours;
			vector<Vec4i> hierarchy;
			findContours(red_hue_image, contours, hierarchy,CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
			int area = contourArea(contours[0]);
			distance = sqrt(area);
			cv::resize(imageRGBA, imageRGBA, cv::Size(distance, distance));
		}

		//image overlay
		for (int y = std::max(p.y - distance / 2, 0); y < min(p.y + distance / 2, frame.rows); ++y)
		{
			if (a == false)
				break;
			int fY = y - (p.y - distance / 2);
			for (int x = std::max(p.x - distance / 2, 0); x < min(p.x + distance / 2, frame.rows); ++x)
			{
				int fX = x - (p.x - distance / 2);
				for (int c = 0; c < frame.channels(); ++c)
				{
					frame.data[y*frame.step + x * frame.channels() + c] = imageRGBA.data[fY*imageRGBA.step + fX * imageRGBA.channels() + c];
				}
			}
		}

		cv::imshow("Source", origin);
		cv::imshow("Output", red_hue_image);

		if (waitKey(10) == 27)
		{
			cout << "Esc key is pressed by user. Stoppig the video" << endl;
			break;
		}
	}

	return 0;
}