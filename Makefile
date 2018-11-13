OPT_FLAGS = -O2
CXX = g++
CXX_FLAGS = --std=c++17 -Wall

INCLUDES = -I./include/
OPENCV_INCS = -I/usr/include/opencv2
OPENCV_LIBS = -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_videostab -lopencv_aruco -lopencv_bgsegm -lopencv_bioinspired -lopencv_ccalib -lopencv_datasets -lopencv_dpm -lopencv_face -lopencv_fuzzy -lopencv_line_descriptor -lopencv_optflow -lopencv_video -lopencv_plot -lopencv_reg -lopencv_saliency -lopencv_stereo -lopencv_structured_light -lopencv_phase_unwrapping -lopencv_rgbd -lopencv_surface_matching -lopencv_ximgproc -lopencv_calib3d -lopencv_features2d -lopencv_flann -lopencv_xobjdetect -lopencv_objdetect -lopencv_ml -lopencv_xphoto -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_photo -lopencv_imgproc -lopencv_core
TESSARACT_LIBS = -ltesseract

src_dir = ./src/
src_generator_dir  = $(src_dir)generator/
src_solver_dir = $(src_dir)solver/
dst_dir = ./bin/

generator_src = 	$(wildcard $(src_generator_dir)*.cpp) $(src_dir)utils.cpp
generator_dep = 	$(generator_src:.cpp=.d)
generator_obj = 	$(generator_src:.cpp=.o)

solver_src = 		$(wildcard $(src_solver_dir)*.cpp) $(src_dir)utils.cpp
solver_dep = 		$(solver_src:.cpp=.d)
solver_obj =		$(solver_src:.cpp=.o)
solver_obj_debug = 	$(solver_src:.cpp=.debug.o)

add_noise_src = 	$(src_dir)add_noise.cpp
add_noise_obj = 	$(add_noise_src:.cpp=.o)

debug_tool_src =	$(src_dir)debug_tool.cpp $(src_dir)utils.cpp
debug_tool_obj =	$(debug_tool_src:.cpp=.o)

.PHONY: clean default debug all
default: solve-puzzle generate-puzzle
debug: solve-puzzle-debug
all: default debug add-noise debug-tool

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

debug-tool: $(debug_tool_obj)
	$(CXX) $^ -o $(dst_dir)$@ $(CXX_FLAGS) $(OPENCV_LIBS)

solve-puzzle: $(solver_obj)
	$(CXX) $^ -o $(dst_dir)$@ $(CXX_FLAGS) $(TESSARACT_LIBS) $(OPENCV_LIBS)

solve-puzzle-debug: $(solver_obj_debug)
	$(CXX) $^ -o $(dst_dir)$@ $(CXX_FLAGS) $(TESSARACT_LIBS) $(OPENCV_LIBS) 

clean:
	rm $(src_solver_dir)*.o $(src_solver_dir)*.d
	rm $(src_generator_dir)*.o $(src_generator_dir)*.d
	rm $(src_dir)*.o $(src_dir)*.d
	rm $(dst_dir)*	

