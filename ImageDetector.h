#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"

#include <stdio.h>

namespace ImageDetector
{
    class ImageDetails
    {
    public:
        ImageDetails();
        ImageDetails(int x, int y, int h, int w);

        int area();

        int x;
        int y;
        int h;
        int w;
    };

    /**
     * Same as Detect but returns a more structured response.
     */
    ImageDetector::ImageDetails detect_v2(cv::Mat src);

    /**
     * Helper function to detect_v2
     */
    ImageDetector::ImageDetails detect_inverse_optional(cv::Mat src, bool inverse = false);

    /**
     * Returns a vector of points that represent the 4 verticies of the found square image.
     */
    std::vector<cv::Point> detect(cv::Mat src);

    /**
     * Calculates the angle between points.
     */
    double angle(cv::Point pt1, cv::Point pt2, cv::Point pt0);

    /**
     * Finds all the squares in the src image.
     */
    void find_squares(cv::Mat &src, std::vector<std::vector<cv::Point>> &squares);

    /**
     * Contour sometimes returns uneven rectangle due to rounded corners. This function
     * accounts for that and expands the rectangle to the max bounds of the src
     * vectors setting it to out.
     */
    void max_square_edges(std::vector<std::vector<cv::Point>> src, std::vector<std::vector<cv::Point>> &dst);

    /**
     * Returns the average int color value (0-255) across the row.
     */
    int avg_color_row(cv::Mat row);

    /**
     * Determines if the first row of an image is white.
     */
    bool first_row_is_white(cv::Mat src);

    /**
     * Finds the largest square by area
     */
    void largest_area(std::vector<std::vector<cv::Point>> squares, std::vector<cv::Point> &dst);

} // namespace ImageDetector
