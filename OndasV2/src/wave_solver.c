#include<stdio.h>
#include<stdlib.h>
#include<math.h>

#include"wave_solver.h"

double Square( double x ){ return x*x; }

// suma dos matrices, el resultado se escribe en la direccion dada por out
void AddByComponents( double** a, double** b, int dim_x, int dim_y, double** out ){
		
	for( int i = 0; i < dim_y; i++ ){
		for( int j = 0; j < dim_x; j++ ){
			
			out[i][j] = a[i][j] + b[i][j];
		}
	}
}

// multiplica una matriz por un escalar, el resultado se escribe en la direccion dada por out
void ScalarMult( double x, double** a, int dim_x, int dim_y, double** out ){

	for( int i = 0; i < dim_y; i++ ){
		for( int j = 0; j < dim_x; j++ ){
			
			out[i][j] = x*a[i][j];
		}
	}

}

// crea una matriz de decimales de doble precision que va a representar nuestro espacio discretizado
// y le asigna los valores correspondientes a las condiciones iniciales
double** CreateGrid( SIMULPARAMS params ){
	
	// agarramos la memoria necesaria, notar que ponemos un factor de 2 en la parte de x
	// esto es porque para cada punto tenemos que saber desplazamiento y velocidades,
	// las velocidades de un punto se almacenan en p[y][ x + numero de elementos en x ] por convencion
	double** simulation = malloc( (params.n_elements_y + 1)* sizeof(double) );
	for( unsigned i = 0; i < params.n_elements_y ; i++ ){
		
		simulation[i] = malloc( 2*params.n_elements_x * sizeof(double) );
	}
	
	// asignamos a cada punto condiciones iniciales
	for( unsigned i = 0; i < params.n_elements_y; i++ ){

		for( unsigned j = 0; j < params.n_elements_x; j++ ){
			
			// calculamos coordenadas del punto correspondiente a los indices i,j
			double x = params.domain_x[0] + j*params.delta_x;
			double y = params.domain_y[0] + i*params.delta_y;

			// a cada punto le asignamos desplazamiento y velocidades como corresponde
			simulation[i][j] = params.initial_condition_displacement( x, y );			
			simulation[i][j + params.n_elements_x ] = params.initial_condition_velocity( x, y );			
		}
	}
	return simulation;
}




// wrapper de la funcion contorno que impone las condiciones de contorno en los bordes de la simulacion
// si el elemento ( ix, iy ) no se encuentra en el borde pasa el control a la funcion definida por el usuario
unsigned boundary_wrapper( unsigned ix, unsigned iy, double t, SIMULPARAMS params ){
			
	double x = params.domain_x[0] + ix*params.delta_x;
	double y = params.domain_y[0] + iy*params.delta_y;
	
	// vemos si el elemento ( ix, iy ) pertenece al contorno
	if( ix == 0 ){
		
		return params.bc_type[0][0];

	}else if( ix == params.n_elements_x - 1 ){
	
		return params.bc_type[0][1];
	}

	if( iy == 0 ){
		
		return params.bc_type[1][0];
		
	}else if(iy == params.n_elements_y - 1 ){
	
		return params.bc_type[1][1];
	}	
	
	// no -> dejamos la decision al usuario
	return params.boundary( x, y, t ); 
}


// dados valores para la funcion de onda y su derivada temporal (in) en un tiempo t
// calcula la derivada temporal de las mismas ( es decir, reemplaza el valor de la funcion de onda por el de su derivada temporal
// y calcula la derivada segunda de acuerdo con la ecuacion de onda )
// ademas, adquiere distintos comportamientos en el contorno de acuerdo con las condiciones de contorno dadas
void FWave( double** in, double t, SIMULPARAMS params, double** result ){	
	
	double partial2_x = 0;
	double partial2_y = 0;
	double normal_derivative = 0;
	double x = 0;
	double y = 0;

	unsigned consecutive_i;
	unsigned consecutive_j;
	unsigned anterior_i;
	unsigned anterior_j;
	
	// hacemos un loop sobre todos los elementos del input
	for( unsigned i = 0; i < params.n_elements_y; i++ ){
			
		for( unsigned j = 0; j < params.n_elements_x; j++ ){
			
			// pedimos el tipo de condicion de contorno al que el elemento i,j esta sujeto
			unsigned bc_type = boundary_wrapper( j, i, t, params );

			x = params.domain_x[0] + j*params.delta_x;
			y = params.domain_y[0] + i*params.delta_y;

			switch(bc_type){	
				
				case NO_BOUNDARY:		// (x,y) no esta en el contorno
					
					// para calcular las derivadas parciales usamos el hecho de que nuestro espacio esta discretizado
					// usamos la definicion de la derivada solo que en vez de un limite usamos nuestros parametros de 
					// discretizacion	
					partial2_x = ( in[i][j+1] - 2*in[i][j] + in[i][j-1] ) / ( params.delta_x*params.delta_x );
					partial2_y = ( in[i+1][j] - 2*in[i][j] + in[i-1][j] ) / ( params.delta_y*params.delta_y );
				
					result[i][j] = in[i][j + params.n_elements_x];	

					result[i][ j + params.n_elements_x ] 
						= 
					Square( params.C( x, y, t ) ) * ( partial2_x + partial2_y - params.forcing_term( x, y, t ) );
					break;	
				
				case DIRICHLET:		// (x,y) esta en el contorno, cond de dirichlet
					
					// con las condiciones de dirichlet la funcion de onda o bien no cambia o cambia pero no corresponde
					// a esta funcion modificar la funcion de onda explicitamente, por eso hacemos todas las derivadas 0
					result[i][j] = 0;
					result[i][ j + params.n_elements_x ] = 0;
					break;

				case NEUMANN:		// (x,y) esta en el contorno, cond de neumann

				case ABSORBING:		// (x,y) esta en el contorno, cond absorbentes
					
					// con las condiciones absorbentes (primer orden) se debe cumplir Dt(f) = C*Dn(f)
					if( i == 0 ){
						normal_derivative = (in[i+1][j] - in[i][j])/params.delta_y;
					}else if( i == params.n_elements_y - 1 ){	
						normal_derivative = (in[i-1][j] - in[i][j])/params.delta_y;					
					}	
					if( j == 0 ){	
						normal_derivative = (in[i][j+1] - in[i][j])/params.delta_x;
					}else if( j == params.n_elements_x - 1 ){	
						normal_derivative = (in[i][j-1] - in[i][j])/params.delta_x;
					}
					result[i][j] = params.C( x, y, t )*normal_derivative;
					break;
				
				case PERIODIC:			// (x,y) esta en el contorno, cond periodicas
			
					consecutive_i = i+1;
					consecutive_j = j+1;
					anterior_i = i-1;
					anterior_j = j-1;

					if( i == 0 ){
						anterior_i = params.n_elements_y - 1;
					}else if( i == params.n_elements_y-1 ){
						consecutive_i = 0;
					}
					if( j == 0 ){
						anterior_j = params.n_elements_x - 1;
					}else if( j == params.n_elements_x-1 ){
						consecutive_j = 0;
					}

					partial2_x = ( in[i][consecutive_j] - 2*in[i][j] + in[i][anterior_j] ) / ( params.delta_x*params.delta_x );
					partial2_y = ( in[consecutive_i][j] - 2*in[i][j] + in[anterior_i][j] ) / ( params.delta_y*params.delta_y );
				
					result[i][j] = in[i][j + params.n_elements_x];	

					result[i][ j + params.n_elements_x ] 
						= 
					Square( params.C( x, y, t ) ) * ( partial2_x + partial2_y - params.forcing_term( x, y, t ) );

				case MEDIUM_CHANGE:		// (x,y) esta en el contorno, cambio de medio
					break;
			}		

		}
	}

	return;
}

// conseguir una nueva aproximacion a partir de una previa usando un metodo de resolucion numerica de EDOs
// en este caso el metodo Runge Kutta de 4to orden: https://lpsa.swarthmore.edu/NumInt/NumIntFourth.html
// el resultado se escribe en la direccion indicada por out
void RungeKutta4thOrder( SIMULPARAMS params, double** PrevAprox, double PrevT, double** out ){

	static double** k1 = NULL;
	static double** k2;
	static double** k3;
	static double** k4;
	static double** aux;
	// la primera vez que la funcion se ejecuta nos encargamos de agarrar la memoria que va a necesitar por
	// el resto del programa
	if( !k1 ){

		k1 = malloc( params.n_elements_y * sizeof(double*) );
		k2 = malloc( params.n_elements_y * sizeof(double*) );
		k3 = malloc( params.n_elements_y * sizeof(double*) );
		k4 = malloc( params.n_elements_y * sizeof(double*) );
		aux = malloc( params.n_elements_y * sizeof(double*) );

		for( unsigned i = 0; i < params.n_elements_y; i++ ){

			k1[i] = malloc( 2 * params.n_elements_x * sizeof(double) );
			k2[i] = malloc( 2 * params.n_elements_x * sizeof(double) );
			k3[i] = malloc( 2 * params.n_elements_x * sizeof(double) );
			k4[i] = malloc( 2 * params.n_elements_x * sizeof(double) );
			aux[i] = malloc( 2 * params.n_elements_x * sizeof(double) );
		}
	}
	// a continuacion unas cuantas operaciones vectoriales bastante obfuscadas

	// k1 = F( PrevAprox, t )
	FWave(PrevAprox, PrevT, params, k1);
	
	// k2 = F( PrevAprox + DeltaT*k1/2 , t + DeltaT/2 )
	ScalarMult( params.delta_t/2, k1, 2*params.n_elements_x, params.n_elements_y, aux );
	AddByComponents( PrevAprox, aux, 2*params.n_elements_x, params.n_elements_y, aux );

	FWave( aux, PrevT + params.delta_t/2, params, k2 );
	
	// k3 = F( PrevAprox + DeltaT*k2/2 , t + DeltaT/2 )
	ScalarMult( params.delta_t/2, k2, 2*params.n_elements_x, params.n_elements_y, aux );
	AddByComponents( PrevAprox, aux, 2*params.n_elements_x, params.n_elements_y, aux );

	FWave( aux, PrevT + params.delta_t/2, params, k3 );
	
	// k4 = F( PrevAprox + DeltaT*k3, t + DeltaT )
	ScalarMult( params.delta_t, k3, 2*params.n_elements_x, params.n_elements_y, aux );
	AddByComponents( PrevAprox, aux, 2*params.n_elements_x, params.n_elements_y, aux );

	FWave( aux, PrevT + params.delta_t, params, k4 );
	
	// ahora hay que hacer m = (k1 + 2*k2 + 2*k3 + k4)/6
	// donde m sera la pendiente de una "recta tangente mejorada" sobre el punto ( t, PrevAprox )
	// luego la aproximacion para t+DeltaT la podemos conseguir como PrevAprox + m*DeltaT
	ScalarMult( 2, k2, 2*params.n_elements_x, params.n_elements_y, k2 );
	ScalarMult( 2, k3, 2*params.n_elements_x, params.n_elements_y, k3 );

	AddByComponents( k1, k2, 2*params.n_elements_x, params.n_elements_y, k1 );
	AddByComponents( k3, k4, 2*params.n_elements_x, params.n_elements_y, k2 );
	AddByComponents( k1, k2, 2*params.n_elements_x, params.n_elements_y, k1 );
	
	// ahora que la variable k1 contiene a nuestra m hacemos out = PrevAprox + k1*DeltaT
	ScalarMult( params.delta_t/6, k1, 2*params.n_elements_x, params.n_elements_y, k1 );
	AddByComponents( k1, PrevAprox, 2*params.n_elements_x, params.n_elements_y, out );

	// aqui abajo hay que añiadir logica relacionada con las condiciones de contorno, necesario por ejemplo para condiciones
	// de dirichlet dependientes del tiempo.
}

// hace una interpolacion lineal de simulacion para obtener una aproximacion en el punto (x,y)
// solo lo hace cuando params_input es NULL, si no es NULL entonces en vez de hacer la interpolacion lineal
// los parametros internos de la funcion son modificados
double Evaluate( double x, double y, SIMULPARAMS params_input, double** simulation_input ){
	
	static SIMULPARAMS params;
	static double** simulation = NULL;
	// chequeamos si params_input es null sino modificamos los parametros
	if( simulation_input != NULL ){
		
		params = params_input;
		simulation = simulation_input;
		return 0;
	}
	
	// sino, hay que hacer la interpolacion lineal
	// calculamos los indices de el punto perteneciente al grid de la simulacion tal que ambas coordenadas ( x, y ) del punto
	// sean las mayores menores a las coordendas del punto a interpolar
	unsigned ix = floor( ( x - params.domain_x[0] ) / params.delta_x ) ;
	unsigned iy = floor( ( y - params.domain_y[0] ) / params.delta_y );
	

	// por seguridad, nos aseguramos que el punto a interpolar este dentro del dominio de la simulacion
	if( ix > params.n_elements_x-2 ){
		return 0;
	}
	if( iy > params.n_elements_y-2 ){
		return 0;
	}
	
	// coordenadas del punto p tal que ambas coordenadas ( x, y ) son las mayores menores a las coordenadas del punto a interpolar
	double p[2] = { params.domain_x[0] + ix*params.delta_x, params.domain_y[0] + iy*params.delta_y };
	
	// calculamos las pendientes en x y en y de la aproximacion del plano tangente de la funcion
	double mx = (simulation[iy][ix+1] - simulation[iy][ix]) / params.delta_x;
	double my = (simulation[iy+1][ix] - simulation[iy][ix]) / params.delta_y;

	// retornamos la interpolacion lineal
	return mx*( x - p[0] ) + my*( y - p[1] ) + simulation[iy][ix];
}

// wrapper de evaluate para cuando solo se quiere hacer la interpolacion y no se quieren cambiar
// los parametros internos
double evaluate_wrapper( double x, double y ){
	SIMULPARAMS nothing;
	return Evaluate( x, y, nothing, NULL );
}
