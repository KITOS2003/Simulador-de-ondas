#include<stdio.h>
#include<stdlib.h>
#include<math.h>

#include"png_encoder.h"
#include"wave_solver.h"
#include"simulation_maker.h"

double ci_onda( double x, double y ){
	
	double dy2 = (y-2.8)*(y-2.8);
	if( dy2 < 0.3*0.3 ){
	
		return 4*exp(-200*dy2);
	}
	return 0;
}

double zero_f( double x, double y ){ return 0; }

unsigned no_b( double x, double y, double t ){ 
	return 0; 
}

double zero_b( double x, double y, double t, unsigned type ){ return 0; }

double zero_t( double z, double y, double t ){ return 0; }

double speed_of_waves( double x, double y, double t ){
	
	double dx2 = x*x;
	double dy2 = (y-1)*(y-1);
	
	if( dx2 + dy2 < 0.5*0.5 ){
		
		return 0.5;
	}

	return 1;
}


int main(){
	
	
	SIMULPARAMS sim_params = {	

		.domain_x = { -2, 2 },
		.domain_y = { -2, 3 },
		.domain_t = { 0, 10 },
		.n_elements_x = 300,
		.n_elements_y = 300,
		.bc_type = { { ABSORBING, ABSORBING },
		             { ABSORBING, ABSORBING }
			   },
		.initial_condition_displacement = &ci_onda,
		.initial_condition_velocity = &zero_f,
		.boundary = &no_b,
		.boundary_value = &zero_b,
		.forcing_term = &zero_t,

		.C = &speed_of_waves

	};

	DISPLAYPARAMS disp_params = { .screen_origin = { -2, -2 }, 
				      .scale = 170, 
				      .tolerance = 1.5,
				      .n_colors = 5,
				      .min_threshold = 0
	};
	
	disp_params.color_palette = malloc( 2*sizeof(COLOR) );
	disp_params.color_palette[0] = SetColor( 0, 0, 255, 255  );
	disp_params.color_palette[1] = SetColor( 64, 224, 208, 255);
	disp_params.color_palette[2] = SetColor( 0, 0, 0  , 255    );	
	disp_params.color_palette[3] = SetColor( 255, 165, 0, 255  );
	disp_params.color_palette[4] = SetColor( 255, 0, 0, 255  );
	
	disp_params.boundary_colors[DIRICHLET] = SetColor( 255, 255, 255, 255 );

	const unsigned fps = 30;
	const unsigned aprox_per_frame = 10;

	create_simulation( sim_params, disp_params, fps, aprox_per_frame );
}
