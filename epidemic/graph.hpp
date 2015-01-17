#include<map>
#include<iostream>
#include<vector>
#include<sstream>

namespace sn{
  struct node{
    int id;
    std::string name,email;
    bool operator==(const node &o)const {
        return id == o.id;
    }

    bool operator<(const node &o) const{
        return id < o.id;
    }
    void set_name(std::string a){name=a;};
    void set_email(std::string a){email =a;};
  };
  struct edge{
    node u,v;
    long int weight;
    edge(int id1,int id2,long int wt){u.id = id1;v.id = id2;weight = wt;};
  };
  class graph{
    std::map<node, std::map<node,long int> > g; // Map of node to map of neighbor to weight
    public:
    void add_edge(node a, node b, int w){
      long int weight=w;
      std::map<node, std::map<node,long int> > ::iterator it = g.find(a);
      if( it == g.end()){
        ;
      }
      else {
        std::map <node,long int> ::iterator neighbor;
        neighbor = it->second.find(b);
        if(neighbor == it->second.end()){
          ;
        }
        else{
          weight += neighbor->second;
        }
      }
      g[a][b] = weight;
      g[b][a] = weight;
    }
    void get_neighbors_of(node a,std::vector<node> & neighbors){
      if(g.find(a) == g.end()){
        return;
      }
      std::map<node,long int>::iterator it;
      for(it = g[a].begin();it!=g[a].end();it++){
        if(it->first.name!="")
          neighbors.push_back(it->first);
      }
    }
    void get_all_nodes(std::vector<node> & nodes){
      std::map<node,std::map <node,long int> >::iterator it;
      for(it=g.begin();it!=g.end();it++){
        if(it->first.name!="")
          nodes.push_back(it->first);
      }
    }
    long int get_total_edge_weight(node a){
      if(g.find(a) == g.end()){
        return -1;
      }
      std::map<node,long int>::iterator it;
      long int total=0;
      for(it = g[a].begin();it!=g[a].end();it++){
        total += it->second;
      }
      return total;
    }

    long int get_weight(edge e){
      return get_weight(e.u,e.v);
    }
    long int get_weight(node a, node b){
      if(g.find(a)!=g.end() and g.find(b)!= g.end()){
        return g[a][b];
      }
      else{
        return -1;
      }
    }

    void add_edge(edge e){
      add_edge(e.u,e.v,e.weight);
    }
    std::string get_gdf(){
      std::stringstream ss;
      std::map<node, std::map<node, long int> > ::iterator it;
      std::map<node, long int>::iterator neighbor;
      ss << "nodedef>name VARCHAR,name VARCHAR,email VARCHAR\n";
      for(it = g.begin();it!=g.end();it++){
        if(it->first.name != ""){
          ss<< (it->first.id) <<","<< (it->first.name);
          ss<< "," << it->first.email <<"\n";
        }
      }
      std::map<node,std::map<node,bool> > visited;
      ss << "edgedef>node1 VARCHAR, node2 VARCHAR,weight INT\n";
      for(it = g.begin();it!=g.end();it++){
        for(neighbor = (it->second).begin();neighbor != (it->second).end();neighbor++){
          if(!(visited[it->first][neighbor->first])){
            if(it->first.name != "" and neighbor->first.name != ""){
              ss << (it->first.id) << ","<< (neighbor->first.id) <<","<< (neighbor->second) <<"\n";
              visited[it->first][neighbor->first] = true;
              visited[neighbor->first][it->first] = true;
            }
          }
        }
      }
      return ss.str();
    }

    void print(){

      std::map<node, std::map<node, long int> > ::iterator it;
      std::map<node, long int>::iterator neighbor;
      std::cout << "nodedef>name VARCHAR,name VARCHAR,email VARCHAR\n";
      for(it = g.begin();it!=g.end();it++){
        if(it->first.name != ""){
          std::cout<< (it->first.id) <<","<< (it->first.name);
          std::cout<< "," << it->first.email <<"\n";
        }
      }
      std::map<node,std::map<node,bool> > visited;
      std::cout << "edgedef>node1 VARCHAR, node2 VARCHAR,weight INT\n";
      for(it = g.begin();it!=g.end();it++){
        for(neighbor = (it->second).begin();neighbor != (it->second).end();neighbor++){
          if(!(visited[it->first][neighbor->first])){
            if(it->first.name != "" and neighbor->first.name != ""){
              std::cout << (it->first.id) << ","<< (neighbor->first.id) <<","<< (neighbor->second) <<"\n";
              visited[it->first][neighbor->first] = true;
              visited[neighbor->first][it->first] = true;
            }
          }
        }
      }
    }
    void add_node_info(int id,std::string a_rollno,std::string a_email){
      node temp;
      temp.id = id;
      temp.name = a_rollno;
      temp.email = a_email;
      std::map<node, std::map<node,long int> >:: iterator it;
      it = g.find(temp);
      if(it!=g.end()){
        std::cerr<<"graph::add_node_info must be called only once per id\n";
        throw 1;
      }
      else{
        g[temp];
      }
    }
  };
}


