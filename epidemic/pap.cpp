#include"pap.hpp"
#include<map>

namespace sn{
  // This is a helper class to maintain which device an Access Point or Client is
  // connected to and at what time.
  class connected{ 
    int device_id; // Device Connected to
    long int timestamp; // Time of connection
    bool is_it_connected,file;
    public:
    connected(){device_id=-1;timestamp=-1;is_it_connected=false;file=false;};
    int connected_to(){return device_id;};
    bool has_file(){return file;};
    void receive_file(){file=true;};
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

  struct connected_clients{ // this is used in the form of an array. 
                            // Here the array is idnexed by the Access Point ID
                            // And a map of "connections" is kept, which is keyed by 
                            // the client_id and the second number represents the time connected for
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

  void populate_bit_array(std::vector<int> list, bool (& to_mark)[15000]){
    using namespace std;
    vector<int>::iterator it;
    for(it = list.begin();it!=list.end();it++){
      to_mark[*it]=true;
    }
  }
  int transfer_files(int cli_id,int ap_id,connected_clients (& access_points)[15000],
  connected (& users)[15000]){
    int count = 0; //count of devices file trasnfered to
    for(std::map<int,long int>::iterator it = access_points[ap_id].connection.begin();
    it!= access_points[ap_id].connection.end();
    it++){
      if( ! users[it->first].has_file()){
        users[it->first].receive_file();
        count++;
      }
    }
    return count;
  }

  // Function source. Function defined in pap.hpp, as static
  bool process_n_populate(std::vector<log> & data, graph &g,std::vector<int> & m_users){
    using namespace std;

    bool marked_users[15000] = {0};
    populate_bit_array(m_users,marked_users);

    connected users[15000]; // To maintain who clients are connected to
    connected_clients access_points[15000]; // To maintain who access points are
                                            // connected to
    // Use this loop to loop over logs one by one, and 
    // keep track who are the news users who have entered an AP where a
    // user with a file is already connected.
    // If that user is a "pre-approved" carrier, then transfer the file
    // And so on
    // Non-english version:
    // - Consider only marked users - those who are participating (using bit)
    // - Add bit to user class to know who has file
    // - After each iteration check how many devices have gotten the file
    // - Break if total marked users = total users who have gotten file
    int count = 0;
    // Need to seed file somewhere first
    // Seeding at first of m_users
    users[m_users[0]].receive_file(); // Incase m_users.size == 0 , exception will be thrown
                                      // Purposefully uncaught. Should not happed
    for(vector<log>::iterator it = data.begin(); it!=data.end();it++){
      if(marked_users[it->cli_id]){
        int status = get_change_in_status(users,it);
        // Following line has to appear before any "update" functions
        int previous_access_point_id = users[it->cli_id].connected_to();
        // End - last comment

        update_users(users,it,status);
        vector<edge> list_of_edges = update_access_points(access_points,it,status,
        previous_access_point_id);
        if(users[it->cli_id].has_file()){
          count += transfer_files(it->cli_id,it->ap_id,access_points,users);
        }
        if(count == m_users.size()-1){ // -1 because 1 user already has the file
          std::cout<<"All users have received the file"<<std::endl;
          std::cout<<"Time finished: "<<it->get_time()<<std::endl;
          break;
        }
        //TBD in future, pass reference to graph g, and add edges in the function
        // itself
        /*
        for(vector<edge>::iterator  e = list_of_edges.begin();
        e != list_of_edges.end();
        e ++){
          g.add_edge(*e);
        }*/
      }
    }
    std::cout<<count<<"/"<<m_users.size()<<" received the file within the given time period"<<std::endl;
  }
}
