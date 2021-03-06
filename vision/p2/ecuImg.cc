/*!
  Para compilar, puedes ejecutar:
    g++ -Wall -o esqueleto esqueleto.cc `pkg-config opencv --cflags --libs`

*/

#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <exception>

//Includes para OpenCV, Descomentar según los módulo utilizados.
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp> 
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include "ecuImg_funciones.hpp"
#include <valarray>

using namespace std;
using namespace cv;
using namespace vision;

/*!\brief Define los parámetro opcionales de tu programa.

  Redefine esta clase en función de tus necesitados
*/
struct CLIParams
{
  CLIParams ():
  	radio(0),
	imagen(""),
	mascara(""),
	destino(""),
	color(0)
    {}
	int radio;
	const char * imagen;
	const char * mascara;
	const char * destino;
	int color;
};

static void mostrarUso (const char * progname) throw ()
{
	cout << "---------------------------------------------------------------" << endl;
  std::cout << "Este programa sirve para ecualización de imagenes" << std::endl;
	cout << "--------------------------------------------------------------- \n" << endl;
  std::cout << "Uso: " << progname << " [-h] [-i valor] [-m valor] arg1 arg2 ... argn" << std::endl;
  std::cout << "Donde: " << std::endl;
  std::cout << "-h\tMuestra  esta la ayuda." << std::endl;
  std::cout << "-i\tNombre de la imagen." << std::endl;
  std::cout << "-m\tNombre de la mascara" << std::endl;
  std::cout << "-b\tEspacio de color" << std::endl;
}

static int parseCLI (int argc, char* const* argv, CLIParams& params) throw ()
{
  // Esta es una forma habitual de recoger argumentos con getopt
  // se usa una iteracion y cada elemento se pasa por un switch-case
  int opcion;
  while ((opcion = getopt (argc, argv, "r:hi:m:d:b:")) != -1)
  {
    switch (opcion)
    {
      
      case 'h':
	mostrarUso(argv[0]);
	exit (EXIT_SUCCESS);
	break;
	
	case 'r':
		params.radio=atoi(optarg);
		break;
	
      case 'i':
	params.imagen = optarg;
	break;

      case 'm':
	params.mascara = optarg;
	break;

	case 'd':
		params.destino = optarg;
		break;
	case 'b':
		params.color=atoi(optarg);
		break;
	
      case '?': // en caso de error getopt devuelve el caracter ?
	
	if (isprint (optopt))
	  std::cerr << "Error: Opción desconocida \'" << optopt
	    << "\'" << std::endl;
	else
	  std::cerr << "Error: Caracter de opcion desconocido \'x" << std::hex << optopt
	    << "\'" << std::endl;
	mostrarUso(argv[0]);    
	exit (EXIT_FAILURE);
	
	// en cualquier otro caso lo consideramos error grave y salimos
      default:
	std::cerr << "Error: línea de comandos errónea." << std::endl;
	mostrarUso(argv[0]);
	exit(EXIT_FAILURE);	
    }  // case
    
  }// while
  return optind;
}

void creaHistograma( valarray <double> &v , Mat &imagen, Mat &mascara, Mat &destino, int radio);
void ecualizaMono(valarray <double> &v , Mat &imagen, Mat &mascara, Mat &destino, int radio);
void ecualizaColor(valarray <double> &v , Mat &imagen, Mat &mascara, Mat &destino, int &radio);

int main (int argc, char* const* argv)
{
  int retCode=EXIT_SUCCESS;
  
  try {    
    CLIParams params;

    int argObligados = parseCLI(argc, argv, params);

	cout << "---------------------------------" << endl;
    std::cout << "Los parámetros introducidos son:" << std::endl;
    std::cout << "-r\t" << params.radio << std::endl;
    std::cout << "-i\t" << params.imagen << std::endl;
	cout << "-m\t" << params.mascara << endl;
	cout << "-b\t" << params.color << endl;
	cout << "---------------------------------" << endl;
    //std::cout << "-f\t" << params.flotante << std::endl;
    //std::cout << "-c\t" << '\"' << params.cadena << '\"' << std::endl;

    std::cout << "Hay " << argc-argObligados << " parámetros obligados que son: " << std::endl;

    for (int i = argObligados; i<argc; ++i)
      std::cout << '\"' << argv[i] << '\"' << std::endl;


    /*Ahora toca que tu rellenes con lo que hay que hacer ...*/

	if(argc<2)
	{
		cout << "Faltan argumentos (usar -h para ver)" << endl;
		exit(-1);
	}

	Mat imagen, mascara, destino, colorEc, auxImagen, auxDestino; //declaración de las matrices de imagenes

	imagen=imread(params.imagen,-1);//cargamos la imagen
	if(imagen.empty())
	{
		cout << "Imagen no cargada" << endl;
	}

	destino=imread(params.imagen,-1);

	mascara=imread(params.mascara,0);//cargamos la mascara (si hay)
	if(mascara.empty())
	{
		cout << "Mascara no cargada " << endl;
	}

	vector <Mat> v;//declaración del vector para los canales
	valarray <double> histograma(256); //vector para el histograma

	if(imagen.channels()==1)
	{
		//Imagen monocroma
		cout << "Imagen monocroma" << endl;
		ecualizaMono(histograma , imagen, mascara, destino, params.radio);
	}
	else
	{
		//Imagen a color
		if(params.color==0) //espacio de color HSV
		{
			cvtColor(imagen, destino, CV_BGR2HSV); //pasamos la imagen cargada de BGR a HSV
		}
		else if(params.color==1) //espacio de color HLS
		{
			cvtColor(imagen, destino, CV_BGR2HLS); //pasamos la imagen cargada de BGR a HLS
		}
		else if(params.color==2) //espacio de color YCrCb
		{
			cvtColor(imagen, destino, CV_BGR2YCrCb); //pasamos la imagen cargada de BGR a YCrCb
		}
		else //espacio de color La*b*
		{
			cvtColor(imagen, destino, CV_BGR2Lab); //pasamos la imagen cargada de BGR a HSV
		}
		cv::Mat salida;
		split(destino,v);//descomponemos la imagen para ecualizar el canal 2
		v[2].copyTo(salida);
		waitKey(0);

		ecualizaColor(histograma, v[2], mascara, salida, params.radio);
		v[2]=salida;
		merge(v,destino);

		if(params.color==0) //espacio de color HSV
		{
			cvtColor(destino, destino, CV_HSV2BGR); //pasamos la imagen cargada de BGR a HSV
		}
		else if(params.color==1) //espacio de color HLS
		{
			cvtColor(destino, destino, CV_HLS2BGR); //pasamos la imagen cargada de BGR a HLS
		}
		else if(params.color==2) //espacio de color YCrCb
		{
			cvtColor(destino, destino, CV_YCrCb2BGR); //pasamos la imagen cargada de BGR a YCrCb
		}
		else //espacio de color La*b*
		{
			cvtColor(destino, destino, CV_Lab2BGR); //pasamos la imagen cargada de BGR a HSV
		}
	}

	imshow("Imagen sin ecualizar",imagen);
	imshow("Imagen ecualizada",destino);
	imwrite(params.destino,destino);
	waitKey(0);
    
  }
  catch (std::exception& e)
  {
    std::cerr << "Capturada excepcion: " << e.what() << std::endl;
    retCode = EXIT_FAILURE;
  }
  return retCode;
}

void creaHistograma( valarray <double> &v , Mat &imagen, Mat &mascara, Mat &destino, int radio)
{
	v = 0.0;

	for(int i=0; i<imagen.rows; ++i)
	{
		for(int j=0; j<imagen.cols; ++j)
		{
			if(mascara.empty() || mascara.at <unsigned char> (i,j) != 0)
				++v[imagen.at <unsigned char> (i,j)];
		}
	}

	//normalizar
	v /= v.sum();
	//acumular
	for(int i=1;i<256;i++)
	{
		v[i] += v[i-1];
	}
	v *= 255.00;
}

void ecualizaMono(valarray <double> &v , Mat &imagen, Mat &mascara, Mat &destino,int radio)
{
	creaHistograma(v,imagen, mascara, destino,radio);
	for(int i=0; i<imagen.rows; i++)
	{
		for(int j=0;j<imagen.cols;j++)
		{
			if (mascara.empty() || mascara.at <unsigned char>(i,j) != 0)
				destino.at <unsigned char> (i,j) = v[imagen.at <unsigned char> (i,j)];
		}
	}
}

void ecualizaColor(valarray <double> &v , Mat &imagen, Mat &mascara, Mat &destino, int &radio)
{
	if (radio == 0)
	{
		creaHistograma(v,imagen, mascara, destino,radio);
		for(int i=0; i<imagen.rows; i++)
		{
			for(int j=0;j<imagen.cols;j++)
			{
				if (mascara.empty() || mascara.at <unsigned char>(i,j) != 0)
					destino.at <unsigned char> (i,j) = v[imagen.at <unsigned char> (i,j)];
			}
		}
	}
	else
	{
		for(int i=radio; i<imagen.rows-radio; i++)
		{
			for(int j=radio; j<imagen.cols-radio; j++)
			{
				if (mascara.empty() || mascara.at <unsigned char>(i,j) != 0)
				{
					
					Mat ventana = imagen(Rect(j-radio, i-radio, 2*radio+1, 2*radio+1));
					
					Mat masc;
					
					if(not mascara.empty())
						masc = mascara(Rect(j-radio, i-radio, 2*radio+1, 2*radio+1));

					creaHistograma(v,ventana, masc, destino,0);
					
					destino.at <unsigned char> (i,j) = v[imagen.at <unsigned char> (i,j)];					
				}
			}
		}
	}
}
