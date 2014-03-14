#include <memory>
#include <iostream>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <dvo/adaptor.hpp>

using std::cout;
using std::endl;
int main( int argc, char *argv[] ) {

     LIBXML_TEST_VERSION

     auto adaptor = std::make_shared<dvo::adaptor>("dVO","/casa/dVO");
     
     cout << "Hello World..." << endl;
     casa::DBusSession::instance( ).dispatcher( ).enter( );
}
