#include "Kmeans.h"
#include<iostream>
#include<fstream>
#include <ctime>

//variable declaration
std::string indexPath="";
unsigned long num_docs=0;
unsigned int num_iterations = 0;
unsigned int k = 0;
std::string clustering_type="kmeans"; //default

//Read paramaters from the config file
void readParameters()
{
  std::ifstream file("fetch_parameters.cfg");
  std::string str;
  std::string delimiter = "=";

  while(std::getline(file,str))
  {
    std::string parameter = str.substr(0,str.find(delimiter));
    //cout<<parameter<<"\n\n";

    if(parameter=="indexPath")
      indexPath = str.substr(str.find(delimiter)+1,str.length()-1);
    else if(parameter=="num_docs")
      num_docs = stol(str.substr(str.find(delimiter)+1,str.length()-1));
    else if(parameter=="clustering_type")
      clustering_type = str.substr(str.find(delimiter)+1,str.length()-1);
    else if(parameter=="iterations")
      num_iterations = stol(str.substr(str.find(delimiter)+1,str.length()-1));
    else if(parameter=="k")
      k = stol(str.substr(str.find(delimiter)+1,str.length()-1));
  }


  file.close();
}

//Create an unordered map of Stopwords
void readStopwords(Kmeans *kmeans)
{
  std::cout << "\nReading stopwords from the file...";
  std::ifstream file("stopwords.txt");
  std::string str;
  while(std::getline(file,str))
  {
    if (str!="") {
      kmeans->stopwords.insert(make_pair(str,1));
    }
  }
  std::cout << "done.";
}

int main(int argc,char *argv[])
{
    readParameters();
    srand(time(NULL));
    std::cout<<indexPath<<"\n";

    if(clustering_type=="kmeans")
    {
      //clock_t start_s=clock();
      time_t start;
      time(&start);


      Kmeans *kmeans= new Kmeans(indexPath,num_docs,k,num_iterations);
      readStopwords(kmeans);
      time_t matrixStart;
      time(&matrixStart);
      kmeans->buildDocumentTermMatrix();
      time_t matrixEnd;
      time(&matrixEnd);
      kmeans->filterStems();
      kmeans->getTFIDFWeights();
      kmeans->getNormalizedWeights();
      kmeans->run();
      kmeans->terminate();
      time_t writingStart;
      time(&writingStart);
      kmeans->writeToDisk();
      time_t writingEnd;
      time(&writingEnd);
      double totalSeconds,matrixSeconds,writeSeconds;
      time_t end;
      time(&end);
      totalSeconds = difftime(end,start);
      matrixSeconds = difftime(matrixEnd,matrixStart);
      writeSeconds = difftime(writingEnd,writingStart);

      std::cout << "Total time taken (seconds):" << totalSeconds<<endl;
      std::cout << "Time taken to build document term matrix (seconds):" << matrixSeconds<<endl;
      std::cout << "Time taken to write to disk (seconds):" << writeSeconds<<endl;
    }



}
