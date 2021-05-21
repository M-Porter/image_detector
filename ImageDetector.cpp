#include "ImageDetector.h"

#include <stdio.h>

namespace ImageDetector
{
    class ImageDetails
    {
    public:
        ImageDetails()
        {
            x = 0;
            y = 0;
            h = 0;
            w = 0;
        }

        ImageDetails(int x, int y, int h, int w)
        {
            this.x = x;
            this.y = y;
            this.h = h;
            this.w = w;
        }

        int area()
        {
            return h * w;
        }

        int x;
        int y;
        int h;
        int w;
    };

    ImageDetector::ImageDetails detect_v2(cv::Mat src)
    {
        // get two versions of the cropped images. Based on the incoming image
        // and where ite was cropped from, the bitwise_not may do an inverse
        // where not needed.
        ImageDetails id_a = detect_inverse_optional(src, true);
        ImageDetails id_b = detect_inverse_optional(src, false);

        return id_a.area() > id_b.area() ? id_a : id_b;
    } // detect_v2

    ImageDetector::ImageDetails detect_inverse_optional(cv::Mat src, bool inverse = false)
    {
        cv::Mat dst;
        src.copyTo(dst);

        // apply some filters to get started
        cv::cvtColor(dst, dst, cv::COLOR_BGR2GRAY);
        cv::medianBlur(dst, dst, 5);
        cv::threshold(dst, dst, 0, 500, cv::THRESH_TRIANGLE);
        cv::morphologyEx(dst, dst, cv::MORPH_ERODE, cv::Mat());

        if (inverse)
        {
            cv::bitwise_not(dst, dst);
        }

        // find potential squares
        std::vector<std::vector<cv::Point>> maybe_squares;
        find_squares(dst, maybe_squares);

        // find_squares sometimes returns rhombuses so we need to
        // "expand" the four corners to be the max x and y values of it.
        std::vector<std::vector<cv::Point>> squares;
        max_square_edges(maybe_squares, squares);

        // get largest square.
        std::vector<cv::Point> l_sq;
        largest_area(squares, l_sq);

        if (l_sq.size() != 4)
        {
            return ImageDetails();
        }

        ImageDetails id = ImageDetails();
        id.x = l_sq[0].x;
        id.y = l_sq[0].y;
        id.w = l_sq[3].x - l_sq[1].x;
        id.h = l_sq[1].x - l_sq[0].x;
        return id;
    } // detect_inverse_optional

    std::vector<cv::Point> detect(cv::Mat src)
    {
        cv::Mat dst;
        src.copyTo(dst);

        // apply some filters to get started
        cv::cvtColor(dst, dst, cv::COLOR_BGR2GRAY);
        cv::medianBlur(dst, dst, 5);
        cv::threshold(dst, dst, 0, 500, cv::THRESH_TRIANGLE);
        cv::morphologyEx(dst, dst, cv::MORPH_ERODE, cv::Mat());

        // at this point, determine if the image is a dark or light mode UI.
        // background color must be black for this to work
        if (first_row_is_white(dst))
        {
            cv::bitwise_not(dst, dst);
        }

        // find potential squares
        std::vector<std::vector<cv::Point>> maybe_squares;
        find_squares(dst, maybe_squares);

        // find_squares sometimes returns rhombuses so we need to
        // "expand" the four corners to be the max x and y values of it.
        std::vector<std::vector<cv::Point>> squares;
        max_square_edges(maybe_squares, squares);

        // get largest square.
        std::vector<cv::Point> l_sq;
        largest_area(squares, l_sq);

        return l_sq;
    } // detect

    double angle(cv::Point pt1, cv::Point pt2, cv::Point pt0)
    {
        double dx1 = pt1.x - pt0.x;
        double dy1 = pt1.y - pt0.y;
        double dx2 = pt2.x - pt0.x;
        double dy2 = pt2.y - pt0.y;
        return (dx1 * dx2 + dy1 * dy2) / sqrt((dx1 * dx1 + dy1 * dy1) * (dx2 * dx2 + dy2 * dy2) + 1e-10);
    } // angle

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
    } // find_squares

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
            }

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
    } // max_square_edges

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
    } // avg_color_row

    bool first_row_is_white(cv::Mat src)
    {
        int avg = avg_color_row(src.row(0));
        int d_white = abs(255 - avg);
        int d_black = abs(0 - avg);
        return d_white < d_black;
    } // first_row_is_white

    void largest_area(std::vector<std::vector<cv::Point>> squares, std::vector<cv::Point> &dst)
    {
        dst.clear();

        int l_area = 0;
        std::vector<cv::Point> l_square = squares[0];

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
    } // largest_area

} // namespace ImageDetector
