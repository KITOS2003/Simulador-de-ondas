#ifndef WAVE_SOLVER_H
#define WAVE_SOLVER_H

#define NO_BOUNDARY 0
#define DIRICHLET 1
#define NEUMANN 2
#define ABSORBING 3
#define PERIODIC 4
#define MEDIUM_CHANGE 5

typedef struct simulation_parameters{
	
	// dominio de la simulacion a determinar por el usuario
	double domain_x[2];
	double domain_y[2];
	double domain_t[2];

	// numero de discretizaciones espaciales y temporales, a determinar por el usuario
	unsigned n_elements_x;
	unsigned n_elements_y;
	
	// intervalos temporales asociados a la discretizacion, dependen explicitamente de los parametros de arriba
	double delta_x;
	double delta_y;
	double delta_t;
	
	// tipo de condiciones de contorno sobre los bordes de la simulacion, el primer indice es 0 si se trata de los extremos del eje x
	// o 1 si se trata de los extremos de y, el segundo indice es 0 si es el extremo en la direccion negativa y 1 si es el extremo en la
	// direccion positiva
	unsigned bc_type[2][2];
	
	// condiciones iniciales, desplazamiento y velocidades, a determinar por el usuario
	double (*initial_condition_displacement)( double, double );
	double (*initial_condition_velocity)( double, double );

	// funcion determinada por el usuario que determina si el punto x, y esta sujeto a condiciones de contorno en el tiempo t
	// debe retornar el tipo de condiciones de contorno ( dirichlet, neumann, etc. )
	unsigned (*boundary)( double x, double y, double t );

	// funcion determinada por el usuario que dado un punto sujeto a condiciones de contorno 
	// y el tipo de condiciones de contorno retorna un valor asociado al comportamiento de la 
	// funcion en el punto ( valor de la funcion en dirichlet, valor de la derivada normal en neumann, etc. )
	double (*boundary_value)( double x, double y, double t, unsigned type );
	
	// termino forzante en la ecuacion de onda, a determinar por el usuario.
	double (*forcing_term)( double x, double y, double t );
	
	// velocidad de las ondas. A determinar por el usuario, es una funcion del espacio y el tiempo ya que se admiten cambios de medio
	double (*C)( double x, double y, double t );

}SIMULPARAMS;


double Square( double x );
void AddByComponents( double** a, double** b, int dim_x, int dim_y, double** out );
void ScalarMult( double x, double** a, int dim_x, int dim_y, double** out );
double** CreateGrid( SIMULPARAMS params );
unsigned boundary_wrapper( unsigned ix, unsigned iy, double t, SIMULPARAMS params );
void FWave( double** in, double t, SIMULPARAMS params, double** result );
void RungeKutta4thOrder( SIMULPARAMS params, double** PrevAprox, double PrevT, double** out );
double Evaluate( double x, double y, SIMULPARAMS params_input, double** simulation_input );
double evaluate_wrapper( double x, double y );

#endif
