import numpy as np
import cv2
import os

INF = 10000000
def calibrate_imgs(img, crop_flag=False):

    if crop_flag:
        width_margin = 140
        height_margin = 110
        height = img.shape[0]
        width = img.shape[1]
        img = img[height_margin:height - height_margin, width_margin:width - width_margin]

    img_gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    img_x = cv2.Sobel(img_gray, cv2.CV_16S, 1, 0)
    img_absx = cv2.convertScaleAbs(img_x)
    _, img_bin = cv2.threshold(img_absx, 128, 255, cv2.THRESH_BINARY)
    lines = cv2.HoughLinesP(img_bin, 1, np.pi/180, 80, 30, 10)

    x_left = INF
    x_right = 0
    for i in range(len(lines)):
        x0, y0, x1, y1 = lines[i][0]

        if abs(x0 - x1) > 20:
            continue
        
        if abs(y0 - y1) < 20:
            continue

        x_left = min(x_left, x0)
        x_right = max(x_right, x0)


    # print(left_top_point, left_bottom_point, right_top_point, right_bottom_point)

    dst_height = 1800 # 6450
    src_height = img.shape[0]
    src_width = x_right - x_left
    dst_width = int(round(src_width * dst_height / src_height))

    # src_anchor = np.float32([left_top_point, left_bottom_point, right_top_point, right_bottom_point])
    # # dst_anchor = np.float32( \
    # #     [left_top_point, \
    # #     [left_top_point[0], left_top_point[1] + dst_height], \
    # #     [left_top_point[0] + dst_width, left_top_point[1]], \
    # #     [left_top_point[0] + dst_width, left_top_point[1] + dst_height]])
    # dst_anchor = np.float32( \
    #     [[0, 0], \
    #     [0, dst_height], \
    #     [dst_width, 0], \
    #     [dst_width, dst_height]])
    # print(src_anchor)
    # print(dst_anchor)

    # mat = cv2.getPerspectiveTransform(src_anchor, dst_anchor)
    # print(mat)
    # stripe = cv2.warpPerspective(img, mat, (dst_width, dst_height))

    stripe = img[:, x_left:x_right]
    stripe = cv2.resize(stripe, (dst_width, dst_height))

    return stripe

if __name__ == '__main__':

    test_case = 'real1_31/'
    img_num = 31
    in_folder = 'data/real_test/' + test_case
    out_folder = 'data/stripes/' + test_case
    if not os.path.isdir(out_folder):
        os.mkdir(out_folder)

    order_path = out_folder + 'order.txt'
    order_file = open(order_path, 'w')
    for img_id in range(img_num):
        order_file.write(str(img_id) + '\n')
    order_file.close()

    for img_id in range(img_num):

        print('current img:', img_id)

        img_path = in_folder + str(img_id+1) + '.XSM/' + '00000001.jpg'
        img = cv2.imread(img_path)
        stripe = calibrate_imgs(img, crop_flag=True)
        cv2.imwrite(out_folder + str(img_id) + '.png', stripe)