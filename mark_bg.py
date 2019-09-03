import os
import cv2
import numpy as np

def mark_bg(img):
    
    color_thres = 200
    block_h = 20
    left_x = 0
    block_cnt = 0
    out_width = 0

    height, width, channels = img.shape

    mask = np.ones(img.shape[:2], dtype=np.uint8) * 255

    for y in range(block_h, height - block_h, block_h):

        x = 0
        while x < width:
            c = img[y][x]
            if c[0] > color_thres and c[1] > color_thres and c[2] > color_thres:
                break
            x += 1
        
        if y == block_h:
            left_x = x
        else:
            left_x = int(round(0.8 * left_x + 0.2 * x))

        x = width - 1
        while x >= 0:
            c = img[y][x]
            if c[0] > color_thres and c[1] > color_thres and c[2] > color_thres:
                break
            x -= 1
        
        if y == block_h:
            right_x = x
        else:
            right_x = int(round(0.8 * right_x + 0.2 * x))
        
        for y_ in range(y - block_h, y):
            for x_ in range(0, left_x):
                mask[y_][x_] = 0
            for x_ in range(right_x + 1, width):
                mask[y_][x_] = 0

    # For last block
    for y_ in range(y - block_h, height):
        for x_ in range(0, left_x):
            mask[y_][x_] = 0
        for x_ in range(right_x + 1, width):
            mask[y_][x_] = 0

    return mask

def vis_mask(mask):

    cv2.imshow('img', img)
    cv2.imshow('mask', mask)
    cv2.waitKey()


if __name__ == '__main__':

    dataset_folder = 'data/stripes/real3_39'
    stripe_list = os.listdir(dataset_folder)
    stripe_list.sort(key = lambda x: (len(x), x))

    print(stripe_list)

    mask_folder = os.path.join(dataset_folder, 'bg_mask')
    mask_vis_folder = os.path.join(dataset_folder, 'bg_mask_vis')

    if not os.path.exists(mask_folder):
        os.makedirs(mask_folder)

    for file_name in stripe_list:
        if file_name[-4:] != '.png':
            continue
        
        print('Processing', file_name)
        stripe_path = os.path.join(dataset_folder, file_name)
        img = cv2.imread(stripe_path)
        mask = mark_bg(img)
        
        # vis_mask(img, mask)

        mask_vis_path = os.path.join(mask_vis_folder, file_name[:-4] + '.png')
        cv2.imwrite(mask_vis_path, mask)

        mask_path = os.path.join(mask_folder, file_name[:-4] + '.npy')
        np.save(mask_path, mask)
        
    