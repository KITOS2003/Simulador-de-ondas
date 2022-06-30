#ifndef PNG_ENCODER_H
#define PNG_ENCODER_H

#include<stdint.h>
#include"wave_solver.h"

#define HEIGHT 720
#define WIDTH 1280

typedef uint32_t COLOR;

typedef struct display_parameters{
	
	// array con la paleta de colores con la cual vamos a dibujar
	COLOR* color_palette;
	unsigned n_colors;
	
	// colores con los que se va a dibujar los contornos:
	COLOR boundary_colors[6];

	// escala de la pantalla en pixeles / metro
	double scale;
	
	double screen_origin[2];

	// mientras mas chico este parametro mas sensible es el programa a los cambios en la funcion a dibujar
	double tolerance;

	double min_threshold;
	
}DISPLAYPARAMS;


void CreatePNG( double (*func)( double, double ), unsigned n_frame,
		DISPLAYPARAMS params, const char* outdir );

COLOR palette( double input, DISPLAYPARAMS params );

COLOR SetColor( unsigned char red, unsigned char green, unsigned char blue, unsigned char opacity );
void SaveImage( COLOR* buffer, const char* out_name );

void CreateImageOutOfFunction( DISPLAYPARAMS params,
			       double (*func)( double, double ),
			       COLOR* image
			      );

unsigned set_boundary_at_t( double x, double y, double t_input, SIMULPARAMS* params_input );
unsigned boundary_at_t( double x, double y );

#endif
