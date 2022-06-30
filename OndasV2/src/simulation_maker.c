#include<stdio.h>
#include<stdlib.h>
#include<math.h>

#include"wave_solver.h"
#include"png_encoder.h"

// chequea que el input sea consistente
void check_input( SIMULPARAMS sim_params, DISPLAYPARAMS disp_params ){

}

// ajusta los parametros interdependientes acordemente
void adjust_parameters( SIMULPARAMS* sim_params, DISPLAYPARAMS* disp_params, double fps, unsigned aprox_per_frame ){

	double aux = sim_params -> domain_x[1] - sim_params -> domain_x[0];
	sim_params -> delta_x = aux / sim_params -> n_elements_x;

	aux = sim_params -> domain_y[1] - sim_params -> domain_y[0];
	sim_params -> delta_y = aux / sim_params -> n_elements_y;

	sim_params -> delta_t = 1 / (fps*aprox_per_frame);
}

// imprime en la consola informacion sobre el progreso de la creacion de la simulacion
void ConsoleDisplay( unsigned total_frames , unsigned generated_frames){
	
	unsigned porcentage = floor( 100*generated_frames/total_frames );

	system("clear");
	printf("--------------------------------------------CREANDO ANIMACION--------------------------------------------\n\nprogreso:\n[");
	unsigned i = 0;
	for(i = 0; i < porcentage; i++ ){
	
		printf("#");
	}
	for(;i < 100; i++ ){
		
		printf(" ");
	}
	printf(" %d%\n\n", porcentage);
	printf("%d frames generados de un total de %d\n", generated_frames, total_frames );


}

// crea todos los frames de la simulacion
void create_simulation( SIMULPARAMS sim_params, DISPLAYPARAMS disp_params, double fps, unsigned aprox_per_frame ){

	check_input( sim_params, disp_params );
	adjust_parameters( &sim_params, &disp_params, fps, aprox_per_frame );
	
	double simulation_time = sim_params.domain_t[1] - sim_params.domain_t[0];
	unsigned total_frames = simulation_time*fps;	

	// creamos nuestro espacio discretizado	
	double** PrevAprox = CreateGrid( sim_params );
	double** NextAprox = CreateGrid( sim_params );
	
	// a continuacion creamos la simulacion
	for( unsigned frame = 0; frame < total_frames; frame++ ){
		
		double t = sim_params.domain_t[0] + frame/fps;
		
		// configuramos los parametros internos de Evaluate y boundary_at_t
		Evaluate( 0, 0, sim_params, PrevAprox );
		set_boundary_at_t( 0, 0, t, &sim_params );
		
		// creamos un frame
		CreatePNG( evaluate_wrapper, frame, disp_params, "output" );
		
		// escribimos el progreso a la consola
		ConsoleDisplay( total_frames, frame+1 );
		
		// aproximamos el proximo frame
		for( unsigned i = 0; i < aprox_per_frame; i++){	
			
			t = sim_params.domain_t[0] + (frame + i/aprox_per_frame)/fps;
			
			RungeKutta4thOrder( sim_params, PrevAprox, t, NextAprox );
			
			// intercambiar PrevAprox y NewAprox
			double** aux = PrevAprox;
			PrevAprox = NextAprox;
			NextAprox = aux;
		}
	}
}
