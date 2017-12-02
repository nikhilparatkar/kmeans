#include<iostream>
#include<vector>
#include<math.h>
#include<unordered_map>
#include "Cluster.h"
Cluster::Cluster()
{
  this->totalDocCount=0;
}
Cluster::Cluster(Cluster&& other)
{
this->clusterMap=other.clusterMap;
this->totalDocCount=other.totalDocCount;
// reset other
other.totalDocCount=0;
other.clusterMap.clear();
}

void Cluster::addDocumentToCluster(unordered_map<std::string,double> documentMap)
{
  unordered_map<std::string,double>::iterator DocumentMapIterator = documentMap.begin();
  while(DocumentMapIterator!=documentMap.end())
  {
    //Add value to clusterMap here
    // if stem found in the cluster map, add TF-IDF weight to current weight
    // else insert into the map
    if(this->clusterMap.find(DocumentMapIterator->first)==this->clusterMap.end())
    {
      this->clusterMap.insert(make_pair(DocumentMapIterator->first,DocumentMapIterator->second));
    }
    else
    {
      this->clusterMap[DocumentMapIterator->first]+=DocumentMapIterator->second;
    }
    DocumentMapIterator++;
  }

  totalDocCount++;
}

void Cluster::CalculateCentroids()
{

}
