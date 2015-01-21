#include<utility>
#include<pqxx/pqxx>
#include<vector>
#include<sstream>

namespace sn{
  class data {
    pqxx::connection * c;
    protected:
    static long int get_time(std::string str){
      tm tm;
      if(str.length() == 0){
        return time(NULL);
      }
      strptime(str.c_str(),"%Y-%m-%d %H:%M:%S",&tm);
      return mktime(&tm);
    }
    public:
    data(){// TBD: Exception Handling!!!!!
      c = new pqxx::connection("dbname=mydb user=postgres password=admin hostaddr=127.0.0.1 port=5432");
    }
    ~data(){
      c->disconnect();
      delete c;
    }
  
    void get_client_ids(std::string from_time, std::string to_time,std::vector<int> & client_ids){
      try{
        pqxx::work w(*c);
        std::string stmt = "SELECT DISTINCT client_id FROM logs JOIN label ON device_id = uid WHERE ts>= to_timestamp('" + from_time + "','yyyy-mm-dd hh24:mi:ss') and ts <= to_timestamp('"+to_time +
        "','yyyy-mm-dd hh24:mi:ss') and building='Academic' and floor = '2' and wing='C' and room='C23' and type != 3;";
        std::cout<<"Making Query:"<<stmt<<std::endl;
        pqxx::result res = w.exec(stmt);
        for(int i = 0;i<res.size();i++){
          client_ids.push_back( res[i][0].as<int>() );
          std::cout<<res[i][0]<<",";
        }
        std::cout<<std::endl;
      }
      catch(std::exception &e){
        std::cerr << e.what() <<std::endl;
      }
    }

    std::string to_string(std::vector<int> & client_ids){
      std::stringstream ss;
      ss << "{";
      for(int i=0;i<client_ids.size();i++){
        if(i!=client_ids.size()-1){
          ss << client_ids[i] <<",";
        }
        else{
          ss << client_ids[i];
        }
      }
      ss << "}";
      return ss.str();
    }

    void get_data(std::string from_time, std::string to_time,std::vector<log> & all_logs,std::vector<int> & client_ids){
      try{
        pqxx::work w(*c);
        std::stringstream ss;
        ss << "SELECT device_id,client_id,ts,type FROM logs WHERE";
        ss << " ts>= to_timestamp('"<< from_time + "'";
        ss << ",'yyyy-mm-dd hh24:mi:ss') and ts <= to_timestamp('" << to_time;
        ss << "','yyyy-mm-dd hh24:mi:ss') and ";
        ss << " client_id IN " <<to_string(client_ids);
        ss << "type != 3 ORDER BY ts;";
        std::cout<<"Making Query:"<<ss.str()<<std::endl;
        pqxx::result res = w.exec(ss.str());
        for(int i = 0;i<res.size();i++){
          int ap_id = res[i][0].as<int>();
          int cli_id = res[i][1].as<int>();
          int type = res[i][3].as<int>();
          log l(ap_id,cli_id,res[i][2].as<std::string>(),type);
          all_logs.push_back(l);
        }
      }
      catch(std::exception &e){
        std::cerr << e.what() <<std::endl;
      }
    }
    void get_person_info(std::vector<std::pair<int,std::pair<std::string,std::string> > > & all_info){
      using namespace std;
      try{
        pqxx::work w(*c);
        std::string stmt = "SELECT uid,rollno,email FROM uid WHERE rollno IS NOT NULL AND email is NOT NULL and type = 'mobile';";
        pqxx::result res = w.exec(stmt);
        for(int i=0;i<res.size();i++){
          pair<string, string> info(res[i][1].as<string>(),res[i][2].as<string>());
          pair<int,pair<string,string> > info_with_uid (res[i][0].as<int>(),info);
          all_info.push_back(info_with_uid);
        }
      }
      catch(exception &e){
        cerr<<e.what()<<endl;
      }
    }


  };
}


