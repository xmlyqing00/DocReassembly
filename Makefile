OPT_FLAGS = -O2
CXX = g++
CXX_FLAGS = --std=c++17 -Wall

INCLUDES = -I./include/
OPENCV_INCS = -I/usr/include/opencv2
OPENCV_LIBS = -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_videostab -lopencv_aruco -lopencv_bgsegm -lopencv_bioinspired -lopencv_ccalib -lopencv_datasets -lopencv_dpm -lopencv_face -lopencv_fuzzy -lopencv_line_descriptor -lopencv_optflow -lopencv_video -lopencv_plot -lopencv_reg -lopencv_saliency -lopencv_stereo -lopencv_structured_light -lopencv_phase_unwrapping -lopencv_rgbd -lopencv_surface_matching -lopencv_ximgproc -lopencv_calib3d -lopencv_features2d -lopencv_flann -lopencv_xobjdetect -lopencv_objdetect -lopencv_ml -lopencv_xphoto -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_photo -lopencv_imgproc -lopencv_core
TESSARACT_LIBS = -ltesseract

src_dir = ./src/
dst_dir = ./bin/

generator_src = 	$(wildcard $(src_dir)generator/*.cpp) utils.cpp
generator_dep = 	$(generator_src:.cpp=.d)
generator_obj = 	$(generator_src:.cpp=.o)

solver_src = 		$(wildcard $(src_dir)solver/*.cpp) utils.cpp
solver_dep = 		$(solver_src:.cpp=.d)
solver_obj =		$(solver_src:.cpp=.o)
solver_obj_debug = 	$(solver_src:.cpp=.debug.o)

add_noise_obj = 	$(src_dir)add_noise.o

.PHONY: clean default debug all
default: solve-puzzle generate-puzzle add-noise
debug: solve-puzzle-debug
all: default debug

%.debug.o: %.cpp
	$(CXX) -c $< -o $@ -MMD $(CXX_FLAGS) $(INCLUDES) -DDEBUG -g
%.o: %.cpp
	$(CXX) -c $< -o $@ -MMD $(CXX_FLAGS) $(INCLUDES) $(OPT_FLAGS)

-include $(generator_dep)
-include $(solver_dep)

generate-puzzle: $(generator_obj)
	$(CXX) $^ -o $(dst_dir)$@ $(CXX_FLAGS) $(OPENCV_LIBS)

add-noise: $(add_noise_obj)
	$(CXX) $^ -o $(dst_dir)$@ $(CXX_FLAGS) $(OPENCV_LIBS)

solve-puzzle: $(solver_obj)
	$(CXX) $^ -o $(dst_dir)$@ $(CXX_FLAGS) $(TESSARACT_LIBS) $(OPENCV_LIBS)

solve-puzzle-debug: $(solver_obj_debug)
	$(CXX) $^ -o $(dst_dir)$@ $(CXX_FLAGS) $(TESSARACT_LIBS) $(OPENCV_LIBS) 

clean:
	rm $(src_dir)*.o $(src_dir)*.d
	rm $(dst_dir)generate-puzzle $(dst_dir)solve-puzzle $(dst_dir)solve-puzzle-debug
	

