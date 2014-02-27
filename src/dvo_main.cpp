#include <memory>
#include <iostream>
#include <dvo/adaptor.hpp>

using std::cout;
using std::endl;
int main( int argc, char *argv[] ) {
     auto adaptor = std::make_shared<dvo::adaptor>("dVO","/casa/dVO");
     
     cout << "Hello World..." << endl;
     casa::DBusSession::instance( ).dispatcher( ).enter( );
}
