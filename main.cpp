#include "Kmeans.h"
#include<iostream>
#include<fstream>


//variable declaration
std::string indexPath="";
unsigned long num_docs=0;
unsigned int num_iterations = 0;
unsigned int k = 0;
std::string clustering_type="kmeans"; //default

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

int main(int argc,char *argv[])
{
    readParameters();
    std::cout<<indexPath<<"\n";

    if(clustering_type=="kmeans")
    {
      Kmeans *kmeans= new Kmeans(indexPath,num_docs,k,num_iterations);
      //kmeans->writeDocuments();
      kmeans->buildDocumentTermMatrix();
      kmeans->filterStems();
      kmeans->getTFIDFWeights();
      kmeans->getNormalizedWeights();
      kmeans->run();
      kmeans->terminate();
    }



}

