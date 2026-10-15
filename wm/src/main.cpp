#include "hyprv4/commands.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
int main(int argc,char** argv){
  hyprv4::CommandRunner runner;
  try{
    for(int i=1;i<argc;++i){const std::string option=argv[i];
      if(option=="--version"){std::cout<<"hyprv4-core 0.1.0\n";return 0;}
      if(option=="--help"){std::cout<<"Usage: hyprv4-core [--config FILE]\nRead WM policy commands from stdin; state emits JSON. No display or shell is opened.\n";return 0;}
      if(option!="--config" || i+1>=argc) throw std::invalid_argument("unknown or incomplete option: "+option);
      std::ifstream file(argv[++i]);if(!file) throw std::runtime_error("cannot read configuration");
      runner.reload(std::string(std::istreambuf_iterator<char>(file),{}));
    }
  }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 2;}
  bool failed=false;std::string line;std::size_t number=0;
  while(std::getline(std::cin,line)){++number;try{const auto reply=runner.execute(line);if(!reply.empty()) std::cout<<reply<<'\n';}
    catch(const std::exception& error){std::cerr<<"line "<<number<<": "<<error.what()<<'\n';failed=true;}}
  if(std::cin.bad()){std::cerr<<"input read failure\n";return 2;}
  return failed?1:0;
}
