//---------------BIBLIOTECAS UTILIZADAS---------------

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>

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
  float *a, *b, reduccion[] = {0.0, 0.0, 0.0, 0.0};
  int i, j, total, numeroHilos, semilla = getpid();
  double medidaCiclos;

  //Se comprueba si el numero de argumentos es valido se exige como minimo el numero de cuaterniones del computo y el numero de hilos
  if(argc >= 3 && argc <= 4)
  {
    //Se comprueba si el numero de cuaterniones es mayor que 0
    if((total = atoi(argv[1])) <= 0)
    {
      printf("El total de operaciones debe ser mayor que 0\n");
      return 1;
    }

    //Se comprueba si el numero de hilos es mayor que 0
    if((numeroHilos = atoi(argv[2])) <= 0)
    {
      printf("El numero de hilos debe ser mayor que 0\n");
      return 1;
    }

    //Se comprueba si la semilla de numeros aleatorios (opcional) es mayor que 0
    if(argc == 4 && (semilla = atoi(argv[3])) < 0)
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

  float aux[numeroHilos][4];

  //Se inicia el cronometro de ciclos
  start_counter();

  //Inicio de la region paralela, en este punto se crean los hilos que se van a usar
  #pragma omp parallel num_threads(numeroHilos) shared(aux)
  {
    //Se calcula el numero del hilo actual (se necesita este identificador)
    int num = omp_get_thread_num();
    float acumulacion[] = {0.0, 0.0, 0.0, 0.0}, mult[4];

    //Se paraleliza el bucle for del computo
    #pragma omp for
    for(i = 0; i < total; i++)
    {
      //Se calcula la multiplicacion de a por b de la iteracion actual
      mult[0] = *(a + i*4 + 0) * *(b + i*4 + 0) - *(a + i*4 + 1) * *(b + i*4 + 1) - *(a + i*4 + 2) * *(b + i*4 + 2) - *(a + i*4 + 3) * *(b + i*4 + 3);
      mult[1] = *(a + i*4 + 0) * *(b + i*4 + 1) + *(a + i*4 + 1) * *(b + i*4 + 0) + *(a + i*4 + 2) * *(b + i*4 + 3) - *(a + i*4 + 3) * *(b + i*4 + 2);
      mult[2] = *(a + i*4 + 0) * *(b + i*4 + 2) - *(a + i*4 + 1) * *(b + i*4 + 3) + *(a + i*4 + 2) * *(b + i*4 + 0) + *(a + i*4 + 3) * *(b + i*4 + 1);
      mult[3] = *(a + i*4 + 0) * *(b + i*4 + 3) + *(a + i*4 + 1) * *(b + i*4 + 2) - *(a + i*4 + 2) * *(b + i*4 + 1) + *(a + i*4 + 3) * *(b + i*4 + 0);

      //Una vez calculada se eleva al cuadrado y se acumula en el sumatorio
      acumulacion[0] += mult[0] * mult[0] - mult[1] * mult[1] - mult[2] * mult[2] - mult[3] * mult[3];
      acumulacion[1] += mult[0] * mult[1];
      acumulacion[2] += mult[0] * mult[2];
      acumulacion[3] += mult[0] * mult[3];
    }

    //Se indica a la matriz compartida que acumulaciones pertenecen a cada hilo usando el identificador
    aux[num][0] = acumulacion[0];
    aux[num][1] = acumulacion[1];
    aux[num][2] = acumulacion[2];
    aux[num][3] = acumulacion[3];
  }

  //Se suman las aportaciones de cada hilo en la variable final
  for(i = 0; i < numeroHilos; i++)
  {
    reduccion[0] += aux[i][0];
    reduccion[1] += aux[i][1];
    reduccion[2] += aux[i][2];
    reduccion[3] += aux[i][3];
  }

  //Se multiplica por 2 las filas pendientes (para ahorrar ciclos se extrajo factor comun de ambos sumatorios)
  for(i = 1; i < 4; i++)
    reduccion[i] *= 2;

  //Se para el cronometro de ciclos
  medidaCiclos = get_counter();

  //Se muestra el resultado del sumatorio para ver si coincide con las otras versiones
  printf("Resultado sumatorio: ");
  representar_quaternion(reduccion);

  //Se abre el fichero en modo append
  if((fichero = fopen("registro4.txt", "a")) == NULL)
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
  free(a);
  free(b);

  //Finalizacion normal del programa
  return 0;
}

//Funcion para representar en pantalla un cuaternion
void representar_quaternion(float* quaternion)
{
  printf("%f + %fi + %fj + %fk\n", *(quaternion + 0), *(quaternion + 1), *(quaternion + 2), *(quaternion + 3));
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
  double resultado;

  access_counter(&ncyc_hi, &ncyc_lo);

  lo = ncyc_lo - cyc_lo;
  borrow = lo > ncyc_lo;
  hi = ncyc_hi - cyc_hi - borrow;
  resultado = (double) hi * (1 << 30) * 4 + lo;

  if (resultado < 0)
    fprintf(stderr, "Error: el contador retorno un valor negativo: %.0f\n", resultado);

  return resultado;
}
