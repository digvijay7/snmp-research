#include"pap.hpp"
#include<map>

namespace sn{
  // This is a helper class to maintain which device an Access Point or Client is
  // connected to and at what time.
  class connected{ 
    int device_id; // Device Connected to
    long int timestamp; // Time of connection
    bool is_it_connected;
    public:
    connected(){device_id=-1;timestamp=-1;is_it_connected=false;};
    int connected_to(){return device_id;};
    long int connected_at(){return timestamp;};
    bool is_connected(){ return is_it_connected;};
    void set_is_connected(bool value){ is_it_connected = value;};
    void got_connected_to(int d_id){device_id = d_id;};
    void got_connected_at(long int ts){timestamp = ts;};
    void reset(){is_it_connected = false;};
    bool operator==(const connected &o)const {
        return device_id == o.device_id;
    }
    bool operator<(const connected &o) const{
        return device_id < o.device_id;
    }

  };

  struct connected_clients{
    int ap_id;
    std::map<int,long int> connection;
  };
  struct waypoint_connected_clients{
    std::map<int,std::pair<long int, int> > connection;
  };

  int get_change_in_status(connected (& users)[15000],std::vector<log>::iterator l){
    int client_id = l->cli_id;
    int access_point = l->ap_id;
    long int timestamp = l->ts;
    int trap_type = l->type;
    int status = 0;
    if( ! (users[client_id].is_connected()) and trap_type != 1){
    }
    else if( ! (users[client_id].is_connected()) and trap_type == 1){
      status = 1;
    }
    else if( (users[client_id].connected_to() == access_point) and (trap_type %2 != 0) ){
      status = 2;
    }
    else if( (users[client_id].connected_to() == access_point) and (trap_type%2 ==0) ){
      status = 3;
    }
    // Cases where access_point ids are different
    else if( trap_type == 1){
      // Connected to new access point w/o disconnecting from previous AP
      status = 4;
    }
    else if(trap_type == 3){
      std::cerr<<"Trap type 3 encounterred. Ignoring.\n";
    }
    else{
      status = -1;
      std::cerr<<"Should never reach here, in pap.cpp::update_users()\n";
      std::cerr<<"Log:"<<access_point<<" "<<client_id<<" "<<timestamp;
      std::cerr<<" "<<trap_type<<std::endl;
    }
    return status;
  }


  void update_users(connected (& users)[15000],std::vector<log>::iterator l,int status){
    int client_id = l->cli_id;
    int access_point = l->ap_id;
    long int timestamp = l->ts;
    int trap_type = l->type;
    if(status <= 0 or status == 2) return;
    else if( status == 1){
      users[client_id].got_connected_to(access_point);
      users[client_id].got_connected_at(timestamp);
      users[client_id].set_is_connected(true);
    }
    else if( status == 3){
      // Disconnected
      long int time_connected; // Can be used in the future
      time_connected = timestamp - users[client_id].connected_at();
      users[client_id].reset();
    }
    // Cases where access_point ids are different
    else if( status == 4){
      // Connected to new access point
      long int time_connected; // Can be used in the future
      time_connected = timestamp - users[client_id].connected_at();
      users[client_id].got_connected_to(access_point);
      users[client_id].got_connected_at(timestamp);
    }
    else{
      std::cerr<<"Unknown status received(>5):"<<status<<std::endl;
    }
    return;
  }
  std::vector<edge> update_access_points(connected_clients (& access_points)[15000],
  std::vector<log>::iterator l, int status,int previous_access_point){
    using namespace std;
    int client_id = l->cli_id;
    int access_point_id = l->ap_id;
    long int timestamp = l->ts;
    int trap_type = l->type;
    vector<edge> edges;
    connected_clients & access_point = access_points[access_point_id];
    if(status <= 0 or status == 2) {}
    else if( status == 1){
      map<int,long int>:: iterator it = access_point.connection.find(client_id);
      if(it!=access_point.connection.end()){
        //Error, 1st time the client has connected, so it should not be found
        cerr<<client_id<< " got connected to "<<access_point_id;
        cerr<< " for the first time. Should not be found\n";
        // throw error
        throw 1; //Throwing 1 until I know best practice :P
      }
      access_point.connection[client_id] = timestamp;
    }
    else if( status == 3){
      // Disconnected
      map<int,long int> :: iterator it;
      // Generate edges
      for(it = access_point.connection.begin();it!=access_point.connection.end();
      it++){
        if(it->first != client_id){
          edge e(client_id,it->first,timestamp - it->second);
/*          e.u = client_id;
          e.v = it->first;
          e.weight = timestamp - it->second;*/
          edges.push_back(e);
        }
      }
      // Remove entry
      access_point.connection.erase(client_id);
    }
    // Cases where access_point ids are different
    else if( status == 4){
      // Connected to new access point
      map<int,long int> :: iterator it;
      // Generate edges
      for( it = access_points[previous_access_point].connection.begin();
      it!=access_points[previous_access_point].connection.end(); 
      it++ ){
        if(it->first != client_id){
          edge e(client_id,it->first,timestamp - it->second);
/*          e.u = client_id;
          e.v = it->first;
          e.weight = timestamp - it->second;*/
          edges.push_back(e);
        }
      }
      // Remove entry
      access_points[previous_access_point].connection.erase(client_id);
      // Add to new access_point
      access_point.connection[client_id] = timestamp;
    }
    else{
      std::cerr<<"Unknown status received(>5):"<<status<<std::endl;
    }
    return edges;
  }

  // Function source. Function defined in pap.hpp, as static
  bool process_n_populate(std::vector<log> & data, graph &g){
    using namespace std;
    connected users[15000]; // To maintain who clients are connected to
    connected_clients access_points[15000]; // To maintain who access points are
                                            // connected to
    for(vector<log>::iterator it = data.begin(); it!=data.end();it++){
      int status = get_change_in_status(users,it);
      // Following line has to appear before any "update" functions
      int previous_access_point_id = users[it->cli_id].connected_to();
      // End - last comment

      update_users(users,it,status);
      vector<edge> list_of_edges = update_access_points(access_points,it,status,
      previous_access_point_id);
      //TBD in future, pass reference to graph g, and add edges in the function
      // itself
      for(vector<edge>::iterator  e = list_of_edges.begin();
      e != list_of_edges.end();
      e ++){
        g.add_edge(*e);
      }
    }
  }
  std::vector<edge> update_access_points_movements(
  waypoint_connected_clients (&access_points)[15000],
  std::vector<log>::iterator l,
  int time_since,
  int previous_ap_id,
  long int previous_connection_time){
    using namespace std;
    int client_id = l->cli_id;
    int access_point_id = l->ap_id;
    long int timestamp = l->ts;
    int trap_type = l->type;
    vector<edge> edges;
    waypoint_connected_clients & access_point = access_points[access_point_id];
    if(previous_ap_id == access_point_id){
      access_point.connection[client_id].first = timestamp;
      // Don't update previous ap_id, for client.
      // It can still be used to get meaningful movements

      // Get only those *new* movements which happened after the last trap
      if( (timestamp -  previous_connection_time ) < time_since){
        time_since = timestamp - previous_connection_time;
      }

      // Find all people who moved to the current AP, from the same
      // previous AP; Generate edges
      map<int,pair<long int,int> >::iterator other_clients;
      for(other_clients = access_point.connection.begin();
      other_clients != access_point.connection.end();
      other_clients++){
        if(other_clients->first != client_id){ // Different Clients
/*          cerr<<"Line:"<<other_clients->second.second;
          cerr<<" "<<previous_ap_id<<" "<<other_clients->second.first;
          cerr<<" "<<timestamp<<" "<<time_since<<endl;*/
          if(other_clients->second.second == previous_ap_id and 
            timestamp - other_clients->second.first  <= time_since){
            edge e(client_id,other_clients->first,1);
            edges.push_back(e);
          }
        }
      }
    }
    else{

      //Delete old waypoint conenction (Connection to previous AP)
      access_points[previous_ap_id].connection.erase(client_id);

      access_point.connection[client_id].first = timestamp;
      access_point.connection[client_id].second = previous_ap_id;
      
      // Find all people who moved to the current AP, from the same
      // previous AP; Generate edges
      map<int,pair<long int,int> >::iterator other_clients;
      for(other_clients = access_point.connection.begin();
      other_clients != access_point.connection.end();
      other_clients++){
        if(other_clients->first != client_id){ // Different Clients
/*          cerr<<"Line:"<<other_clients->second.second;
          cerr<<" "<<previous_ap_id<<" "<<other_clients->second.first;
          cerr<<" "<<timestamp<<" "<<time_since<<endl;*/
          if(other_clients->second.second == previous_ap_id and 
            timestamp - other_clients->second.first <= time_since){
            edge e(client_id,other_clients->first,1);
            edges.push_back(e);
          }
        }
      }
    }
    return edges;
  }

  bool process_movement(std::vector<log> & data, graph &g,int time){
    using namespace std;
    connected users[15000];
    waypoint_connected_clients access_points[15000];
    for(vector<log>::iterator it= data.begin();it!=data.end();it++){
      if(it->type == 1){ // Only need to consider trap type 1 (associations)
        //The following lines must appear before any updates to data structures
        int previous_access_point_id = users[it->cli_id].connected_to();
        long int previous_connection_time = users[it->cli_id].connected_at();
        // End - Before update
        //Update users data structure
        users[it->cli_id].got_connected_to(it->ap_id);
        users[it->cli_id].got_connected_at(it->ts);
        //End - update users
        //Update access_point data
        vector<edge> list_of_edges = update_access_points_movements(
        access_points,it,time,previous_access_point_id,
        previous_connection_time);
        // End - update access_points
        for(vector<edge>::iterator e = list_of_edges.begin();
        e != list_of_edges.end();
        e++){
          g.add_edge(*e);
        }
      }
    }
  }
}
