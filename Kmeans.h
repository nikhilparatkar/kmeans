#include "indri/Repository.hpp"
#include "indri/QueryEnvironment.hpp"
#include "indri/LocalQueryServer.hpp"
#include "indri/CompressedCollection.hpp"
#include<unordered_map>
#include "Cluster.h"

class Kmeans
{
public:
  indri::collection::Repository repository;
  std::vector<long> seeds;
  UINT64 totalDocCount;
  std::vector<lemur::api::DOCID_T> docs;
  std::string indexPath;
  int num_docs;
  int k;
  int num_iterations;
  //std::vector< vector<double> > documentTermMatrix;
  std::vector<int> documentFreq;
  std::vector<double> euclideanNorms;
  unordered_map<long, unordered_map<std::string,double> > DTMap;
  unordered_map<std::string,long> unique_terms;

  unordered_map<std::string,int> stopwords;

  //std::vector<Cluster> ClusterSet1;
  //std::vector<Cluster> ClusterSet2;
  unordered_map<int, Cluster> ClusterSet1;
  unordered_map<int, Cluster> ClusterSet2;

  Kmeans(std::string indexPath, int num_docs,int k,int num_iterations);
  void buildDocumentTermMatrix();
  void getTFIDFWeights();
  void getNormalizedWeights();
  void filterStems();
  void run();
  void assignDocsToClusters();
  double calculateDistance(unordered_map<std::string,double> docMap,unordered_map<std::string,double> seed);
  void terminate();
  void writeDocuments();
  void printMap(unordered_map<std::string,double> map);
  void testFunction();
  void writeToDisk();
};
