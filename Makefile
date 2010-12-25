OPS= -g -I /usr/local/include/opencv -L /usr/local/lib  -lm -lcv -lhighgui -lcvaux

all: goodfeatures opticalflow_roi optical_flow_global optical_flow_field movement_filter

goodfeatures: goodfeatures.cpp
	g++ goodfeatures.cpp -o goodfeatures $(OPS)
		     
opticalflow_roi: opticalflow_roi.cpp
	g++ opticalflow_roi.cpp -o opticalflow_roi $(OPS)
	
optical_flow_global: optical_flow_global.cpp
	g++ optical_flow_global.cpp -o optical_flow_global $(OPS)
	
optical_flow_field: optical_flow_field.cpp
	g++ optical_flow_field.cpp -o optical_flow_field $(OPS)

movement_filter: movement_filter.cpp
	g++ movement_filter.cpp -o movement_filter $(OPS)
