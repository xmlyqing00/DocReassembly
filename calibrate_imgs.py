import numpy as np
import cv2

INF = 10000000
def calibrate_imgs():

    img = cv2.imread('data/real_test/real0_raw_21/19.jpg')

    img_gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    img_canny = cv2.Canny(img_gray, 70, 150)
    lines = cv2.HoughLinesP(img_canny, 1, np.pi/180, 80, 30, 10)

    left_top_point = np.array([INF, INF])
    left_bottom_point = np.array([INF, 0])
    right_top_point = np.array([0, INF])
    right_bottom_point = np.array([0, 0])
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

    dst_height = 3800
    src_height = max(left_bottom_point[1] - left_top_point[1], right_bottom_point[1] - right_top_point[1])
    src_width = max(right_top_point[0] - left_top_point[0], right_bottom_point[0] - left_bottom_point[0])
    dst_width = int(round(src_width * src_height / dst_height))

    vec_top = right_top_point - left_top_point
    vec_bottom = right_bottom_point - left_bottom_point
    angle_top = np.angle(vec_top[0] + 1j * vec_top[1])
    angle_bottom = np.angle(vec_bottom[0] + 1j * vec_bottom[1])
    angle_avg = (angle_top + angle_bottom) / 2

    mat = cv2.getRotationMatrix2D(tuple(left_top_point), angle_avg, 1)

    # src_anchor = np.float32([left_top_point, left_bottom_point, right_top_point])
    dst_anchor = np.int32( \
        [left_top_point, \
        [left_top_point[0], left_top_point[1] + dst_height], \
        [left_top_point[0] + dst_width, left_top_point[1]], \
        [left_top_point[0] + dst_width, left_top_point[1] + dst_height]])
    # dst_anchor = np.float32( \
        # [[0, 0], \
        # [0, dst_height], \
        # [dst_width, 0]])
    # print(src_anchor)
    # print(dst_anchor)

    img_warp = cv2.warpAffine(img, mat, (img.shape[1], img.shape[0]))
    stripe = img_warp[left_top_point[1]:left_top_point[1]+dst_height, left_top_point[0]:left_top_point[0]+dst_width]

    cv2.imwrite('data/stripes/real0_21/19.png', stripe)
    cv2.imshow('stripe', stripe)
    # cv2.imshow('img', img)
    # cv2.imshow('canvas', img_canny)
    cv2.waitKey()

if __name__ == '__main__':
    calibrate_imgs()