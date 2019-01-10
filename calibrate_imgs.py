import numpy as np
import cv2

def calibrate_imgs():

    img = cv2.imread('data/real_test/real0_raw_21/19.jpg')
    img = cv2.resize(img, None, None, 0.2, 0.2, cv2.INTER_LINEAR)

    img_gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    img_canny = cv2.Canny(img_gray, 200, 300)
    a, b = cv2.findContours(img_canny, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
    print(len(a))
    print(b.shape)
    cv2.imshow('img', img_canny)
    cv2.waitKey()

if __name__ == '__main__':
    calibrate_imgs()