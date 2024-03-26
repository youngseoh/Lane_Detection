#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/xfeatures2d.hpp>
using namespace cv;
using namespace std;


int main() {

    cv::VideoCapture cap("clip1.mp4");

    if (!cap.isOpened()) {
        std::cout << "Cannot open the video file" << std::endl;
        return -1;
    }

    while (true) {
        cv::Mat frame;
        cap >> frame;

        if (frame.empty()) {
            std::cout << "Video is empty" << std::endl;
            break;
        }


        // 1. 차선 색깔인 흰색, 하늘색만 검출 되도록 
        cv::Scalar lower_white = cv::Scalar(100, 100, 100);
        cv::Scalar upper_white = cv::Scalar(255, 200, 255);
        cv::Mat white_mask, white_image;
        cv::inRange(frame, lower_white, upper_white, white_mask);
        cv::bitwise_and(frame, frame, white_image, white_mask);

        cv::Mat hsv;
        cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
        cv::Scalar lower_blue = cv::Scalar(90, 80, 80);
        cv::Scalar upper_blue = cv::Scalar(130, 255, 255);
        cv::Mat blue_mask, blue_image;
        cv::inRange(hsv, lower_blue, upper_blue, blue_mask);
        cv::bitwise_and(frame, frame, blue_image, blue_mask);

        cv::Mat result_image;
        cv::addWeighted(white_image, 1.0, blue_image, 1.0, 0.0, result_image);



        // 2. 그레이스케일로 변환
        Mat gray;
        cvtColor(result_image, gray, COLOR_BGR2GRAY);

        // 3. 가우시안 블러 적용 
        Mat blur;
        GaussianBlur(gray, blur, Size(5, 5), 0);

        // 4. 캐니 엣지 검출
        Mat edges;
        Canny(blur, edges, 50, 150);


        // 5. 관심 영역 지정 
        Mat output;



        int width = edges.cols;
        int height = edges.rows;


        Mat mask1 = Mat::zeros(height, width, CV_8UC1);

        // 직사각형 좌표 계산
        Point points[4]{
            Point(width * 0.15, height * 0.8),   // 좌측 하단
            Point(width * 0.38, height * 0.4),   // 좌측 상단
            Point(width * 0.5, height * 0.4),   // 우측 상단
            Point(width * 0.85, height * 0.8)   // 우측 하단

        };
        

          

        // 직사각형 내부를 색칠하여 ROI 영역을 생성
        fillConvexPoly(mask1, points, 4, cv::Scalar(255, 255, 255));

        // ROI 영역만 추출
        bitwise_and(edges, mask1, output);


        // 6. 허프 변환으로 에지에서의 직선 성분 추출 
        vector<Vec4i> lines;
        HoughLinesP(output, lines, 1, CV_PI / 180, 50, 50, 10);


        if (lines.size() > 0) {
            vector<vector<Vec4i>> output1(2);
            Point point1, point2;
            vector<double> slopes;
            vector<Vec4i> final_lines, left_lines, right_lines;
            double slope_thresh = 0.3;


            // 7. 검출된 직선들의 기울기를 계산
            for (int i = 0; i < lines.size(); i++) {
                Vec4i line = lines[i];
                point1 = Point(line[0], line[1]);
                point2 = Point(line[2], line[3]);

                double slope;
                if (point2.x - point1.x == 0)
                    slope = 999.0;
                else
                    slope = (point2.y - point1.y) / (double)(point2.x - point1.x);


                if (abs(slope) > slope_thresh) {
                    slopes.push_back(slope);
                    final_lines.push_back(line);
                }
            }


            // 8. 선들을 좌우 선으로 분류
            bool right_detect = false, left_detect = false;
            double img_center = (double)((output.cols / 2));

            for (int i = 0; i < final_lines.size(); i++) {
                point1 = Point(final_lines[i][0], final_lines[i][1]);
                point2 = Point(final_lines[i][2], final_lines[i][3]);

                if (slopes[i] > 0 && point1.x > img_center && point2.x > img_center) {
                    right_detect = true;
                    right_lines.push_back(final_lines[i]);
                }
                else if (slopes[i] < 0 && point1.x < img_center && point2.x < img_center) {
                    left_detect = true;
                    left_lines.push_back(final_lines[i]);
                }
            }

            output1[0] = right_lines;
            output1[1] = left_lines;




            if ((right_lines.size() > 0) and (left_lines.size() > 0)) {

                // 9. 선형회귀로 적합한 선 찾기 
                vector<Point> output2(4);
                Point p1, p2, p3, p4;
                Vec4d left_line, right_line;
                vector<Point> left_points, right_points;

                double left_m, right_m;
                Point left_b, right_b;

                if (right_detect) {
                    for (auto i : output1[0]) {
                        p1 = Point(i[0], i[1]);
                        p2 = Point(i[2], i[3]);

                        right_points.push_back(p1);
                        right_points.push_back(p2);
                    }

                    if (right_points.size() > 0) {

                        fitLine(right_points, right_line, DIST_L2, 0, 0.01, 0.01);

                        right_m = right_line[1] / right_line[0];
                        right_b = Point(right_line[2], right_line[3]);
                    }
                }

                if (left_detect) {
                    for (auto j : output1[1]) {
                        p3 = Point(j[0], j[1]);
                        p4 = Point(j[2], j[3]);

                        left_points.push_back(p3);
                        left_points.push_back(p4);
                    }

                    if (left_points.size() > 0) {

                        fitLine(left_points, left_line, DIST_L2, 0, 0.01, 0.01);

                        left_m = left_line[1] / left_line[0];
                        left_b = Point(left_line[2], left_line[3]);
                    }
                }


                //y = m*x + b  --> x = (y-b) / m
                int y1 = (frame.rows) * 0.8;
                int y2 = (frame.cols) * 0.35;

                double right_x1 = ((y1 - right_b.y) / right_m) + right_b.x;
                double right_x2 = ((y2 - right_b.y) / right_m) + right_b.x;

                double left_x1 = ((y1 - left_b.y) / left_m) + left_b.x;
                double left_x2 = ((y2 - left_b.y) / left_m) + left_b.x;

                output2[0] = Point(right_x1, y1);
                output2[1] = Point(right_x2, y2);
                output2[2] = Point(left_x1, y1);
                output2[3] = Point(left_x2, y2);




                // 10. 선그리기 
                vector<Point> poly_points;
                Mat output3;
                frame.copyTo(output3);

                poly_points.push_back(output2[2]);
                poly_points.push_back(output2[0]);
                poly_points.push_back(output2[1]);
                poly_points.push_back(output2[3]);

                fillConvexPoly(output3, poly_points, Scalar(255,0,0), LINE_AA, 0);
                addWeighted(output3, 0.3, frame, 0.7, 0, frame);


                line(frame, output2[0], output2[1], Scalar(0, 255, 255), 5, LINE_AA);
                line(frame, output2[2], output2[3], Scalar(0, 255, 255), 5, LINE_AA);
            }



        };


        imshow("lane_detection", frame);


        if (cv::waitKey(30) == 27) {
            break;
        }
    }

    return 0;
}
