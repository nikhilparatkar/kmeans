#include<iostream>
#include<vector>
#include "indri/Repository.hpp"
#include "indri/QueryEnvironment.hpp"
#include "indri/LocalQueryServer.hpp"
#include<unordered_map>
#include<fstream>
#include<random>

indri::collection::Repository repository;
using namespace indri::api;
using namespace std;
unordered_map<string, double> TFumap;

//variable declaration
std::string indexPath="";
unsigned long num_docs=0;
unsigned int num_iterations = 0;
unsigned int k = 0;
std::string clustering_type="kmeans"; //default

class Document
{
public:
  long docID;
  unordered_map<string, double> TFs;
  Document(long docID)
  {
    this->docID = docID;

  }

  //build a hashmap of TFs of all terms in a document
  void buildTFs()
  {
    indri::server::LocalQueryServer local(repository);
    lemur::api::DOCID_T documentID = docID;

    std::vector<lemur::api::DOCID_T> documentIDs;
    documentIDs.push_back(documentID);

    indri::server::QueryServerVectorsResponse* response = local.documentVectors( documentIDs );

    if( response->getResults().size() ) {
      indri::api::DocumentVector* docVector = response->getResults()[0];

      /*std::cout << "--- Fields ---" << std::endl;

      for( size_t i=0; i<docVector->fields().size(); i++ ) {
        const indri::api::DocumentVector::Field& field = docVector->fields()[i];
        std::cout << field.name << " " << field.begin << " " << field.end << " " << field.number << std::endl;
      }*/

      //std::cout << "--- Terms ---" << std::endl;

      for( size_t i=0; i<docVector->positions().size(); i++ ) {
        int position = docVector->positions()[i];
        const std::string& stem = docVector->stems()[position];
        //std::cout << i << " " << position << " " << stem << std::endl;
        //Add stem to TFs
        if(TFs.find(stem)==TFs.end()){
          TFs.insert(make_pair(stem,1));
        }else{
          TFs[stem]+=1;
        }
      }
      delete docVector;
    }
    delete response;
  }
};

class Cluster
{
public:
  std::vector<long> docs;
  void addDocument(long documentID)
  {
    docs.push_back(documentID);
  }

  void removeDocument(long documentID)
  {
    docs.erase(std::remove(docs.begin(), docs.end(), documentID), docs.end());
  }
};

class Kmeans
{
private:
  UINT64 totalDocCount;
  std::vector<long> seeds;
  //std::vector<double> distances;
  std::vector<long> docs;
  unordered_map<string, double> distances;
  std::vector<Cluster> clusters;
public:
  Kmeans()
  {
    cout<<"In Constuctor\n\n";
    indri::server::LocalQueryServer local(repository);
    totalDocCount = local.documentCount();
    //std::random_device rd;
    //std::mt19937 eng(rd());
    //std::uniform_int_distribution<> distr(1, totalDocCount);

    //this vector stores the doc numbers of the documents under consideration
    std::vector<long> docs(num_docs);

    //generate random document numbers between 1 and totalDocCount
    for (int i = 0; i < num_docs; i++) {
      docs[i] = 1+rand()%totalDocCount;
    }

    //Create k number of random seeds
    long seedDoc;
    for(int i=0;i<k;i++)
    {
      seedDoc = 1+rand()%totalDocCount;
      seeds.push_back(seedDoc);
    }

    //print docs undr consideration
    std::cout << "Docs:" << '\n';
    for(int i=0;i<docs.size();i++)
    {
      cout<<docs[i]<<"\n";
    }

    //print selected seeds
    std::cout << "Seeds:" << '\n';
    for(int i=0;i<seeds.size();i++)
    {
      cout<<seeds[i]<<"\n";
    }

  }


  void assignDocsToClusters()
  {
    indri::server::LocalQueryServer local(repository);
    long seed;
    for(int i=0;i<docs.size();i++)
    {
      seed = getNearestSeed(docs[i]);
      clusters[seed].addDocument(docs[i]);
    }
  }


  //find out the nearest seed for a document
  double getNearestSeed(long docID)
  {
    double min=2.0;
    double distance;
    long nearest;
    for(int i=0;i<k;i++)
    {
      distance = getDistance(docID,seeds[i]);
      if(distance<min)
      {
        min=distance;
        nearest = seeds[i];
      }
    }
    return min;
  }

  //get distance between a document and a seed
  double getDistance(long docID,long seedID)
  {
    Document doc(docID);
    doc.buildTFs();
    Document seed(seedID);
    seed.buildTFs();
    std::vector<string> unique_terms;
    //unordered_map<Key,T>::iterator it;
    //for(auto kv : doc.TFs) {
    //  unique_terms.push_back(kv.first);
    //}
    /*std::unordered_map<std::string, int> mapOfWordCount;
  	// Insert Element in map
  	mapOfWordCount.insert(std::pair<std::string, int>("first", 1));
  	mapOfWordCount.insert(std::pair<std::string, int>("second", 2));
  	mapOfWordCount.insert(std::pair<std::string, int>("third", 3));
  	mapOfWordCount.insert(std::pair<std::string, int>("third", 4));
  	mapOfWordCount.insert(std::pair<std::string, int>("third", 5));
    */
  	// Create a map iterator and point to beginning of map
  	std::unordered_map<std::string, double>::iterator it1 = doc.TFs.begin();
    std::unordered_map<std::string, double>::iterator it2 = seed.TFs.begin();
  	// Iterate over the map using Iterator till end.
  	while (it1 != doc.TFs.end())
  	{
  		// Accessing KEY from element pointed by it.
  		std::string word = it1->first;

  		// Accessing VALUE from element pointed by it.
  		double count = it1->second;

      unique_terms.push_back(it1->first);

  		// Increment the Iterator to point to next entry
  		it1++;
  	}

    while (it2 != seed.TFs.end())
  	{
  		// Accessing KEY from element pointed by it.
  		std::string word = it2->first;

  		// Accessing VALUE from element pointed by it.
  		double count = it2->second;

      //add only if not already present
      if(std::find(unique_terms.begin(), unique_terms.end(), it2->first) == unique_terms.end()){
          unique_terms.push_back(it2->first);
      }
  		// Increment the Iterator to point to next entry
  		it2++;
  	}

    int docTermMatrix[2][unique_terms.size()];
    for(int i=0;i<unique_terms.size();i++)
    {
      docTermMatrix[0][i]=doc.TFs[unique_terms[i]];
    }
    for(int i=0;i<unique_terms.size();i++)
    {
      docTermMatrix[1][i]=seed.TFs[unique_terms[i]];
    }



  }

  void reassignSeeds()
  {

  }

  void run()
  {
    for(int i=0;i<num_iterations;i++)
    {
      assignDocsToClusters();
      reassignSeeds();
    }
  }



  //get TF of a term
  void getTF(string term)
  {
    if(TFumap.find(term)!=TFumap.end()){
      cout<< "\nTF of '"<< term <<"':" << TFumap[term]<<"\n\n";
    }
    else{
      cout<<"\nNot Found\n\n";
    }
  }

  //get field text of a document
  void getFieldtext(int documentID,std::string field)
  {
      std::cout<<"\n***Inside the fucntion.***\n";
      indri::collection::Repository::index_state repIndexState = repository.indexes();
      indri::index::Index *thisIndex=(*repIndexState)[0];

      // get the field ID
      int fieldID=thisIndex->field(field);


      // is this a valid field? If not, exit
      if (fieldID < 1) { return; }

      // retrieve the document vector for this document
      const indri::index::TermList *termList=thisIndex->termList(documentID);

      // ensure we have a valid term list!
      if (!termList) { return; }

      // get the vector of fields
      indri::utility::greedy_vector< indri::index::FieldExtent > fieldVec=termList->fields();

      // go through the fields (create an iterator)
      indri::utility::greedy_vector< indri::index::FieldExtent >::iterator fIter=fieldVec.begin();
      while (fIter!=fieldVec.end()) {
        // does the ID of this field extent match our field ID?
        if ((*fIter).id==fieldID) {
          // yes! We can print out the text here
          int beginTerm=(*fIter).begin;
          int endTerm=(*fIter).end;

          // note that the text is inclusive of the beginning
          // but exclusive of the ending
          for (int t=beginTerm; t < endTerm; t++) {
            // get this term ID
            int thisTermID=termList->terms()[t];
            // convert the term ID to its string representation
            cout << thisIndex->term(thisTermID) << " ";
          }
          cout << endl;
        }

        // increment the iterator
        fIter++;
      }

      // destroy the term list object
      delete termList;
      termList=NULL;
  }

};



/*
void kmeans()
{
  indri::server::LocalQueryServer local(repository);
  UINT64 totalDocCount = local.documentCount();
  //std::random_device rd;
  //std::mt19937 eng(rd());
  //std::uniform_int_distribution<> distr(1, totalDocCount);

  //this vector stores the doc numbers of the documents under consideration
  std::vector<long> docs(num_docs);
  //generate random document numbers between 1 and totalDocCount
  for (int i = 0; i < num_docs; i++) {
    docs[i] = 1+rand()%totalDocCount;
  }


}*/

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
    //const char *indexPath = "/data/clueweb09/indexes/categoryB/central/CW09-B";
    std::cout<<indexPath<<"\n";
    repository.openRead(indexPath);
    //indri::server::LocalQueryServer local(repository);

    if(clustering_type=="kmeans")
    {
      //Kmeans *kmeans= new Kmeans();
    }


    repository.close();

}

