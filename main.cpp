#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

cv::Mat sobel_detection(cv::Mat img_original)
{
    cv::Mat img_blurred;
    cv::Mat img_gray;
    cv::Mat img_sobol;
    cv::Mat img_grad_x;
    cv::Mat img_grad_y;

    cv::GaussianBlur(img_original, img_blurred, cv::Size(9, 9), 0, 0, cv::BORDER_DEFAULT);
    cv::cvtColor(img_blurred, img_gray, cv::COLOR_BGR2GRAY);
    cv::Sobel(img_gray, img_grad_x, CV_64F, 1, 0);
    cv::Sobel(img_gray, img_grad_y, CV_64F, 0, 1);

    cv::convertScaleAbs(img_grad_x, img_grad_x);
    cv::convertScaleAbs(img_grad_y, img_grad_y);

    cv::addWeighted(img_grad_x, 0.5, img_grad_y, 0.5, 0, img_sobol);

    // grad y is the best for vertical edge detection
    // cv::imshow("grad x", img_grad_x);

    // grad y is the best for horizontal edge detection
    // cv::imshow("grad y", img_grad_y);

    // sobel is only good for image outlines but not good for finding meme bounds
    // cv::imshow("sobol", img_sobol);

    return img_sobol;
}

cv::Mat canny_detection(cv::Mat img_original)
{
    cv::Mat img_blurred;
    cv::Mat img_gray_blurred;
    cv::Mat img_canny;

    cv::GaussianBlur(img_original, img_blurred, cv::Size(9, 9), 0, 0, cv::BORDER_DEFAULT);
    cv::cvtColor(img_blurred, img_gray_blurred, cv::COLOR_BGR2GRAY);
    cv::Canny(img_gray_blurred, img_canny, 100, 200);

    // cv::imshow("canny", img_canny);

    return img_canny;
}

cv::Mat mser_detection(cv::Mat img_original)
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

    // cv::imshow("mser", img_output);

    return img_output;
}

int main()
{
    cv::String img_path = "test_images/img_4.png";

    cv::Mat img = cv::imread(img_path, cv::IMREAD_COLOR);
    if (img.empty())
    {
        std::cout << "Could not read the image: " << img_path << std::endl;
        return 1;
    }

    cv::imshow("canny", canny_detection(img));
    // cv::imshow("mser", mser_detection(img)); // rule out mser for now
    cv::imshow("sobel", sobel_detection(img));

    char key;
    do
    {
        key = (char)cv::waitKey(0);
    } while (key != 'q');

    return 0;
}
