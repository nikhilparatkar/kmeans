#include "indri/Repository.hpp"
#include "indri/QueryEnvironment.hpp"
#include "indri/LocalQueryServer.hpp"
#include<unordered_map>
class Cluster
{
public:
  unordered_map<std::string,double> clusterMap;
  UINT64 totalDocCount;

  Cluster();
  void addDocumentToCluster(unordered_map<std::string,double> documentMap);
  void CalculateCentroids();
};

