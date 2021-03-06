/*
main.cpp is the "good" code that does what we want. this file is just a scratch
to mess around with.
*/

#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"

#include <iostream>

cv::Mat do_sobel(cv::Mat in)
{
    cv::Mat img_grad_x;
    cv::Mat img_grad_y;
    cv::Sobel(in, img_grad_x, CV_64F, 1, 0);
    cv::Sobel(in, img_grad_y, CV_64F, 0, 1);
    cv::convertScaleAbs(img_grad_x, img_grad_x);
    cv::convertScaleAbs(img_grad_y, img_grad_y);

    cv::Mat out;
    cv::addWeighted(img_grad_x, 0.33, img_grad_y, 0.33, 0, out);

    return out;
}

void sobel_detection(cv::Mat img_original)
{
    cv::Mat img_blurred;
    cv::Mat img_gray;
    cv::GaussianBlur(img_original, img_blurred, cv::Size(9, 9), 0, 0, cv::BORDER_DEFAULT);
    cv::cvtColor(img_blurred, img_gray, cv::COLOR_BGR2GRAY);

    cv::imshow("sobol", do_sobel(img_original));
}

void canny_detection(cv::Mat img_original)
{
    cv::Mat img_blurred;
    cv::Mat img_gray_blurred;
    cv::Mat img_canny;

    cv::GaussianBlur(img_original, img_blurred, cv::Size(9, 9), 0, 0, cv::BORDER_DEFAULT);
    cv::cvtColor(img_blurred, img_gray_blurred, cv::COLOR_BGR2GRAY);
    cv::Canny(img_gray_blurred, img_canny, 100, 200);

    cv::imshow(__func__, img_canny);
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

    cv::imshow(__func__, img_output);
}

void morph(cv::Mat img_original)
{
    cv::Mat img_gray;
    cv::Mat img_out;

    cv::cvtColor(img_original, img_out, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(img_out, img_out, cv::Size(9, 9), 0);
    img_out = do_sobel(img_out);
    cv::morphologyEx(img_out, img_out, cv::MORPH_CLOSE, cv::Mat());

    cv::imshow(__func__, img_out);
}

/*
THRESH_TRIANGLE makes the image combine into a single solid which
then makes it easier to determine the bounds of the image for cropping

light mode = black area is what we want
dark mode = gray area is what we want
 */
void threshold(cv::Mat src)
{
    cv::Mat dst;

    src.copyTo(dst);

    cv::cvtColor(dst, dst, cv::COLOR_BGR2GRAY);
    cv::medianBlur(dst, dst, 5);
    cv::threshold(dst, dst, 100, 200, cv::THRESH_TRIANGLE);
    cv::morphologyEx(dst, dst, cv::MORPH_OPEN, cv::Mat());

    cv::imshow(__func__, dst);
}

double angle(cv::Point pt1, cv::Point pt2, cv::Point pt0)
{
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1 * dx2 + dy1 * dy2) / sqrt((dx1 * dx1 + dy1 * dy1) * (dx2 * dx2 + dy2 * dy2) + 1e-10);
}

void find_squares(cv::Mat &src, std::vector<std::vector<cv::Point>> &squares)
{
    squares.clear();

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(src, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

    std::vector<cv::Point> approx;

    for (size_t i = 0; i < contours.size(); i++)
    {
        cv::approxPolyDP(contours[i], approx, cv::arcLength(contours[i], true) * 0.02, true);

        if (approx.size() == 4 &&
            fabs(cv::contourArea(approx)) > 1000 &&
            cv::isContourConvex(approx))
        {
            double maxCosine = 0;

            for (int j = 2; j < 5; j++)
            {
                double cosine = fabs(angle(approx[j % 4], approx[j - 2], approx[j - 1]));
                maxCosine = MAX(maxCosine, cosine);
            }

            if (maxCosine < 0.3)
            {
                squares.push_back(approx);
            }
        }
    }
}

/**
 * Contour sometimes returns uneven rectangle due to rounded corners. This function
 * accounts for that and expands the rectangle to the max bounds of the src
 * vectors setting it to out.
 */
void max_square_edges(std::vector<std::vector<cv::Point>> src, std::vector<std::vector<cv::Point>> &dst)
{
    dst.clear();

    // first get the max min x and y values for each set of points
    for (size_t i = 0; i < src.size(); i++)
    {
        int max_x = INT_MIN;
        int min_x = INT_MAX;
        int max_y = INT_MIN;
        int min_y = INT_MAX;

        for (size_t j = 0; j < src[i].size(); j++)
        {
            if (max_x == INT_MIN)
                max_x = src[i][j].x;
            if (min_x == INT_MAX)
                max_x = src[i][j].x;
            if (max_y == INT_MIN)
                max_y = src[i][j].y;
            if (min_y == INT_MAX)
                min_y = src[i][j].y;

            max_x = MAX(max_x, src[i][j].x);
            min_x = MIN(min_x, src[i][j].x);

            max_y = MAX(max_y, src[i][j].y);
            min_y = MIN(min_y, src[i][j].y);

            // std::cout
            //     << "x: "
            //     << src[i][j].x
            //     << ", y: "
            //     << src[i][j].y
            //     << ", min x: "
            //     << min_x
            //     << ", max x: "
            //     << max_x
            //     << ", min y: "
            //     << min_y
            //     << ", max y: "
            //     << max_y
            //     << std::endl;
        }

        // std::cout << std::endl;
        // std::cout << "max x: " << max_x << std::endl;
        // std::cout << "min x: " << min_x << std::endl;
        // std::cout << "max y: " << max_y << std::endl;
        // std::cout << "min y: " << min_y << std::endl;

        std::vector<cv::Point> square_p;

        // index 0 - top left, min x, min y
        // index 1 - bottom left, min x, max y
        // index 2 - bottom right, max x, max y
        // index 3 - top right, max x, min y
        cv::Point top_left;
        top_left.x = min_x;
        top_left.y = min_y;
        square_p.push_back(top_left);

        cv::Point bottom_left;
        bottom_left.x = min_x;
        bottom_left.y = max_y;
        square_p.push_back(bottom_left);

        cv::Point bottom_right;
        bottom_right.x = max_x;
        bottom_right.y = max_y;
        square_p.push_back(bottom_right);

        cv::Point top_right;
        top_right.x = max_x;
        top_right.y = min_y;
        square_p.push_back(top_right);

        dst.push_back(square_p);
    }
}

/**
 * Determines if the first row of an image is white.
 */
bool first_row_is_white(cv::Mat src)
{
    cv::Mat row = src.row(0);

    // get values as 0-255
    std::vector<int> shape = row.reshape(0);

    // get average and determine if closer to 0 or 255
    size_t len = shape.size();
    int total = 0;
    for (size_t i = 0; i < shape.size(); i++)
    {
        total += shape[i];
    }

    int avg = total / len;

    // std::cout
    //     << "Average: "
    //     << avg
    //     << std::endl;

    int d_white = abs(255 - avg);
    int d_black = abs(0 - avg);
    bool is_white = d_white < d_black;

    // std::cout
    //     << "is white: "
    //     << is_white
    //     << ", d_white: "
    //     << d_white
    //     << ", d_black: "
    //     << d_black
    //     << std::endl;

    return is_white;
}

void threshold_contours(cv::Mat src)
{
    cv::Mat dst;

    src.copyTo(dst);

    cv::cvtColor(dst, dst, cv::COLOR_BGR2GRAY);
    cv::medianBlur(dst, dst, 5);
    cv::threshold(dst, dst, 0, 500, cv::THRESH_TRIANGLE);
    cv::dilate(dst, dst, cv::Mat());

    // at this point, determine if the image is a dark or light mode UI.
    // background color must be black for this to work
    if (first_row_is_white(dst))
    {
        cv::bitwise_not(dst, dst);
    }

    std::vector<std::vector<cv::Point>> maybe_squares;
    find_squares(dst, maybe_squares);

    // find_squares sometimes returns rhombuses so we need to
    // "expand" the four corners to be the max x and y values of it.
    std::vector<std::vector<cv::Point>> squares;
    max_square_edges(maybe_squares, squares);

    // cvtColor to bgr so that polylines are green and not gray
    cv::cvtColor(dst, dst, cv::COLOR_GRAY2BGR);
    cv::polylines(dst, squares, true, cv::Scalar(0, 0, 255), 3, cv::LINE_AA);

    cv::imshow(__func__, dst);
}

int main(int argc, char *argv[])
{
    cv::String img_path = argv[1];

    cv::Mat img = cv::imread(img_path, cv::IMREAD_COLOR);
    if (img.empty())
    {
        std::cout << "Could not read image: " << img_path << std::endl;
        return 1;
    }

    // canny_detection(img);
    // mser_detection(img); // rule out mser for now
    // sobel_detection(img);
    // morph(img);
    // threshold(img);
    threshold_contours(img);

    char key;
    do
    {
        key = (char)cv::waitKey(0);
    } while (key != 'q');

    return 0;
}
