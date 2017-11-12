#include "Kmeans.h"
#include<iostream>
#include<vector>
#include<math.h>
#include<fstream>
#include<stdlib.h>

/* GET RANDOM DOCUMENTS, GET RANDOM SEEDS
*****************************/
Kmeans::Kmeans(std::string indexPath,int num_docs,int k,int num_iterations)
{
  this->indexPath = indexPath;
  this->num_docs = num_docs;
  this->k = k;
  this->num_iterations=num_iterations;
  std::cout << "Kmeans constructor" << '\n';

  repository.openRead(indexPath);

  /* GET RANDOM N DOCUMENTS in a vector*/
  indri::server::LocalQueryServer local(repository);
  totalDocCount = local.documentCount();

  std::vector<lemur::api::DOCID_T> docs(num_docs);
  //generate random document numbers between 1 and totalDocCount
  //srand( time(NULL) );
  /*for (int i = 0; i < num_docs; i++) {
    docs[i] = 1+rand()%totalDocCount;
  }*/
  int docNum=1;
  for(int i=0;i<num_docs;i++)
  {
    docs[i] = docNum;
    docNum++;
  }
  this->docs=docs;

  /*std::cout << "Docuemnts:" << '\n';
  for (int i = 0; i < docs.size(); i++) {
    std::cout << " " << docs[i];
  }*/

  /* Get K number of random Seeds from the documents vector*/
  /*long seedDoc;
  for(int i=0;i<k;i++)
  {
    seedDoc = rand()%docs.size(); //get random index between 0 and docs.size()
    seeds.push_back(docs[seedDoc]);
  }*/

  seeds.push_back(4);
  seeds.push_back(5);
  seeds.push_back(6);
  std::cout << "\nseeds:" << '\n';
  for(int i=0;i<seeds.size();i++)
  {
    std::cout << " " << seeds[i];
  }


  /*std::cout << "Documents:" << '\n';
  for(int j=0;j<this->docs.size();j++)
  {
    std::cout<<this->docs[j]<<"\n";
  }*/
  /*std::cout << "\nSeeds:" << '\n';
  for(int j=0;j<seeds.size();j++)
  {
    std::cout<<seeds[j]<<"\n";
  }*/



}

void Kmeans::writeDocuments()
{
  indri::server::LocalQueryServer local(repository);

  indri::server::QueryServerVectorsResponse* response = local.documentVectors( docs );

  /* Create a vector of all the unique stems in the selected documents
  *  The index of this vector will give us the posistion on each stem
  */
  //for number of documents

  std::ofstream myfile;
  std::string prefix="/home/nikhil/samplecollection/";
  for(int docCounter=0;docCounter<response->getResults().size();docCounter++){
    indri::api::DocumentVector* docVector = response->getResults()[docCounter];

    stringstream ss;
    ss << docCounter;
    string str = ss.str();
    myfile.open(prefix+str+".txt");

    //For each stem in a document, update the matrix
    for( int stemCounter=0; stemCounter<docVector->positions().size(); stemCounter++ ) {
      int position = docVector->positions()[stemCounter];
      const std::string& stem = docVector->stems()[position];
      myfile<<stem<<" ";
    }

    myfile.close();
  }
}

/* CREATE A MATRIX THAT STORES TF-IDF WEIGHTS OF ALL DOCUMENTS UNDER CONSIDERATION
*****************************/
void Kmeans::buildDocumentTermMatrix()
{
  indri::server::LocalQueryServer local(repository);

  indri::server::QueryServerVectorsResponse* response = local.documentVectors( docs );

  /* Create a vector of all the unique stems in the selected documents
  *  The index of this vector will give us the posistion on each stem
  */
  //for number of documents
  for(int docCounter=0;docCounter<response->getResults().size();docCounter++){
    indri::api::DocumentVector* docVector = response->getResults()[docCounter];

    unordered_map<std::string,double> stemWeights;

    //TEST print
    //std::cout << "\nPrinting docs whole:" << docCounter;

    //For each stem in a document, update the matrix
    for( int stemCounter=0; stemCounter<docVector->positions().size(); stemCounter++ ) {
      int position = docVector->positions()[stemCounter];
      const std::string& stem = docVector->stems()[position];

      //TEST print each stem in the document
      //std::cout << " " << stem;

      // Add to the unique_terms | add 0 as document frequency for now
      if(unique_terms.find(stem)==unique_terms.end())
      {
        unique_terms.insert(make_pair(stem,0));
      }

      // Add to the unordered map of stems in a document
      if(stemWeights.find(stem)==stemWeights.end())
      {
        stemWeights.insert(make_pair(stem,1));
      }
      else
      {
        stemWeights[stem]+=1;
      }
    } // end for -> stems in a document

    //insert into outer map
    DTMap.insert(make_pair(docs[docCounter],stemWeights));

    //Calculate Document Frequencies
    unordered_map<std::string,double>::iterator rowIterator=stemWeights.begin();
    while(rowIterator!=stemWeights.end())
    {
      unique_terms[rowIterator->first]+=1;
      rowIterator++;
    }

    delete docVector;
    stemWeights.clear();
  } //end for -> document vector

  /* Print Unique terms
  */
  /*unordered_map<std::string,long>::iterator it=unique_terms.begin();
  while(it!=unique_terms.end())
  {
    if(it->second>2)
      cout<<" "<<it->first<<":"<<it->second;
    it++;
  }*/

  delete response;
}

/* Filter out very low/high frequency stems
*/
void Kmeans::filterStems()
{
  //float lowCutOff=(float)docs.size() * 0.03;
  float lowCutOff=2;
  float highCutOff=(float)docs.size() * 0.80;
  int low=0,high=0;

  std::cout << "\n\nLowCutoff:"<<lowCutOff<<" HighCutoff:"<<highCutOff<< '\n';
  std::cout << "\n\nNumber of Unique terms:"<<unique_terms.size()<<"\n";

  unordered_map<std::string,long>::iterator it=unique_terms.begin();
  while(it!=unique_terms.end())
  {
    if((float)it->second > highCutOff)
    {
      //cout<<"\nRemoving:"<<it->first;
      //unique_terms.erase(it->first);
      high++;
    }
    else if((float)it->second < lowCutOff)
    {
      cout<<"\nRemoving:"<<it->first;
      unique_terms.erase(it->first);
      low++;
    }
    it++;
  }

  std::cout << "\n\nUpdated Number of Unique terms:"<<unique_terms.size()<<"\n";
  std::cout << "\nLow removed:" <<low<<" High removed:"<<high<< '\n';
}

/* Compute TF-IDF weights for each of the elements of the document term matrix
*/
void Kmeans::getTFIDFWeights()
{
  unordered_map<long, unordered_map<std::string,double> >::iterator DTMapIterator=DTMap.begin();
  while(DTMapIterator!=DTMap.end())
  {
      unordered_map<std::string,double>::iterator stemWeightsIterator = DTMapIterator->second.begin();
      while(stemWeightsIterator!=DTMapIterator->second.end())
      {
        //if the stem has not been removed in the filterstems step, then calculate idf
        //otherwise, remove it from the DTMap
        if(unique_terms.find(stemWeightsIterator->first)!=unique_terms.end())
        {
          double IDF = 1 + log((float)docs.size()/(float)unique_terms[stemWeightsIterator->first]);
          stemWeightsIterator->second*=IDF;
        }
        else
        {
            //remove from DTMap
            DTMap[DTMapIterator->first].erase(stemWeightsIterator->first);
        }

        stemWeightsIterator++;
      }
      DTMapIterator++;
  }
  /* //Print the entire DTMap
  cout<<"\nAfter Calculating TFIDF-\n";
  unordered_map<long, unordered_map<std::string,double> >::iterator It1=DTMap.begin();
  while(It1!=DTMap.end())
  {
      unordered_map<std::string,double>::iterator It2 = It1->second.begin();
      while(It2!=It1->second.end())
      {
        cout<<"\t"<<It2->first<<":"<<It2->second;
        It2++;
      }
      std::cout<< '\n';
      It1++;
  */
}

void Kmeans::getNormalizedWeights()
{
  unordered_map<long, unordered_map<std::string,double> >::iterator DTMapIterator=DTMap.begin();
  //iterate over document rows
  while(DTMapIterator!=DTMap.end())
  {
    double norm = 0;
    unordered_map<std::string,double>::iterator stemWeightsIterator = DTMapIterator->second.begin();
    // Calculate sum of squares of weights
    while(stemWeightsIterator!=DTMapIterator->second.end())
    {
      norm += pow(stemWeightsIterator->second,2.0);
      stemWeightsIterator++;
    }

    //divide each weight by the square root of the sum calculated
    norm = pow(norm,0.5);
    stemWeightsIterator = DTMapIterator->second.begin();
    while(stemWeightsIterator!=DTMapIterator->second.end())
    {
      stemWeightsIterator->second = stemWeightsIterator->second/norm;
      stemWeightsIterator++;
    }

    DTMapIterator++;
  }

  // Print the big matrix
  /*unordered_map<long, unordered_map<std::string,double> >::iterator It1=DTMap.begin();
  while(It1!=DTMap.end())
  {
      unordered_map<std::string,double>::iterator It2 = It1->second.begin();
      while(It2!=It1->second.end())
      {
        cout<<"\t"<<It2->first<<":"<<It2->second;
        It2++;
      }
      std::cout<< '\n';
      It1++;
  }*/
}

double Kmeans::calculateDistance(unordered_map<std::string,double> docMap,unordered_map<std::string,double> seed)
{
  unordered_map<std::string,double>::iterator docIt = docMap.begin();
  double value = 0;
  while(docIt!=docMap.end())
  {
    //x1y1 + x2y2 + ...
    //check if the stem exists in the seed map
    if(seed.find(docIt->first)!=seed.end())
    {
      //multiply the weights of the stem and add to the value
      value += docIt->second * seed[docIt->first];
    }
    docIt++;
  }

  return acos(value);
}

/*void Kmeans::assignDocsToClusters()
{
  unordered_map<long, unordered_map<std::string,double> >::iterator DTMapIterator=DTMap.begin();
  // go through each document vector
  while(DTMapIterator!=DTMap.end())
  {
    double dist=0,min=2.0;
    long nearest;
    //Calculate distance of a document from each seed
    for(int i=0;i<seeds.size();i++)
    {
      dist = calculateDistance(DTMapIterator->second, DTMap[seeds[i]]);
      if(dist<min)
      {
        min = dist;
        nearest = seeds[i];
      }
    }
    //Add the document to the nearest cluster
    Clusters[nearest].addDocumentToCluster(DTMap[nearest]);

    DTMapIterator++;
  }
}*/

void Kmeans::printMap(unordered_map<std::string,double> map)
{
  unordered_map<std::string,double>::iterator it=map.begin();
  while(it!=map.end())
  {
    cout<<" "<<it->first<<":"<<it->second;
    it++;
  }
}

void Kmeans::assignDocsToClusters()
{
  //STEP 1: CLEAR CLUSTER SET 1
  for(int i=0;i<ClusterSet1.size();i++)
  {
    ClusterSet1[i].clusterMap.clear();
    ClusterSet1[i].totalDocCount=0;
  }

  //STEP 2: COPY EVERYTHING FROM SET 2 to SET 1
  for(int i=0;i<ClusterSet2.size();i++)
  {
    ClusterSet1[i].clusterMap = ClusterSet2[i].clusterMap;
  }

  //STEP 3: CLEAR SET 2
  for(int i=0;i<ClusterSet2.size();i++)
  {
    ClusterSet2[i].clusterMap.clear();
    ClusterSet2[i].totalDocCount=0;
  }

  //STEP 4: CALCULATE DISTANCES OF DOCS FROM SET 1 CLUSTERS
  //AND ADD THEM TO SET 2 CLUSTERS
  unordered_map<long, unordered_map<std::string,double> >::iterator DTMapIterator=DTMap.begin();
  // go through each document vector
  while(DTMapIterator!=DTMap.end())
  {
    double dist=0,min=2.0;
    long nearest;
    //Calculate distance of a document from each cluster
    for(int i=0;i<ClusterSet1.size();i++)
    {
      dist = calculateDistance(DTMapIterator->second, ClusterSet1[i].clusterMap);
      if(dist<min)
      {
        min = dist;
        nearest = i;
      }
    }
    //Add the document to the nearest cluster
    //ClusterSet2[nearest].addDocumentToCluster(ClusterSet1[nearest].clusterMap);
    ClusterSet2[nearest].addDocumentToCluster(DTMapIterator->second);

    DTMapIterator++;
  }

  //divide values from each cluster by the number of documents added
  for(int i=0;i<ClusterSet2.size();i++)
  {
    if(ClusterSet2[i].totalDocCount>0)
    {
      int totalDocCount = ClusterSet2[i].totalDocCount;
      unordered_map<std::string,double>::iterator clusterIterator = ClusterSet2[i].clusterMap.begin();
      while(clusterIterator!=ClusterSet2[i].clusterMap.end())
      {
        //divide value by total documents added to the cluster
        clusterIterator->second = clusterIterator->second/totalDocCount;
        clusterIterator++;
      }
    }
  }

}


void Kmeans::run()
{
  /* Initialize clusters here and add them to the cluster matrix.*/

  /* Here, do an additional step of finding out the Indri doc Ids of
    the seed docs that you've selected and pass the new docIds into clusters
  */
  indri::collection::CompressedCollection* collection = repository.collection();

  std::string prefix = "/home/nikhil/testClustering/";
  std::string postfix = ".txt";

  for(int i=0;i<k;i++)
  {
    std::string docid = prefix + std::to_string(static_cast<unsigned long long>(seeds[i])) + postfix;
    Cluster *clusterInstance = new Cluster();
    //Clusters.insert(make_pair(i,*clusterInstance));
    //clusterInstance->clusterMap = DTMap[seeds[i]];

    //get indri ID for each document and find the clusterMap of that document
    clusterInstance->clusterMap = DTMap[collection->retrieveIDByMetadatum( "docno", docid )[0]];

    ClusterSet2.push_back(*clusterInstance);
    ClusterSet1.push_back(*clusterInstance);
  }


  //assignDocsToClusters();
  for(int iteration;iteration<num_iterations;iteration++)
  {
    assignDocsToClusters();
  }

  //print cluster set 2 counts
  for(int i=0;i<ClusterSet2.size();i++)
  {
    std::cout << "\nCluster["<<i<<"] :" << ClusterSet2[i].totalDocCount;
  }

  std::cout <<'\n';
}

void Kmeans::terminate()
{
  repository.close();
}

