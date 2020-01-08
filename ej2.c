//---------------BIBLIOTECAS UTILIZADAS---------------

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
  float *a, *b, multiplicacion[4], reduccion[] = {0.0, 0.0, 0.0, 0.0};
  int i, j, total, semilla = getpid();
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

  //Se reserva memoria dinamicamente para los vectores de cuaterniones a,b y multiplicacion
  a = (float*)malloc(total*4*sizeof(float));
  b = (float*)malloc(total*4*sizeof(float));

  //Se inicializan los cuaterniones de a y b con numeros aleatorios en el rango definido por las constantes
  for(i = 0; i < total; i++)
    for(j = 0; j < 4; j++)
    {
      *(a + i*4 + j) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
      *(b + i*4 + j) = drand48() * (LIMITE_SUP - LIMITE_INF) + LIMITE_INF;
    }

  //Se inicia el cronometro de ciclos
  start_counter();

  //En el mismo bucle se hace todo el computo
  for(i = 0; i < total; i++)
  {
    //Primero se calcula la multiplicacion de a y b correspondiente
    multiplicacion[0] = *(a + i*4 + 0) * *(b + i*4 + 0) - *(a + i*4 + 1) * *(b + i*4 + 1) - *(a + i*4 + 2) * *(b + i*4 + 2) - *(a + i*4 + 3) * *(b + i*4 + 3);
    multiplicacion[1] = *(a + i*4 + 0) * *(b + i*4 + 1) + *(a + i*4 + 1) * *(b + i*4 + 0) + *(a + i*4 + 2) * *(b + i*4 + 3) - *(a + i*4 + 3) * *(b + i*4 + 2);
    multiplicacion[2] = *(a + i*4 + 0) * *(b + i*4 + 2) - *(a + i*4 + 1) * *(b + i*4 + 3) + *(a + i*4 + 2) * *(b + i*4 + 0) + *(a + i*4 + 3) * *(b + i*4 + 1);
    multiplicacion[3] = *(a + i*4 + 0) * *(b + i*4 + 3) + *(a + i*4 + 1) * *(b + i*4 + 2) - *(a + i*4 + 2) * *(b + i*4 + 1) + *(a + i*4 + 3) * *(b + i*4 + 0);

    //Despues se acumula ya esa multiplicacion haciendo el producto y sumandola a la reduccion
    reduccion[0] += multiplicacion[0] * multiplicacion[0] - multiplicacion[1] * multiplicacion[1] - multiplicacion[2] * multiplicacion[2] - multiplicacion[3] * multiplicacion[3];
    reduccion[1] += multiplicacion[0] * multiplicacion[1];
    reduccion[2] += multiplicacion[0] * multiplicacion[2];
    reduccion[3] += multiplicacion[0] * multiplicacion[3];
  }

  reduccion[1] *= 2;
  reduccion[2] *= 2;
  reduccion[3] *= 2;

  //Se para el cronometro de ciclos
  medidaCiclos = get_counter();

  //Se libera la memoria de los vectores dinamicos utilizados
  free(a);
  free(b);

  //Se muestra el resultado del sumatorio para ver si coincide con las otras versiones
  printf("Resultado sumatorio: ");
  representar_quaternion(reduccion);

  //Se abre el fichero en modo append
  if((fichero = fopen("registro2.txt", "a")) == NULL)
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

  //Finalizacion normal del programa
  return 0;
}

//Funcion para representar en pantalla un cuaternion
void representar_quaternion(float* quaternion)
{
  printf("%f + %fi + %fj + %fk\n", *(quaternion + 0), *(quaternion + 1), *(quaternion + 2), *(quaternion + 3));
}

//---------------IMPLEMENTACION FUNCIONES DE MEDIDA---------------

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
