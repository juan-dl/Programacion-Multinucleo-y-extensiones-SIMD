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
	float *x, *y;
	float k[4] __attribute__((aligned(16)));
	double medidaCiclos;

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

	__m128 b1221, b3443, b2112, b4334, parteResta, parteSuma, Cuadrado, _0000, Cons0, Cons1, Cons2, ResParc, Producto, Resultado;
	__m128 *a, *b, *suma, *multiplicacion;

	//Se reserva memoria dinamicamente para los vectores de cuaterniones a, b, suma y multiplicacion
	a = (__m128*)_mm_malloc(total*sizeof(__m128), 16);
	b = (__m128*)_mm_malloc(total*sizeof(__m128), 16);
	suma = (__m128*)_mm_malloc(total*sizeof(__m128), 16);
	multiplicacion = (__m128*)_mm_malloc(total*sizeof(__m128), 16);

	//Se reserva memoria dinamicamente para x e y
	x = (float*)_mm_malloc(4 * sizeof(float),16); //Ambas variables deben estar alineadas a 16 bytes
  y = (float*)_mm_malloc(4 * sizeof(float),16);

  for(i = 0; i < total; i++) //Se realiza un bucle para obtener todos los valores aleatorios necesarios para realizar los calculos
	{
    *(x + 0) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		*(y + 0) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		*(x + 1) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		*(y + 1) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		*(x + 2) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
		*(y + 2) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
    *(x + 3) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
    *(y + 3) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;

		*(a + i) = _mm_load_ps(&x[0]); //Finalmente se carga el valor en una posicion del cuaternion
		*(b + i) = _mm_load_ps(&y[0]);
	}

  for(i = 0; i < total; i++)
  {
  	*(suma + i) = _mm_add_ps(a[i],b[i]);
  }

	Cons0 = _mm_set1_ps(0.0);
	Cons1 = _mm_set_ps(-1.0,-1.0,-1.0,1.0); //Cargamos tres registros de 128 bits con valores constantes, los cuales utilizaremos para calculos posteriores
	Cons2 = _mm_set_ps(2.0,2.0,2.0,1.0);

	//Se inicia el cronometro de ciclos
	start_counter();

	Resultado = _mm_setzero_ps(); //Inicializamos el registro resultado

	for(i = 0; i < total; i++)
  {

		b1221 = _mm_shuffle_ps(b[i],b[i],_MM_SHUFFLE(0,1,1,0));
		b3443 = _mm_shuffle_ps(b[i],b[i],_MM_SHUFFLE(2,3,3,2)); //Realizamos una serie de shuffles para obtener los operandos deseados de la componente de la multiplicacion b
		b2112 = _mm_shuffle_ps(b[i],b[i],_MM_SHUFFLE(1,0,0,1));
		b4334 = _mm_shuffle_ps(b[i],b[i],_MM_SHUFFLE(3,2,2,3));

		parteResta = _mm_hsub_ps(_mm_mul_ps(a[i],b1221),_mm_mul_ps(a[i],b3443));//Empleando las funciones hsub y hadd vamos restando/sumando las componentes de la multiplicacion entre si 2 a 2
		parteSuma = _mm_hadd_ps(_mm_mul_ps(a[i],b2112),_mm_mul_ps(a[i],b4334));


		//Realizamos un addsub de forma que se alterna entre una resta(en las posiciones pares) y una suma(en las posiciones impares), combinados con suffles para obtener el orden deseado
		multiplicacion[i] = _mm_addsub_ps(_mm_shuffle_ps(parteResta,parteSuma,_MM_SHUFFLE(0,2,2,0)),_mm_shuffle_ps(parteSuma,parteResta,_MM_SHUFFLE(3,1,1,3)));
		multiplicacion[i] = _mm_shuffle_ps(multiplicacion[i],multiplicacion[i],_MM_SHUFFLE(2,1,3,0)); //Finalmente realizamos un shuffle final para obtener el resultado final de la multiplicaion

		_0000 = _mm_shuffle_ps(multiplicacion[i],multiplicacion[i],_MM_SHUFFLE(0,0,0,0)); //Obtenemos un registro que solo contiene la primera posicion de la multiplicacion 

		Cuadrado = _mm_hadd_ps(_mm_hadd_ps(_mm_mul_ps(_mm_mul_ps(multiplicacion[i],multiplicacion[i]),Cons1),Cons0),Cons0); //Usando dos veces hadd combinamos en una unica componente el cuadrado de la resta 																																																														de los elementos de la multiplicacion 		
		Producto = _mm_mul_ps(_0000,multiplicacion[i]); //Calculamos las otras tres componentes del cuadrado de la multiplicacion
		ResParc = _mm_shuffle_ps(_mm_shuffle_ps(Cuadrado,Producto,_MM_SHUFFLE(1,1,0,0)),Producto,_MM_SHUFFLE(3,2,2,0)); // Para finalizar obtenemos el resultado correcto reordenando el registro usando shuffles

		Resultado = _mm_add_ps(Resultado,ResParc); //Sumamos el resultado parcial obtenido antes al resultado total
  }

	Resultado = _mm_mul_ps(Resultado,Cons2); //Multiplicamos las ultimas tres componentes del cuaternion por dos

	//Se para el cronometro de ciclos
	medidaCiclos = get_counter();

	_mm_store_ps(&k[0], Resultado);

	//Se muestra el resultado del sumatorio para ver si coincide con las otras versiones
	printf("Resultado sumatorio: ");
	representar_quaternion(k);

	//Se abre el fichero en modo append
	if((fichero = fopen("registro3A.txt", "a")) == NULL)
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
	_mm_free(a);
	_mm_free(b);
	_mm_free(suma);
	_mm_free(multiplicacion);
  _mm_free(x);
 	_mm_free(y);

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
