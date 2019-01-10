import numpy as np
import cv2

def calc_areas(contour):
    pass
    return 1
    
def calibrate_imgs():

    img = cv2.imread('data/real_test/real0_raw_21/19.jpg')
    img = cv2.resize(img, None, None, 0.2, 0.2, cv2.INTER_LINEAR)

    img_gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    img_canny = cv2.Canny(img_gray, 200, 300)
    img2, contours, hierarchy = cv2.findContours(img_canny, cv2.RETR_LIST, cv2.CHAIN_APPROX_SIMPLE)

    max_area = 0
    max_area_id = 0
    for i in range(len(contours)):
        area = calc_areas(contours[i])
        if max_area < area:
            max_area = area
            max_area_id = i

    # cv2.drawContours(img, contours, -1, (0, 255, 0))
    print(len(contours))
    # print(b.shape)
    cv2.imshow('img', img)
    cv2.waitKey()

if __name__ == '__main__':
    calibrate_imgs()