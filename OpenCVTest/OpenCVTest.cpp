// OpenCVTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

using namespace cv;
using namespace std;

const int max_value_H = 360 / 2;
const int max_value = 255;
int low_H = 0, low_S = 0, low_V = 0;
int high_H = max_value_H, high_S = max_value, high_V = max_value;

double euclidDist(const cv::Vec4i& line)
{
	return std::sqrt(static_cast<double>((line[0] - line[2]) * (line[0] - line[2]) + (line[1] - line[3]) * (line[1] - line[3])));
}	

//make this just return a line
std::vector<cv::Vec4i> filterLinesByLen(const std::vector<cv::Vec4i>& lines, double minLen)
{
	std::vector<cv::Vec4i> longLines;
	for (cv::Vec4i line : lines)
	{
		if (euclidDist(line) >= minLen) { longLines.push_back(line); }
	}
	return longLines;
}

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		cout << " Usage: display_image ImageToLoadAndDisplay" << endl;
		return -1;
	}

	Mat image, dst, cdst, cdstP, frame_HSV, whiteColors, whitey;
	image = imread(argv[1], CV_LOAD_IMAGE_COLOR);   // Read the file

	if (!image.data)                              // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	//FILTER TO GET LINE
	cvtColor(image, frame_HSV, COLOR_BGR2HSV);	
	inRange(frame_HSV, Scalar(low_H, low_S, low_V), Scalar(0, 0, 255), whiteColors);
	bitwise_and(image, image, whitey, whiteColors);
	imshow("whitey", whitey);
	waitKey(0); 
	
	Canny(whitey, dst, 50, 200, 3);
	cvtColor(dst, cdst, COLOR_GRAY2BGR);
	cdstP = cdst.clone();
	vector<Vec2f> lines; // will hold the results of the detection
	HoughLines(dst, lines, 1, CV_PI / 180, 150, 0, 0); // runs the actual detection
													   // Draw the lines
	for (size_t i = 0; i < lines.size(); i++)
	{
		float rho = lines[i][0], theta = lines[i][1];
		Point pt1, pt2;
		double a = cos(theta), b = sin(theta);
		double x0 = a * rho, y0 = b * rho;
		pt1.x = cvRound(x0 + 1000 * (-b));
		pt1.y = cvRound(y0 + 1000 * (a));
		pt2.x = cvRound(x0 - 1000 * (-b));
		pt2.y = cvRound(y0 - 1000 * (a));
		line(cdst, pt1, pt2, Scalar(0, 0, 255), 3, LINE_AA);
	}
	// Probabilistic Line Transform
	vector<Vec4i> linesP; // will hold the results of the detection
	HoughLinesP(dst, linesP, 1, CV_PI / 180, 50, 50, 10); // runs the actual detection
														  // Draw the lines

	//take the lines that we want at the lower half of the screen, then use that. 
	vector<Vec4i> longestLine = filterLinesByLen(linesP, image.cols-100);
	for (size_t i = 0; i < longestLine.size(); i++)
	{
		Vec4i l = longestLine[i];
		line(cdstP, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 3, LINE_AA);
	}
	// Show results
	imshow("Source", image);	
	imshow("Detected Lines (in red) - Standard Hough Line Transform", cdst);	
	imshow("Detected Lines (in red) - Probabilistic Line Transform", cdstP);
	// Wait and Exit
	waitKey();
	return 0;
}
