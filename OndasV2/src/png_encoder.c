#include<stdio.h>
#include<stdlib.h>
#include<math.h>

#include"lodepng.h"
#include"png_encoder.h"
#include"wave_solver.h"

// Esta funcion retorna un valor de 4 bytes donde el 1ro representa el rojo, el 2do el verde, el 3ro el azul, y el 4to la opacidad
COLOR SetColor( unsigned char red, unsigned char green, unsigned char blue, unsigned char opacity ){

	COLOR result = 0;
	char* magic = (char*)(&result);

	magic[0] = red;
	magic[1] = green;
	magic[2] = blue;
	magic[3] = opacity;

	return result; 
}

// Esta funcion es un wrapper de la funcion de libreria lodepng_encode32_file, genera un archivo PNG dados los colores de cada pixel
// en caso de fallar presenta un codigo de error en la consola
void SaveImage( COLOR* buffer, const char* out_name ){

	unsigned error = lodepng_encode32_file( out_name, (unsigned char*)buffer, WIDTH, HEIGHT );
	if( error ){
		printf("Err %u: %s", error, lodepng_error_text(error) );
		exit(1);
	}
}

// esta funcion crea un array de colores(imagen) a partir de una funcion
void CreateImageOutOfFunction( DISPLAYPARAMS params,
			       double (*func)( double, double ),
			       COLOR* image
			      ){
	
	// hacemos un bucle sobre todos los pixeles de la pantalla
	for( int i = 0; i < WIDTH; i++ ){
		
		for( int j = 0; j < HEIGHT; j++ ){
			
			// calculamos las coordenadas del pixel y su indice en el array
			double y = i/params.scale + params.screen_origin[1];
			double x = j/params.scale + params.screen_origin[0];
			
			int index = j * WIDTH + i;
			
			// pintamos el pixel
			image[index] = palette( func( x, y ), params );
			
			// si el punto esta sujeto a condiciones de contorno, hay que representarlo
			unsigned bc_type = boundary_at_t( x, y );

			if( bc_type ){
				image[index] = params.boundary_colors[bc_type];
			}
		}
	}

}


// funcion que crea uno de los frames de la animacion
void CreatePNG( double (*func)( double, double ), unsigned n_frame,
		DISPLAYPARAMS params, const char* outdir
		){
	
	// agarramos memoria para la imagen
	COLOR* image = NULL;
	if( image == NULL ){
		
		image = malloc( HEIGHT * WIDTH * sizeof(COLOR) );
	}
	// hacemos magia para convertir el valor de n_frame en caracteres ASCII
	char frame_id[20] = "";
	snprintf(frame_id, sizeof(frame_id), "%05d", n_frame );

	// armamos el nombre de la PNG resultante
	char filename[100] = "";
	strcat( filename, outdir );
	strcat( filename, "/frame_" );
	strcat( filename, frame_id );
	strcat( filename, ".png" );
	
	// llamamos a las otras funciones para crear la imagen y generar el archivo PNG
	CreateImageOutOfFunction( params, func, image );
	
	SaveImage( image, filename );
	
}

// decide que color pintar de acuerdo con un imput
// toma dos colores de la paleta de colores y hace una interpolacion lineal entre ellos
COLOR palette( double input, DISPLAYPARAMS params ){
	
	if( input < params.min_threshold ){
		
		return SetColor( 0, 0, 0, 255 );	
	}	

	// chequeamos si el imput supera la tolerancia
	if( input >= params.tolerance ){
		
		return params.color_palette[ params.n_colors - 1 ];
	}
	if( input <= -params.tolerance ){
		
		return params.color_palette[ 0 ];
	}
	
	// calculamos que porcion del dominio de input corresponde a cada color
	input += params.tolerance;
	double color_subdomain_size = 2*params.tolerance / (params.n_colors-1);
	
	// calculamos a que subdominio pertenece input y en base a eso entre que colores vamos a interpolar
	unsigned base_index = 1;
	for(; base_index * color_subdomain_size < input; base_index++ );
	base_index--;

	COLOR base = params.color_palette[ base_index ];
	COLOR high = params.color_palette[ base_index + 1 ];
	
	// ahora interpolamos linealmente cada valor RGB
	double color_factor = input/color_subdomain_size - base_index;

	unsigned char* base_rgb = (unsigned char*)(&base);
	unsigned char* high_rgb = (unsigned char*)(&high);

	unsigned char result_r = color_factor * high_rgb[0] + (1-color_factor) * base_rgb[0];
	unsigned char result_g = color_factor * high_rgb[1] + (1-color_factor) * base_rgb[1];
	unsigned char result_b = color_factor * high_rgb[2] + (1-color_factor) * base_rgb[2];
	
	return SetColor( result_r, result_g, result_b, 255 );
}

// wrapper de contorno(ver wave_solver.h) para eliminar la dependencia de los parametros de simulacion y el tiempo
unsigned set_boundary_at_t( double x, double y, double t_input, SIMULPARAMS* params_input ){
	
	static SIMULPARAMS params;
	static double t = 0;

	if( params_input == NULL ){
		
		return params.boundary( x, y, t );

	}else{
	
		params = *params_input;
		t = t_input;
		return 0;
	}
}

// wrapper de la funcion de arriba
unsigned boundary_at_t( double x, double y ){
	return set_boundary_at_t( x, y, 0, NULL );
}

