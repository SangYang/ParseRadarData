objects = main.o unsign_string.o debug_assert.o debug_log.o \
	section_lock.o parse_radar_data.o device_config.o radar_data_list.o \
	grid_coord.o com_port.o radar_data_interface.o

ParseRadarData : $(objects)
	gcc -o ParseRadarData $(objects) -lm -lpthread -g

main.o :
unsign_string.o : debug_assert.h debug_log.h
debug_assert.o :
debug_log.o :
section_lock.o : 
	gcc -c section_lock.c -lpthread
parse_radar_data.o : debug_assert.h debug_log.h unsign_string.h
device_config.o : debug_assert.h debug_log.h grid_coord.h
radar_data_list.o : debug_assert.h debug_log.h device_config.h section_lock.h parse_radar_data.h
grid_coord.o : debug_assert.h
com_port.o : debug_assert.h debug_log.h parse_radar_data.h radar_data_list.h unsign_string.h
radar_data_interface.o : debug_assert.h debug_log.h com_port.h device_config.h grid_coord.h \
	radar_data_list.h parse_radar_data.h

.PHONY : clean

clean :
	rm ParseRadarData $(objects)

