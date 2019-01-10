import numpy as np
import cv2

INF = 10000000
def calibrate_imgs():

    img = cv2.imread('data/real_test/real0_raw_21/19.jpg')

    img_gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    img_canny = cv2.Canny(img_gray, 70, 150)
    lines = cv2.HoughLinesP(img_canny, 1, np.pi/180, 80, 30, 10)

    left_top_point = [INF, INF]
    left_bottom_point = [INF, 0]
    right_top_point = [0, INF]
    right_bottom_point = [0, 0]
    for i in range(len(lines)):
        x0, y0, x1, y1 = lines[i][0]

        delta = (left_top_point[0] - x0) + (left_top_point[1] - y0)
        if delta > 0:
            left_top_point[0] = x0
            left_top_point[1] = y0
        
        delta = (x0 - right_top_point[0]) + (right_top_point[1] - y0)
        if delta > 0:
            right_top_point[0] = x0
            right_top_point[1] = y0
        
        delta = (left_bottom_point[0] - x0) + (y0 - left_bottom_point[1])
        if delta > 0:
            left_bottom_point[0] = x0
            left_bottom_point[1] = y0
        
        delta = (x0 - right_bottom_point[0]) + (y0 - right_bottom_point[1])
        if delta > 0:
            right_bottom_point[0] = x0
            right_bottom_point[1] = y0

    print(left_top_point, left_bottom_point, right_top_point, right_bottom_point)

    height = (3938 - 151 + 3948 - 163) / 2
    # cv2.imshow('img', img)
    # cv2.imshow('canvas', img_canny)
    # cv2.waitKey()

if __name__ == '__main__':
    calibrate_imgs()