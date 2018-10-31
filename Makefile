OPT_FLAGS = -O2
CXX = g++
CXX_FLAGS = --std=c++17 -Wall

INCLUDES = -I./include/
OPENCV_INCS = -I/usr/include/opencv2
OPENCV_LIBS = -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_videostab -lopencv_aruco -lopencv_bgsegm -lopencv_bioinspired -lopencv_ccalib -lopencv_datasets -lopencv_dpm -lopencv_face -lopencv_fuzzy -lopencv_line_descriptor -lopencv_optflow -lopencv_video -lopencv_plot -lopencv_reg -lopencv_saliency -lopencv_stereo -lopencv_structured_light -lopencv_phase_unwrapping -lopencv_rgbd -lopencv_surface_matching -lopencv_ximgproc -lopencv_calib3d -lopencv_features2d -lopencv_flann -lopencv_xobjdetect -lopencv_objdetect -lopencv_ml -lopencv_xphoto -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_photo -lopencv_imgproc -lopencv_core
TESSARACT_LIBS = -ltesseract

src_dir = ./src/
dst_dir = ./bin/
src = $(wildcard $(src_dir)*.cpp)
obj = $(src:.cpp=.o)
dep = $(src:.cpp=.d)

c_stripes = $(src_dir)generate_puzzle.o \
			$(src_dir)stripes_generator.o \
			$(src_dir)squares_generator.o
			
s_stripes = $(src_dir)solve_stripes.o \
			$(src_dir)stripes_solver.o \
			$(src_dir)stripe_pair.o \
			$(src_dir)fragment.o
s_stripes_debug = $(s_stripes:.o=.debug.o)

x = a b c
y = $(src_dir)$(x)

.PHONY: clean default debug all
default: solve-stripes generate-puzzle
debug: solve-stripes-debug
all: default debug

%.debug.o: %.cpp
	$(CXX) -c $< -o $@ -MMD $(CXX_FLAGS) $(INCLUDES) -DDEBUG -g
%.o: %.cpp
	$(CXX) -c $< -o $@ -MMD $(CXX_FLAGS) $(INCLUDES) $(OPT_FLAGS)

-include $(dep)

generate-puzzle: $(c_stripes)
	$(CXX) $^ -o $(dst_dir)$@ $(CXX_FLAGS) $(OPENCV_LIBS)

solve-stripes: $(s_stripes)
	$(CXX) $^ -o $(dst_dir)$@ $(CXX_FLAGS) $(TESSARACT_LIBS) $(OPENCV_LIBS)

solve-stripes-debug: $(s_stripes_debug)
	$(CXX) $^ -o $(dst_dir)$@ $(CXX_FLAGS) $(TESSARACT_LIBS) $(OPENCV_LIBS) 

clean:
	rm $(src_dir)*.o $(src_dir)*.d
	rm $(dst_dir)generate-puzzle $(dst_dir)solve-stripes $(dst_dir)solve-stripes-debug
	

