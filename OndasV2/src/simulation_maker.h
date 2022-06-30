#ifndef SIMULATION_MAKER_H
#define SIMULATION_MAKER_H

#include"wave_solver.h"
#include"png_encoder.h"

void check_input( SIMULPARAMS sim_params, DISPLAYPARAMS disp_params );
void adjust_parameters( SIMULPARAMS* sim_params, DISPLAYPARAMS* disp_params, double fps, unsigned aprox_per_frame );
void console_display( unsigned total_frames , unsigned generated_frames);
void create_simulation( SIMULPARAMS sim_params, DISPLAYPARAMS disp_params, double fps, unsigned aprox_per_frame );

#endif
