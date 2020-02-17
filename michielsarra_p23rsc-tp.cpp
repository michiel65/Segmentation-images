/* PROJET DE TRAITEMENT D'IMAGE
* Auteur: DJUISSI FOTSO Michiel Sarra
* P23RSC
* Comptage du nombre d'objet dans une image
*/

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/opencv.hpp"


using namespace cv;
using namespace std;


// variable necessaire à l'élimination du bruit gaussien
int MAX_KERNEL_LENGTH = 6;


/*Variables nécessaires à la modification du contraste et de la luminosité*/
int contraste = 1;//alpha pour le contraste. ne change pas lorsque sa valeur est 1
int lum = 45;//beta pour la luminausité. ne change pas lorsque sa valeur est 0

////// Fonction permettant de generer un identifiat en fonction de la l'heure du système //////
string genererID()
{
    time_t seconds;
    time(&seconds);
    stringstream ss;
    ss << seconds;
    string ts = ss.str();
    return "_"+ts;
}
/////elimination du bruit de type poivre et sel dans une image avec le filtre median//////////
Mat bruitMedian(Mat src)
{

 Mat median = src.clone();
 for ( int i = 1; i < MAX_KERNEL_LENGTH; i = i + 2 )
    {
        medianBlur( src, median, i );
        
    }
 return median;
}


/////fonction de correction du Contraste et de la luminausité de l'image///////////////
Mat transflinsaturation(Mat src, int con, int lum)
{
	 Mat new_image = Mat::zeros( src.size(), src.type() );
	 for( int y = 0; y < src.rows; y++ )
	    { for( int x = 0; x < src.cols; x++ )
	         { for( int c = 0; c < 3; c++ )
	              {
	      new_image.at<Vec3b>(y,x)[c] =
	         saturate_cast<uchar>( con*( src.at<Vec3b>(y,x)[c] ) + lum );
	             }
	    }
	    }


	 return new_image;
}


//////////////////Segmentation de l'image/////////////////////

Mat segmentation(Mat src){
    Point p1, p2;
    p1.x = 10;
    p1.y = 0;
    p2.x = 120;
    p2.y = 255;

        Mat otsuImg;
        // conversion de l'image couleur en image en niveaux de gris
        cvtColor(src, otsuImg, COLOR_RGB2GRAY);

        // application de l'algorithme d'OTSU
        threshold(otsuImg, otsuImg, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);
	
return otsuImg;
}
        

//////////////////////////////// Postsegmentation de l'image ////////////////////////
Mat posttraitement(Mat src){
          string timeStamp = genererID();
        // définition des éléments structurants
        Mat struct_1 = getStructuringElement(MORPH_RECT, Size(16, 16));
        Mat struct_2 = getStructuringElement(MORPH_RECT, Size(7, 7));
        Mat transformImg;

        /// dilatation et erosion
        dilate(src, transformImg, struct_1);
        erode(transformImg, transformImg, struct_2);
        imwrite("posttraitement/fermeture"+timeStamp+".jpg", transformImg);
        imshow("fermeture du resultat de la segmentation", transformImg);

        // déclaration d'un vecteur pour contenir les contours des régions détectées
        vector<vector<Point> > contours;

        // recherche des contours des régions détectées
        findContours(transformImg, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
        

/////////////////////////////// Etiquetage de l'image/////////////////////////

        // etiquetage des régions
        Mat markers = Mat::zeros(transformImg.size(), CV_32SC1);
        for (size_t i = 0; i < contours.size(); i++)
            drawContours(markers, contours, static_cast<int>(i),
                    Scalar::all(static_cast<int>(i) + 1), -1);
        
        /// génération aléatoire de couleurs
        vector<Vec3b> colors;
        for (size_t i = 0; i < contours.size(); i++) {
            int b = theRNG().uniform(0, 255);
            int g = theRNG().uniform(0, 255);
            int r = theRNG().uniform(0, 255);
            colors.push_back(Vec3b((uchar) b, (uchar) g, (uchar) r));
        }

        // coloration des régions avec des couleurs différentes
        Mat post_seg_img = Mat::zeros(markers.size(), CV_8UC3);
        for (int i = 0; i < markers.rows; i++) {
            for (int j = 0; j < markers.cols; j++) {
                int index = markers.at<int>(i, j);
                if (index > 0 && index <= static_cast<int>(contours.size()))
                    post_seg_img.at<Vec3b>(i, j) = colors[index - 1];
                else
                    post_seg_img.at<Vec3b>(i, j) = Vec3b(0, 0, 0);
            }
        }

       
        /// fin de la phase de post-Segmentation
    return post_seg_img;
}




//Programme principale
int main (int argc, char* argv[])
{
/////////////	recuperation de l'image à traiter//////////////
   Mat src;

     ///////////////vérifie que les arguments ont bien été passés en paramètres du programme
     //////////// arg[0]=nom du programme, arg[1]= fichier de l'image d'origine

  src = imread (argv[1],1);
  Mat src_bis=src;

  if(!src.data)
  {
    fprintf (stderr, "Impossible d'ouvrir l'image: %s\n", argv[1]);
    return EXIT_FAILURE;
  }else{

  Size size(400,500);// taille de l'image de destination
  resize(src,src,size);//Reduction de la taille de l'image à traiter

  
  

//////// Affichage de l'image originale/////////////////
   namedWindow ("Image initiale", WINDOW_AUTOSIZE);
   imshow("Image initiale", src);
/// Appel de la fonction Generer Id pour generer un identifiant permettant de composer le nom de l'image resultat/////
 string timeStamp = genererID();

/**************** Pretaitement de l'image********************/
  /////////Elimination du bruit de type poivre et sel dans l'image
   Mat median = bruitMedian(src);
   imwrite("pretraitement/median"+timeStamp+".jpg", median);
    namedWindow("filtre median", WINDOW_AUTOSIZE );		
    imshow("filtre median",median);

  //////Modification du contraste et de la luminosité dans l'image par transformation linéaire avec saturation/////
  Mat imgContrast = transflinsaturation(median, contraste, lum);
  imwrite("pretraitement/imagecontrast"+timeStamp+".jpg", imgContrast );
    namedWindow("Ajout de luminosité et contraste", WINDOW_AUTOSIZE );		
    imshow("Ajout de luminosité et contraste",imgContrast);


/***************Segmentation de l'image **********************/
        // image ayant subi l'algorithme OTSU
        Mat otsuImg = segmentation(imgContrast);
        imshow("Segmentation OTSU", otsuImg);
        imwrite("segmentation/OTSUSegmentation"+timeStamp+".jpg", otsuImg);

/******************** posttraitement de l'image **************/
        //  post-segmentation de l'image avec etiquetage des regions
       Mat postsegimg = posttraitement(otsuImg);

        imshow("Image etiqueté", postsegimg);
        imwrite("posttraitement/Imgetiquete"+timeStamp+".jpg", postsegimg);

	// ET logique entre l'image entrée et le masque obtenu après post-segmentation
        Mat segimg;
        imgContrast.copyTo(segimg, postsegimg);
   imshow("Image traité", segimg);
   imwrite("imagestraités/imgBon"+timeStamp+".jpg", segimg);

}

/// Affichage du nom dans une fenetre////////
  string montexte;
  Mat text(200,500,CV_8UC3);
  montexte+=" DJUISSI FOTSO Michiel Sarra";
  text=Scalar(255,255,255);
  
  putText(text, montexte, Point(20, 100), FONT_HERSHEY_SIMPLEX, 1, Scalar(0,0,0),1,8, false);
  moveWindow("TP: Comptage d'objet dans une image", 200,200);
  imshow("TP: Comptage d'objet dans une image", text);

   waitKey (0);
    destroyAllWindows ();
    return EXIT_SUCCESS;
}





