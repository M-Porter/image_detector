#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

void sobel_detection(cv::Mat img_original)
{
}

void canny_detection(cv::Mat img_original)
{
    cv::Mat img_blurred;
    cv::Mat img_gray_blurred;
    cv::Mat img_canny;

    cv::GaussianBlur(img_original, img_blurred, cv::Size(9, 9), 0, 0, cv::BORDER_DEFAULT);
    cv::cvtColor(img_blurred, img_gray_blurred, cv::COLOR_RGBA2GRAY);
    cv::Canny(img_gray_blurred, img_canny, 100, 200);

    cv::imshow("canny", img_canny);

    return;
}

void mser_detection(cv::Mat img_original)
{
    cv::Mat img_blurred;
    cv::Mat img_output;

    img_original.copyTo(img_output);

    cv::GaussianBlur(img_original, img_blurred, cv::Size(99, 99), 0);

    cv::Ptr<cv::MSER> mser = cv::MSER::create();
    std::vector<std::vector<cv::Point>> regions;
    std::vector<cv::Rect> bbox;
    mser->detectRegions(img_blurred, regions, bbox);

    for (int i = 0; i < regions.size(); i++)
    {
        cv::rectangle(img_output, bbox[i], CV_RGB(0, 255, 0));
    }

    cv::imshow("mser", img_output);

    return;
}

int main()
{
    cv::String img_path = "test_images/img_1.png";

    cv::Mat img_original = cv::imread(img_path, cv::IMREAD_COLOR);
    if (img_original.empty())
    {
        std::cout << "Could not read the image: " << img_path << std::endl;
        return 1;
    }

    canny_detection(img_original);
    mser_detection(img_original);

    char key;
    do
    {
        key = (char)cv::waitKey(0);
    } while (key != 'q');

    return 0;
}
