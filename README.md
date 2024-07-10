## Project Overview

OpenCV를 활용하여 video에서의 도로의 차선을 검출하는 방법을 구현하였다. 주요 과정은  detecting lane colers, edge detection, defining the region of interest, appling Hough Transform, calculating slopes, performing linear regression , drawing lanes 로 구성된다. 

## Key Steps

- **Lane Color Detection**
    - Filter for white (BGR: (100,100,100) - (255,200,255)) and blue (HSV: (90,80,80) - (130,255,255)) colors.
- **Preprocessing**
    - Convert to grayscale.
    - Apply Gaussian blur (5x5).
- **Edge Detection**
    - Use Canny Edge Detection (minVal: 50, maxVal: 150).
- **Region of Interest (ROI)**
    - Define specific ROIs for each clip to focus on the lanes.
- **Hough Transform**
    - Use HoughLinesP (threshold: 100, minLineLength: 100, maxLineGap: 10) to detect lines.
- **Lane Line Calculation**
    - Filter meaningful lines by calculating their slopes.
    - Perform linear regression to get the best-fit lines for left and right lanes.
- **Draw Lanes**
    - Draw the detected lane lines and fill the lane area with red using `fillConvexPoly()`.
    - Draw the lane boundaries with yellow lines using `line()`.

for more information, [Report](https://drive.google.com/file/d/1snk370-txn2Kq3XPbealdH5UUNiuM_b4/view?usp=drive_link)

## Results

- clip1
![image](https://github.com/youngseoh/Lane_Detection/assets/100707876/b1369823-6d91-4e07-9a19-14c66207020c)
- clip2
![image](https://github.com/youngseoh/Lane_Detection/assets/100707876/c6f529b4-e229-4be2-8629-f6975a723acf)
- clip3
  ![image](https://github.com/youngseoh/Lane_Detection/assets/100707876/40cfa5b5-59cb-41e4-a128-3faacaa2cfb3)
