//g++ -std=c++11 final_fotos.cpp `pkg-config --libs --cflags opencv` -o m

#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <utility>
#include <math.h>
#include <fstream>
#include <map>



#include <unistd.h>

using namespace std;
using namespace cv;

VideoCapture TheVideoCapturer;

int minH = 50;
int minS = 90;
int minV = 250;

int maxH = 255;
int maxS = 255;
int maxV = 255;

float theta;
float rotation = 0.0;
vector<vector<int > > pointCloud;

void triangulation(int x, int y, int &_X, int &_Z){
    x = (240-x)*1.0; //240
    _X = x*cos(rotation);
    _Z = -x*sin(rotation);
}

void media(vector<vector<Point> > contours, map<int,int> &mPoints){
    if(contours.size()){
        map<int,int>::iterator it;

        for(int i=0; i<contours.size(); ++i){
            for(int j =0; j < contours[i].size(); ++j){

                it = mPoints.find( contours[i][j].y );
                if( it != mPoints.end() ){
                    mPoints[contours[i][j].y ] = ( mPoints[ contours[i][j].y ] + contours[i][j].x )/2;
                }else{
                    mPoints[ contours[i][j].y ] = contours[i][j].x;
                }
            }
        }
    }
}



void saveObj(){
    ofstream myfile("test.obj");
    string ver = "v ";

    for(int i=0;i<pointCloud.size();++i){
       myfile << "v "<<pointCloud[i][0]<<" "<<pointCloud[i][1]<<" "<<pointCloud[i][2]<<endl;
    }
    myfile.close();
}


void scan(){

    TheVideoCapturer.open("http://192.168.42.129:8080/video?x.mjpeg");
    if( !TheVideoCapturer.isOpened()){
        cout<<"no se pudo abrir a camara"<<endl;
        return;
    }
    int cont = 0;
    int x,y;
    char key = 0;
    float ang = 0.0;

    usleep(500000);

    while(key != 27){
            Mat imgOriginal;
            bool Success = TheVideoCapturer.read(imgOriginal);

            if(!Success){
                cout<<"No se puede abrir video stream"<<endl;
                continue;
            }
            
            Mat imgHSV;
            cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV);
            
            Mat imgThresholded;

            inRange(imgHSV, Scalar(minH, minS, minV),Scalar(maxH,maxS,maxV), imgThresholded); //colores en rango
            resize(imgThresholded, imgThresholded, Size(imgThresholded.cols/2, imgThresholded.rows/2)); // tamaño dela pantalla a la mitad

            //dilate(imgThresholded,imgThresholded,Mat(), Point(-1, -1), 2, 1, 1);
            //erode(imgThresholded,imgThresholded,Mat(), Point(-1, -1), 2, 1, 1);
           
            imshow("test2",imgThresholded);

            Mat salida =imgThresholded.clone();
            Canny(imgThresholded, salida, 100, 200);
            vector< vector< Point> > contours;
            map<int,int> mPoints;
            findContours(salida, contours, RETR_LIST,CV_CHAIN_APPROX_SIMPLE);
            media(contours,mPoints);
            
            vector<Point> mp;
            int _X,_Z;

            cout<<cont<<"casascacas "<< rotation<<"   ang:  "<<ang<<"    angulosss   "<<imgOriginal.cols<< "   "<<salida.rows<<endl;
            for(const auto &myPair : mPoints){ // iteracion, mas rapido?
            if(myPair.first < 790 and myPair.first>120){
                        triangulation(myPair.second,myPair.first,_X,_Z);
                        vector<int> tmp;
                        tmp.push_back(_X);
                        tmp.push_back(myPair.first + 50);
                        tmp.push_back(_Z);
                        pointCloud.push_back(tmp); // malla de puntos
                    }
                }
            rotation= ang*((22/7)/180.0);
            ang+=1.33;

            saveObj();
            if(ang > 400) break;   //355

            imshow("canny",salida);
            key = 0;
            key=waitKey(20);
    }
}



int main(int argc, char *argv[]) { 
    
    scan();

    return 0;
}