OPT_FLAGS = -O3
CXX = g++
CXX_FLAGS = $(OPT_FLAGS) --std=c++17 -Wall

INCLUDES = -I./include/
OPENCV_INCS = -I/usr/include/opencv2
OPENCV_LIBS = -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_videostab -lopencv_aruco -lopencv_bgsegm -lopencv_bioinspired -lopencv_ccalib -lopencv_datasets -lopencv_dpm -lopencv_face -lopencv_freetype -lopencv_fuzzy -lopencv_hdf -lopencv_line_descriptor -lopencv_optflow -lopencv_video -lopencv_plot -lopencv_reg -lopencv_saliency -lopencv_stereo -lopencv_structured_light -lopencv_phase_unwrapping -lopencv_rgbd -lopencv_viz -lopencv_surface_matching -lopencv_text -lopencv_ximgproc -lopencv_calib3d -lopencv_features2d -lopencv_flann -lopencv_xobjdetect -lopencv_objdetect -lopencv_ml -lopencv_xphoto -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_photo -lopencv_imgproc -lopencv_core
TESSARACT_LIBS = -llept -ltesseract

src_dir = ./src/
dst_dir = ./
src = $(wildcard $(src_dir)*.cpp)
obj = $(src:.cpp=.o)
dep = $(src:.cpp=.d)

default: StripeReassembly
.PHONY: clean

%.o: %.cpp
	$(CXX) -c $< -o $@ -MMD $(CXX_FLAGS) $(INCLUDES) $(OPENCV_INCS) 

-include $(dep)

StripeReassembly: $(obj)
	$(CXX) $< $(TESSARACT_LIBS) $(OPENCV_LIBS) -o $(dst_dir)$@

clean:
	rm $(dst)StripeReassembly $(obj) $(dep)

