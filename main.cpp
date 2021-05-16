#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"

#include <iostream>

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

            std::cout
                << "x: " << src[i][j].x
                << "\ty: " << src[i][j].y
                << "\tmin x: " << min_x
                << "\tmax x: " << max_x
                << "\tmin y: " << min_y
                << "\tmax y: " << max_y
                << std::endl;
        }

        std::cout << std::endl;

        std::cout
            << "max x: " << max_x
            << "\tmax y: " << max_y
            << "\tmax y: " << max_y
            << "\tmin y: " << min_y
            << std::endl
            << std::endl;

        std::vector<cv::Point> square_p;

        // index 0 - top left, min x, min y
        cv::Point top_left;
        top_left.x = min_x;
        top_left.y = min_y;
        square_p.push_back(top_left);

        // index 1 - bottom left, min x, max y
        cv::Point bottom_left;
        bottom_left.x = min_x;
        bottom_left.y = max_y;
        square_p.push_back(bottom_left);

        // index 2 - bottom right, max x, max y
        cv::Point bottom_right;
        bottom_right.x = max_x;
        bottom_right.y = max_y;
        square_p.push_back(bottom_right);

        // index 3 - top right, max x, min y
        cv::Point top_right;
        top_right.x = max_x;
        top_right.y = min_y;
        square_p.push_back(top_right);

        dst.push_back(square_p);
    }
}

int avg_color_row(cv::Mat row)
{
    // get values as 0-255
    std::vector<int> shape = row.reshape(0);

    // get average and determine if closer to 0 or 255
    size_t len = shape.size();
    int total = 0;
    for (size_t i = 0; i < shape.size(); i++)
    {
        total += shape[i];
    }

    return total / len;
}

/**
 * Determines if the first row of an image is white.
 */
bool first_row_is_white(cv::Mat src)
{
    int avg = avg_color_row(src.row(0));
    int d_white = abs(255 - avg);
    int d_black = abs(0 - avg);
    bool is_white = d_white < d_black;

    std::cout
        << "avg: " << avg
        << "\tis_white: " << is_white
        << "\td_white: " << d_white
        << "\td_black: " << d_black
        << std::endl
        << std::endl;

    return is_white;
}

/**
 * Finds the largest square by area
 */
void largest_area(std::vector<std::vector<cv::Point>> squares, std::vector<cv::Point> &dst)
{
    dst.clear();

    int l_area;
    std::vector<cv::Point> l_square;

    // loop over squares
    for (size_t i = 0; i < squares.size(); i++)
    {
        // 3rd point is the bottom right so we can calculate the area off that
        int h = squares[i][0].y - squares[i][1].y;
        int w = squares[i][1].x - squares[i][2].x;
        int area = h * w;

        if (i == 0)
        {
            l_square = squares[i];
            l_area = area;
            continue;
        }

        if (area > l_area)
        {
            l_area = area;
            l_square = squares[i];
        }
    }

    dst = l_square;
}

void find_image(cv::Mat src)
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

    // get largest square.
    std::vector<std::vector<cv::Point>> l_sqs;
    std::vector<cv::Point> l_sq;
    largest_area(squares, l_sq);
    l_sqs.push_back(l_sq);

    // cvtColor to bgr so that polylines are green and not gray
    cv::cvtColor(dst, dst, cv::COLOR_GRAY2BGR);
    cv::polylines(dst, l_sqs, true, cv::Scalar(0, 255, 0), 3, cv::LINE_AA);

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

    find_image(img);

    char key;
    do
    {
        key = (char)cv::waitKey(0);
    } while (key != 'q');

    return 0;
}
