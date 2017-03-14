#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

#define SHOW_IMAGE 1

using namespace cv;
using namespace std;

void readFile(string&, vector<string>&, vector<string>&);
int getImgSize(vector<string>&);
Mat mergeMatrix(int, vector<string>&);
Mat subtractMatrix(Mat, Mat);
Mat getAverageVector(Mat, int);
Mat getBestEigenVectors(Mat, Mat, int);

Mat getBestEigenVectors(Mat covar, Mat difference, int imgRows)
{
    //Get all eigenvalues and eigenvectors from covariance matrix
    Mat allEigenValues, allEigenVectors;
    eigen(covar, allEigenValues, allEigenVectors);
    
    Mat eigenVec = allEigenVectors * (difference.t());
    //Normalize eigenvectors
    for(int i = 0; i < eigenVec.rows; i++ )
    {
        Mat tempVec = eigenVec.row(i);
        normalize(tempVec, tempVec);
    }
    
    if (SHOW_IMAGE) {
        //Display eigen face
        Mat eigenFaces, allEigenFaces;
        for (int i = 0; i < eigenVec.rows; i++) {
            eigenVec.row(i).reshape(0, imgRows).copyTo(eigenFaces);
            normalize(eigenFaces, eigenFaces, 0, 1, cv::NORM_MINMAX);
            if(i == 0){
                allEigenFaces = eigenFaces;
            }else{
                hconcat(allEigenFaces, eigenFaces, allEigenFaces);
            }
        }
        
        namedWindow("EigenFaces", CV_WINDOW_NORMAL);
        imshow("EigenFaces", allEigenFaces);
    }
    
    return eigenVec;
}

Mat getAverageVector(Mat facesMatrix, int imgRows)
{
    //To calculate average face, 1 means that the matrix is reduced to a single column.
    //vector is 1D column vector, face is 2D Mat
    Mat vector, face;
    reduce(facesMatrix, vector, 1, CV_REDUCE_AVG);
    
    if (SHOW_IMAGE) {
        vector.reshape(0, imgRows).copyTo(face);
        //Just for display face
        normalize(face, face, 0, 1, cv::NORM_MINMAX);
        namedWindow("AverageFace", CV_WINDOW_NORMAL);
        imshow("AverageFace", face);
    }
    
    return vector;
}

Mat subtractMatrix(Mat facesMatrix, Mat avgVector)
{
    Mat resultMatrix;
    facesMatrix.copyTo(resultMatrix);
    for (int i = 0; i < resultMatrix.cols; i++) {
        subtract(resultMatrix.col(i), avgVector, resultMatrix.col(i));
    }
    return resultMatrix;
}

Mat mergeMatrix(int row, vector<string>& facesPath)
{
    int col = int(facesPath.size());
    Mat mergedMatrix(row, col, CV_32FC1);
    
    for (int i = 0; i < col; i++) {
        Mat tmpMatrix = mergedMatrix.col(i);
        //Load grayscale image 0
        Mat tmpImg;
        imread(facesPath[i], 0).convertTo(tmpImg, CV_32FC1);
        //convert to 1D matrix
        tmpImg.reshape(1, row).copyTo(tmpMatrix);
    }
    //cout << "Merged Matix(Width, Height): " << mergedMatrix.size() << endl;

    return mergedMatrix;
}

int getImgSize(vector<string>& facesPath)
{
    Mat sampleImg = imread(facesPath[0], 0);
    if (sampleImg.empty()) {
        cout << "Fail to Load Image" << endl;
        exit(1);
    }
    //Dimession of Features
    int size = sampleImg.rows * sampleImg.cols;
    //cout << "Per Image Size is: " << size << endl;
    return size;
}

void readFile(string& listFilePath, vector<string>& facesPath, vector<int>& facesID)
{
    ifstream file(listFilePath.c_str(), ifstream::in);
    
    if (!file) {
        cout << "Fail to open file: " << listFilePath << endl;
        exit(0);
    }
    
    string line, path, id;
    while (getline(file, line)) {
        stringstream lines(line);
        getline(lines, id, ';');
        getline(lines, path);
        
        path.erase(remove(path.begin(), path.end(), '\r'), path.end());
        path.erase(remove(path.begin(), path.end(), '\n'), path.end());
        path.erase(remove(path.begin(), path.end(), ' '), path.end());
        
        facesPath.push_back(path);
        facesID.push_back(atoi(id.c_str()));
    }
    
    for(int i = 0; i < facesPath.size(); i++) {
        cout << facesID[i] << " : " << facesPath[i] << endl;
    }
}

int main(int argc, char** argv)
{
    string trainListFilePath = "/Users/zichun/Documents/Assignment/FaceRecognition/FRG/train_list.txt";
    vector<string> trainFacesPath;
    vector<int> trainFacesID;
    
    //Load training sets' ID and path to vector
    readFile(trainListFilePath, trainFacesPath, trainFacesID);
    //Get dimession of features for single image
    int imgSize = getImgSize(trainFacesPath);
    int imgRows = imread(trainFacesPath[0],0).rows;
    //Create a (imgSize X #ofSamples) floating 2D Matrix to store training data
    Mat trainFacesMatrix = mergeMatrix(imgSize, trainFacesPath);
    //Get average face vector
    Mat trainAvgVector = getAverageVector(trainFacesMatrix, imgRows);
    //Subtract average face from faces matrix
    Mat subTrainFaceMatrix = subtractMatrix(trainFacesMatrix, trainAvgVector);
    //Get covariance matrix
    Mat covarMatrix = (subTrainFaceMatrix.t()) * subTrainFaceMatrix;
    //Get eigenvectors
    Mat eigenVectors = getBestEigenVectors(covarMatrix, subTrainFaceMatrix, imgRows);
    cout << "Eigenvectors(W, H): " <<eigenVectors.size() << endl;

    waitKey();
    return 0;
}
