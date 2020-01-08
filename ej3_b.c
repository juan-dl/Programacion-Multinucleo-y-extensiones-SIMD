//---------------BIBLIOTECAS UTILIZADAS---------------

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pmmintrin.h>

//---------------CONSTANTES---------------

#define LIMITE_SUP 1000000
#define LIMITE_INF -1000000

//---------------FUNCIONES EMPLEADAS---------------

//Programada por el equipo
void representar_quaternion(float* quaternion);

//Funciones externas de medida de ciclos
void access_counter(unsigned *hi, unsigned *lo);
void start_counter();
double get_counter();

//Variables globales para la medida de ciclos
static unsigned cyc_hi = 0;
static unsigned cyc_lo = 0;

//---------------FUNCION PRINCIPAL---------------

int main(int argc, char** argv)
{
	FILE *fichero;
	int i, total, semilla = getpid();
	double medidaCiclos;
	float a1[4] __attribute__((aligned(16))), 
				a2[4] __attribute__((aligned(16))),
				a3[4] __attribute__((aligned(16))),
				a4[4] __attribute__((aligned(16))),
				b1[4] __attribute__((aligned(16))), 
				b2[4] __attribute__((aligned(16))),
				b3[4] __attribute__((aligned(16))),
				b4[4] __attribute__((aligned(16))), 
				x[4] __attribute__((aligned(16)));

	//Se comprueba si el numero de argumentos es valido se exige como minimo el numero de cuaterniones del computo
	if(argc >= 2 && argc <= 3)
	{
		//Se comprueba si el numero de cuaterniones es mayor que 0
		if((total = atoi(argv[1])) <= 0)
	  {
	    printf("El total de operaciones debe ser mayor que 0\n");
	    return 1;
	  }

		//Se comprueba si la semilla de numeros aleatorios (opcional) es mayor que 0
    if(argc == 3 && (semilla = atoi(argv[2])) < 0)
    {
	    printf("La semilla debe ser positiva\n");
	    return 1;
	  }
	}
	else
  {
    printf("El numero de argumentos pasado no es valido\n");
    return 1;
  }

	//Se establece la semilla, si no se indica por terminal se establece el PID del proceso como semilla
  srand48(semilla);
  
	__m128 Cons0, Cons2, ResFinal;
	__m128 *A1,*A2,*A3,*A4,*B1,*B2,*B3,*B4, *multiplicacion, *Resultado;

	//Se reserva memoria dinamicamente para los vectores de cuaterniones A1,A2,A3,A4,B1,B2,B3,B4, suma y multiplicacion
	A1 = (__m128*)_mm_malloc((total/4)*sizeof(__m128), 16);
	B1 = (__m128*)_mm_malloc((total/4)*sizeof(__m128), 16);
	A2 = (__m128*)_mm_malloc((total/4)*sizeof(__m128), 16);
	B2 = (__m128*)_mm_malloc((total/4)*sizeof(__m128), 16);
	A3 = (__m128*)_mm_malloc((total/4)*sizeof(__m128), 16);
	B3 = (__m128*)_mm_malloc((total/4)*sizeof(__m128), 16);
	A4 = (__m128*)_mm_malloc((total/4)*sizeof(__m128), 16);
	B4 = (__m128*)_mm_malloc((total/4)*sizeof(__m128), 16);
	multiplicacion = (__m128*)_mm_malloc(4*sizeof(__m128), 16);
	Resultado = (__m128*)_mm_malloc(4*sizeof(__m128), 16);

  for(i = 0; i < total/4; i++) //Se llenan los float correspondientes con cada uno de los valores aleatorios necesario, se emplea este orden para que los resultados con el resto de versión sea el mismo
	{
    *(a1 + 0) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		*(b1 + 0) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		*(a2 + 0) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		*(b2 + 0) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		*(a3 + 0) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		*(b3 + 0) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
    *(a4 + 0) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		*(b4 + 0) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		
		*(a1 + 1) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		*(b1 + 1) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		*(a2 + 1) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		*(b2 + 1) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		*(a3 + 1) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		*(b3 + 1) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
    *(a4 + 1) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		*(b4 + 1) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		
		*(a1 + 2) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		*(b1 + 2) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		*(a2 + 2) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		*(b2 + 2) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		*(a3 + 2) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		*(b3 + 2) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
    *(a4 + 2) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		*(b4 + 2) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		
		*(a1 + 3) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		*(b1 + 3) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		*(a2 + 3) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		*(b2 + 3) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		*(a3 + 3) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		*(b3 + 3) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
    *(a4 + 3) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		*(b4 + 3) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;

		*(A1 + i) = _mm_load_ps(&a1[0]);
		*(B1 + i) = _mm_load_ps(&b1[0]); //Se carga cada uno de los registros correspondientes a cada una de las componentes de los cuaterniones
		*(A2 + i) = _mm_load_ps(&a2[0]);
		*(B2 + i) = _mm_load_ps(&b2[0]);
		*(A3 + i) = _mm_load_ps(&a3[0]);
		*(B3 + i) = _mm_load_ps(&b3[0]);
		*(A4 + i) = _mm_load_ps(&a4[0]);
		*(B4 + i) = _mm_load_ps(&b4[0]);
  }

	Cons0 = _mm_set1_ps(0.0);
	Cons2 = _mm_set1_ps(2.0); //Se inicializan una serie de constantes necesarias para el calculo

	//Se inicia el cronometro de ciclos
	start_counter();
	
	Resultado[0] = _mm_setzero_ps();
	Resultado[1] = _mm_setzero_ps();
	Resultado[2] = _mm_setzero_ps(); //Se inicializa a 0 el registro Resultado
	Resultado[3] = _mm_setzero_ps();

	for(i = 0; i < total/4; i++)
  {
  
  	//Combinando sub, add y mul realizamos las operaciones componente por componente de los cuaterniones, de forma que  obtenemos las componentes resultantesde la multiplicacion
		multiplicacion[0] = _mm_sub_ps(_mm_sub_ps(_mm_sub_ps(_mm_mul_ps(A1[i],B1[i]),_mm_mul_ps(A2[i],B2[i])),_mm_mul_ps(A3[i],B3[i])),_mm_mul_ps(A4[i],B4[i]));
		
		multiplicacion[1] = _mm_sub_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(A1[i],B2[i]),_mm_mul_ps(A2[i],B1[i])),_mm_mul_ps(A3[i],B4[i])),_mm_mul_ps(A4[i],B3[i]));

		multiplicacion[2] = _mm_add_ps(_mm_add_ps(_mm_sub_ps(_mm_mul_ps(A1[i],B3[i]),_mm_mul_ps(A2[i],B4[i])),_mm_mul_ps(A3[i],B1[i])),_mm_mul_ps(A4[i],B2[i]));
		
		multiplicacion[3] = _mm_add_ps(_mm_sub_ps(_mm_add_ps(_mm_mul_ps(A1[i],B4[i]),_mm_mul_ps(A2[i],B3[i])),_mm_mul_ps(A3[i],B2[i])),_mm_mul_ps(A4[i],B1[i]));

		//Posteriormente realizamos el mismo proceso(combinando sub,add,mul) para calcular el cuadrado de la multiplicacion y sumarlo a cada componenente del resultado
		Resultado[0] = _mm_add_ps(Resultado[0],_mm_sub_ps(_mm_sub_ps(_mm_sub_ps(_mm_mul_ps(multiplicacion[0],multiplicacion[0]),_mm_mul_ps(multiplicacion[1],multiplicacion[1])),
				_mm_mul_ps(multiplicacion[2],multiplicacion[2])),_mm_mul_ps(multiplicacion[3],multiplicacion[3])));

		Resultado[1] = _mm_add_ps(Resultado[1],_mm_mul_ps(multiplicacion[0],multiplicacion[1]));

		Resultado[2] = _mm_add_ps(Resultado[2],_mm_mul_ps(multiplicacion[0],multiplicacion[2]));

		Resultado[3] = _mm_add_ps(Resultado[3],_mm_mul_ps(multiplicacion[0],multiplicacion[3]));	
  }
  
  //Multiplicamos las tres ultimas componentes del resultado por 2
  Resultado[1] = _mm_mul_ps(Resultado[1],Cons2);

	Resultado[2] = _mm_mul_ps(Resultado[2],Cons2);

	Resultado[3] = _mm_mul_ps(Resultado[3],Cons2);
	
	
	//Para finalizar, acumulamos todas las componentes del resultado en una y realizando tres shuffles reordenamos todo para obtener el resultado final
	Resultado[0] = _mm_hadd_ps(_mm_hadd_ps(Resultado[0],Cons0),Cons0);

	Resultado[1] = _mm_hadd_ps(_mm_hadd_ps(Resultado[1],Cons0),Cons0);

	Resultado[2] = _mm_hadd_ps(_mm_hadd_ps(Resultado[2],Cons0),Cons0);

	Resultado[3] = _mm_hadd_ps(_mm_hadd_ps(Resultado[3],Cons0),Cons0);

	ResFinal = _mm_shuffle_ps(_mm_shuffle_ps(Resultado[1],Resultado[0],_MM_SHUFFLE(0,0,0,0)),_mm_shuffle_ps(Resultado[3],Resultado[2],_MM_SHUFFLE(0,0,0,0)),_MM_SHUFFLE(0,2,0,2));

	//Se para el cronometro de ciclos
	medidaCiclos = get_counter();

	_mm_store_ps(&x[0], ResFinal);

	//Se muestra el resultado del sumatorio para ver si coincide con las otras versiones
  printf("Resultado: ");
  representar_quaternion(x);


	//Se abre el fichero en modo append
	if((fichero = fopen("registro3B.txt", "a")) == NULL)
  {
    printf("Hubo un error al abrir el archivo en modo append\n");
    return 1;
  }

	//Se escribe en el fichero el total de cuaterniones y la medida de ciclos
  if(fprintf(fichero, "%d %f\n", total, medidaCiclos) < 0)
  {
    printf("Hubo un error al escribir el fichero\n");
    return 1;
  }

	//Se cierra el fichero
  if(fclose(fichero) == EOF)
  {
    printf("Hubo un error al cerrar el archivo\n");
    return 1;
  }

	//Se libera la memoria de los vectores dinamicos utilizados
	_mm_free(A1);
	_mm_free(B1);
	_mm_free(A2);
	_mm_free(B2);
	_mm_free(A3);
	_mm_free(B3);
	_mm_free(A4);
	_mm_free(B4);
	_mm_free(multiplicacion);
	_mm_free(Resultado);

	//Finalizacion normal del programa
  return 0;
}

//Funcion para representar en pantalla un cuaternion
void representar_quaternion(float* quaternion)
{
    printf("%f, %fi, %fj, %fk\n", quaternion[0], quaternion[1], quaternion[2], quaternion[3]);
}

void access_counter(unsigned *hi, unsigned *lo)
{
  asm("rdtsc; movl %%edx,%0; movl %%eax,%1"
  : "=r" (*hi), "=r" (*lo)
  :
  : "%edx", "%eax");
}

void start_counter()
{
  access_counter(&cyc_hi, &cyc_lo);
}

double get_counter()
{
  unsigned ncyc_hi, ncyc_lo, hi, lo, borrow;
  double result;

  access_counter(&ncyc_hi, &ncyc_lo);

  lo = ncyc_lo - cyc_lo;
  borrow = lo > ncyc_lo;
  hi = ncyc_hi - cyc_hi - borrow;
  result = (double) hi * (1 << 30) * 4 + lo;

  if (result < 0)
    fprintf(stderr, "Error: counter returns neg value: %.0f\n", result);

  return result;
}
