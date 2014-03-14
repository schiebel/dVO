#pragma once
#include <cstddef>
#include <type_traits>
#include <libxml/parser.h>

namespace dvo {
    namespace xml2 {

        struct sax {

            // offsetof(...) cannot be used on a structure until it is complete, i.e. after the closing brace
            // the 'foo' struct above is an interesting case, but I wasn't able to have the template expansion
            // (here 'cb') discover it's offset within the parent structure at the time of expansion. I believe
            // that I could have written a macro that would essentially create callback structures (as in 'foo'
            // above) in place within 'sax', but this would have necessitated a macro for each of the function
            // arities (instead of the nice "packaged arguments/parameters" of variadic templates. The cost of
            // this convienience, though, is meta-information decoupled from the callback table ('table').
            struct Table {

                // typical SAX callbacks "R name(void *ctx, T1 t1, T2 t2 ... )"
                template<unsigned int index, typename R, typename... Args> class cb {
                    std::function<R(Args...)> func;
                    static R cfunc(void *s,Args... args) {
                        return static_cast<cb<index,R,Args...>*>( (void*) ( (char*) s + offsetof(sax,table) +
                                                                            static_cast<sax*>(s)->meta[index] ) )->func(args...);
                    }
                public:
                    cb( ) { }
                    cb( decltype(func) f ) : func(f) { }
                    void operator=( decltype(func) f ) { func = f; }
                    operator bool( ) const { return (bool) func; }
                    operator decltype(func)( ) const { return func; }
                    operator decltype(cfunc)*( ) const { return cfunc; }
                };

                // spacial case SAX stdargs callbacks "void name(void *ctx, const char *, ... )"
                template<unsigned int index> class vacb {
                    std::function<void(std::string)> func;
                    static void cfunc(void *s, const char *format, ...) {
                        char buffer[1024 * 4];
                        va_list argptr;
                        va_start(argptr, format);
                        vsprintf(buffer, format, argptr);
                        va_end(argptr);
                        static_cast<vacb<index>*>( (void*) ( (char*) s + offsetof(sax,table) +
                                                             static_cast<sax*>(s)->meta[index] ) )->func(std::string(buffer));
                    }
                public:
                    vacb( ) { }
                    vacb( decltype(func) f ) : func(f) { }
                    void operator=( decltype(func) f ) { func = f; }
                    operator bool( ) const { return (bool) func; }
                    operator decltype(func)( ) const { return func; }
                    operator decltype(cfunc)*( ) const { return cfunc; }
                };

                // typical SAX function pointer -- argument prefixed with 'void*' to pick off SAX user context argument...
                //                                 which is not part of our cb<...> signature
                template<class F> struct traits;
                template<class R, class... Args>
                struct traits<R(*)(void*,Args...)> : public traits<R(Args...)> { };

                // special case SAX stdargs function pointer -- argument prefixed with 'void*' to pick off SAX user context
                //                                              argument... which is not part of our vacb<...> signature
                struct vatraits { };
                template<class R> struct traits<R(*)(void *, const char *, ...)> : public vatraits {
                     template <unsigned int num> struct index {
                          typedef vacb<num> type;
                     };
                };
                // typical SAX function traits with types explicitly enumerated...
                template<class R, class... Args> struct traits<R(Args...)> {
                    template <unsigned int num> struct index {
                        typedef cb<num, R, Args...> type;
                    };
                };

                // table of callbacks...
                traits<internalSubsetSAXFunc>::index<0>::type internalSubset;
                traits<isStandaloneSAXFunc>::index<1>::type isStandalone;
                traits<hasInternalSubsetSAXFunc>::index<2>::type hasInternalSubset;
                traits<hasExternalSubsetSAXFunc>::index<3>::type hasExternalSubset;
                traits<resolveEntitySAXFunc>::index<4>::type resolveEntity;
                traits<getEntitySAXFunc>::index<5>::type getEntity;
                traits<entityDeclSAXFunc>::index<6>::type entityDecl;
                traits<notationDeclSAXFunc>::index<7>::type notationDecl;
                traits<attributeDeclSAXFunc>::index<8>::type attributeDecl;
                traits<elementDeclSAXFunc>::index<9>::type elementDecl;
                traits<unparsedEntityDeclSAXFunc>::index<10>::type unparsedEntityDecl;
                traits<setDocumentLocatorSAXFunc>::index<11>::type setDocumentLocator;
                traits<startDocumentSAXFunc>::index<12>::type startDocument;
                traits<endDocumentSAXFunc>::index<13>::type endDocument;
                traits<startElementSAXFunc>::index<14>::type startElement;
                traits<endElementSAXFunc>::index<15>::type endElement;
                traits<referenceSAXFunc>::index<16>::type reference;
                traits<charactersSAXFunc>::index<17>::type characters;
                traits<ignorableWhitespaceSAXFunc>::index<18>::type ignorableWhitespace;
                traits<processingInstructionSAXFunc>::index<19>::type processingInstruction;
                traits<commentSAXFunc>::index<20>::type comment;
                traits<warningSAXFunc>::index<21>::type warning;
                traits<errorSAXFunc>::index<22>::type error;
                traits<fatalErrorSAXFunc>::index<23>::type fatalError; /* unused error() get all the errors */
                traits<getParameterEntitySAXFunc>::index<24>::type getParameterEntity;
                traits<cdataBlockSAXFunc>::index<25>::type cdataBlock;
                traits<externalSubsetSAXFunc>::index<26>::type externalSubset;
                traits<startElementNsSAX2Func>::index<27>::type startElementNs;
                traits<endElementNsSAX2Func>::index<28>::type endElementNs;
                traits<xmlStructuredErrorFunc>::index<29>::type serror;
                Table( ) { }
            } table;
            sax( ) { }

            xmlSAXHandler *handler( ) {
                fill( );
                return &_handler;
            }
            
            // offset meta information for callback table...
            // NOTE: static definition:
            //
            //      constexpr unsigned long dvo::xml2::sax::meta[];
            //
            // must be included somewhere in a *.cpp file...
            static constexpr unsigned long meta[] = { offsetof(Table,internalSubset), offsetof(Table,isStandalone),
                                                      offsetof(Table,hasInternalSubset), offsetof(Table,hasExternalSubset),
                                                      offsetof(Table,resolveEntity), offsetof(Table,getEntity),
                                                      offsetof(Table,entityDecl), offsetof(Table,notationDecl),
                                                      offsetof(Table,attributeDecl), offsetof(Table,elementDecl),
                                                      offsetof(Table,unparsedEntityDecl), offsetof(Table,setDocumentLocator),
                                                      offsetof(Table,startDocument), offsetof(Table,endDocument),
                                                      offsetof(Table,startElement), offsetof(Table,endElement),
                                                      offsetof(Table,reference), offsetof(Table,characters),
                                                      offsetof(Table,ignorableWhitespace), offsetof(Table,processingInstruction),
                                                      offsetof(Table,comment),
                                                      offsetof(Table,warning), offsetof(Table,error), offsetof(Table,fatalError),
                                                      offsetof(Table,getParameterEntity), offsetof(Table,cdataBlock),
                                                      offsetof(Table,externalSubset), offsetof(Table,startElementNs),
                                                      offsetof(Table,endElementNs), offsetof(Table,serror) };
         // private:
         //
         // Introducing 'private:' results in:
         // ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
         // In file included from dvo_adaptor.cpp:9:
         //     ../mk/../include/dvo/libxml2.hpp:24:89: warning: offset of on non-standard-layout type 'dvo::xml2::sax' [-Winvalid-offsetof]
         //     return static_cast<cb<index,R,Args...>*>( (void*) ( (char*) s + offsetof(sax,table) +
         //                                                                     ^            ~~~~~
         //     /usr/bin/../lib/clang/5.0/include/stddef.h:84:24: note: expanded from macro 'offsetof'
         //     #define offsetof(t, d) __builtin_offsetof(t, d)

                void fill( ) {
                    memset( &_handler, 0, sizeof(_handler) );
                    xmlSAXVersion( &_handler, 2 );
                    _handler.initialized = XML_SAX2_MAGIC;                    // so we do this to force parsing as SAX2.
                    if ( table.internalSubset ) _handler.internalSubset = table.internalSubset;
                    if ( table.isStandalone ) _handler.isStandalone = table.isStandalone;
                    if ( table.hasInternalSubset ) _handler.hasInternalSubset = table.hasInternalSubset;
                    if ( table.hasExternalSubset ) _handler.hasExternalSubset = table.hasExternalSubset;
                    if ( table.resolveEntity ) _handler.resolveEntity = table.resolveEntity;
                    if ( table.getEntity ) _handler.getEntity = table.getEntity;
                    if ( table.entityDecl ) _handler.entityDecl = table.entityDecl;
                    if ( table.notationDecl ) _handler.notationDecl = table.notationDecl;
                    if ( table.attributeDecl ) _handler.attributeDecl = table.attributeDecl;
                    if ( table.elementDecl ) _handler.elementDecl = table.elementDecl;
                    if ( table.unparsedEntityDecl ) _handler.unparsedEntityDecl = table.unparsedEntityDecl;
                    if ( table.setDocumentLocator ) _handler.setDocumentLocator = table.setDocumentLocator;
                    if ( table.startDocument ) _handler.startDocument = table.startDocument;
                    if ( table.endDocument ) _handler.endDocument = table.endDocument;
                    if ( table.startElement ) _handler.startElement = table.startElement;
                    if ( table.endElement ) _handler.endElement = table.endElement;
                    if ( table.reference ) _handler.reference = table.reference;
                    if ( table.characters ) _handler.characters = table.characters;
                    if ( table.ignorableWhitespace ) _handler.ignorableWhitespace = table.ignorableWhitespace;
                    if ( table.processingInstruction ) _handler.processingInstruction = table.processingInstruction;
                    if ( table.comment ) _handler.comment = table.comment;
                    if ( table.warning ) _handler.warning = table.warning;
                    if ( table.error ) _handler.error = table.error;
                    if ( table.fatalError ) _handler.fatalError = table.fatalError;
                    if ( table.getParameterEntity ) _handler.getParameterEntity = table.getParameterEntity;
                    if ( table.cdataBlock ) _handler.cdataBlock = table.cdataBlock;
                    if ( table.externalSubset ) _handler.externalSubset = table.externalSubset;
                    if ( table.startElementNs ) _handler.startElementNs = table.startElementNs;
                    if ( table.endElementNs ) _handler.endElementNs = table.endElementNs;
                    if ( table.serror ) _handler.serror = table.serror;
                }
                xmlSAXHandler _handler;
        };

    }
}