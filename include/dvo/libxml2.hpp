//# libxml2.hpp: C++ interface to libxml2
//# Copyright (C) 2014
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
#pragma once
#include <tuple>
#include <vector>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <libxml/parser.h>
#include <codecvt>

namespace dvo {
    namespace xml2 {

        // std::string can handle utf-8. however, it is important to keep in mind the fact that the lengths
        // reported are the bytes that make up the string (rather than the characters, e.g. two chinese
        // characters may be represented by 6 bytes. As long as all characters are represented as utf-8
        // within the program all is still good...
        inline std::string as_string(const xmlChar *xmlString) {
             return xmlString ? std::string((char*)xmlString) : std::string( );
        }

        namespace pvt {

            //----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
            template< class, class > struct union_t;
            
            template< template< class... > class T, class...Args1,
                      template< class... > class U, class...Args2 >
            struct union_t< T< Args1... >, U< Args2... > > {
                     typedef T<Args1...,Args2...> type;
            };
            template< template< class... > class T, class...Args1 >
            struct union_t< T< Args1... >, void > {
                     typedef T<Args1...> type;
            };


            //----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
            template<typename Result, typename... Args> struct function_t {
                typedef std::function<Result(Args...)> func;
                typedef function_t<Result,Args...> type;
                typedef std::tuple<Args...> args;
            };


            //----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
            // generate sequences of integers for pulling off elements of a tuple...
            template<int...indexes> struct seq { static std::vector<int> value( ) { std::vector<int> result{indexes...}; return result; } };
            template<int N, int ...S> struct genseq : genseq<N-1, N-1, S...> { };
            template<int ...S> struct genseq<0, S...> {
                typedef seq<S...> type;
            };


            //----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
            template<typename... Rest> struct morph;
            template<> struct morph<> {
                typedef void type;
            
                template<class TUPLE, int N>
                static void fill(TUPLE &args) { }
            
            };

            template<typename... Rest> struct morph<int,int,const xmlChar**, Rest...> {
                // throw away the "number defaulted" parameter (second 'int')...
                typedef typename union_t<function_t<std::vector<std::tuple<std::string,std::string,std::string,std::string>>>,typename morph<Rest...>::type>::type::type type;
                template<class TUPLE, int N>
                static void fill(TUPLE &args, int num, int, const xmlChar **a1, Rest...rest) {                                   //  <-----<<< thought to be attributes
                    std::vector<std::tuple<std::string,std::string,std::string,std::string>> vals;
                    if ( a1 ) {
                        unsigned int index=0;
                        for ( int x=0; x < num; ++x, index += 5 ) {
                            std::tuple<std::string,std::string,std::string,std::string> val;
                            std::get<0>(val) = std::move(a1[index]   ? std::string((const char*) a1[index]) : std::string( ));   //  <-----<<< local name
                            std::get<1>(val) = std::move(a1[index+1] ? std::string((const char*) a1[index+1]) : std::string( )); //  <-----<<< prefix
                            std::get<2>(val) = std::move(a1[index+2] ? std::string((const char*) a1[index+2]) : std::string( )); //  <-----<<< URI
                            const xmlChar *valueBegin = a1[index+3];                                                             //  <-----<<< value
                            const xmlChar *valueEnd = a1[index+4];
                            std::get<3>(val) = std::move( valueBegin && valueEnd ? std::string((const char*)valueBegin,(const char*)valueEnd) : std::string( ));
                            vals.push_back(std::move(val));
                        }
                    }
                    std::get<N>(args) = std::move(vals);
                    morph<Rest...>::template fill<TUPLE,N+1>(args,rest...);
                }
            
            };

            template<typename... Rest> struct morph<int,const xmlChar**, Rest...> {
                typedef typename union_t<function_t<std::vector<std::tuple<std::string,std::string>>>,typename morph<Rest...>::type>::type::type type;
                template<class TUPLE, int N>
                static void fill(TUPLE &args, int num, const xmlChar **a1, Rest...rest) {                                        //  <-----<<< thought to be namespaces
                    std::vector<std::tuple<std::string,std::string>> vals;
                    if ( a1 ) {
                        unsigned int index=0;
                        for ( int x=0; x < num; ++x, index += 2 ) {
                            std::tuple<std::string,std::string> val;
                            std::get<0>(val) = std::move(a1[index]   ? std::string((const char*) a1[index]) : std::string( ));   //  <-----<<< prefix
                            std::get<1>(val) = std::move(a1[index+1] ? std::string((const char*) a1[index+1]) : std::string( )); //  <-----<<< URI
                            vals.push_back(std::move(val));
                        }
                    }
                    std::get<N>(args) = std::move(vals);
                    morph<Rest...>::template fill<TUPLE,N+1>(args,rest...);
                }
            
            };

            template<typename... Rest> struct morph<const xmlChar*,Rest...> {
                typedef typename union_t<function_t<std::string>,typename morph<Rest...>::type>::type::type type;
            
                template<class TUPLE, int N>
                static void fill(TUPLE &args, const xmlChar *a1, Rest...rest) {
                    std::get<N>(args) = std::move(a1 ? std::string((char*)a1) : std::string( ));
                    morph<Rest...>::template fill<TUPLE,N+1>(args,rest...);
                }
            
            };
            
            template<typename T, typename... Rest> struct morph<T,Rest...> {
                typedef typename union_t<function_t<T>,typename morph<Rest...>::type>::type type;

                // std::enable_if<...>::type needed because 'T'/'U' can be void (when it's the return type)...
                template<class TUPLE, int N, typename U = T>
                static typename std::enable_if<!std::is_same<U,void>::value,void >::type fill(TUPLE &args, U a1, Rest...rest) {
                    std::get<N>(args) = a1;
                    morph<Rest...>::template fill<TUPLE,N+1>(args,rest...);
                }
            
                static T invoke( typename union_t<function_t<T>,typename morph<Rest...>::type>::type::func f, Rest...rest ) {
                    typename union_t<function_t<T>,typename morph<Rest...>::type>::type::args args;
                    morph<Rest...>::template fill<decltype(args),0>(args,rest...);
                    return invoke_1( f, args );
                }
            private:
                template<typename...TupArgs, int...index>
                static T invoke_2( typename union_t<function_t<T>,typename morph<Rest...>::type>::type::func f, std::tuple<TupArgs...> args, seq<index...> ) {
                    return f(std::get<index>(args)...);
                }
                template<typename...TupArgs>
                static T invoke_1( typename union_t<function_t<T>,typename morph<Rest...>::type>::type::func f, std::tuple<TupArgs...> args ) {
                    return invoke_2( f, args, typename genseq<sizeof...(TupArgs)>::type( ) ); 
                }
            
            };
        }
            
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
                template<unsigned int cbindex, typename R, typename... Args> class cb {
                    // 'func' provides the binding point for callback functions, e.g. a lambda function...
                    typename pvt::morph<R,Args...>::type::func func;
                    // 'cfunc' provides the C-stype callback function which is coupled to the
                    // stored (e.g. lambda) function but can be given to libxml2 C library...
                    static R cfunc(void *s,Args... cargs) {
                        return pvt::morph<R,Args...>::invoke(static_cast<cb<cbindex,R,Args...>*>( (void*) ( (char*) s + offsetof(sax,table) + static_cast<sax*>(s)->meta[cbindex] ) )->func, cargs...);
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
                template<unsigned int cbindex> class vacb {
                    std::function<void(std::string)> func;
                    static void cfunc(void *s, const char *format, ...) {
                        char buffer[1024 * 4];
                        va_list argptr;
                        va_start(argptr, format);
                        vsprintf(buffer, format, argptr);
                        va_end(argptr);
                        static_cast<vacb<cbindex>*>( (void*) ( (char*) s + offsetof(sax,table) +
                                                             static_cast<sax*>(s)->meta[cbindex] ) )->func(std::string(buffer));
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
