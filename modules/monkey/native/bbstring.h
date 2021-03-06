
#ifndef BB_STRING_H
#define BB_STRING_H

#include "bbtypes.h"
#include "bbmemory.h"

class bbCString;

class bbString{

	struct Rep{
		int refs;
		int length;
		bbChar data[0];
		
		static Rep *alloc( int length ){
			if( !length ) return &_nullRep;
			Rep *rep=(Rep*)bbMalloc( sizeof(Rep)+length*sizeof(bbChar) );
			rep->refs=1;
			rep->length=length;
			return rep;
		}
		
		template<class C> static Rep *create( const C *p,int length ){
			Rep *rep=alloc( length );
			for( int i=0;i<length;++i ) rep->data[i]=p[i];
			return rep;
		}
		
		template<class C> static Rep *create( const C *p ){
			const C *e=p;
			while( *e ) ++e;
			return create( p,e-p );
		}
	};
	
	Rep *_rep;

	static Rep _nullRep;
	
	void retain()const{
		++_rep->refs;
	}
	
	void release(){
		if( !--_rep->refs && _rep!=&_nullRep ) bbFree( _rep );
	}
	
	bbString( Rep *rep ):_rep( rep ){
	}
	
	template<class C> static int t_memcmp( const C *p1,const C *p2,int count ){
		return memcmp( p1,p2,count*sizeof(C) );
	}

	//returns END of dst!	
	template<class C> static C *t_memcpy( C *dst,const C *src,int count ){
		return (C*)memcpy( dst,src,count*sizeof(C) )+count;
	}
	
	public:
	
	const char *c_str()const;
	
	bbString():_rep( &_nullRep ){
	}
	
	bbString( const bbString &str ):_rep( str._rep ){
		retain();
	}
	
	template<class C> bbString( const C *data ):_rep( Rep::create( data ) ){
	}

	template<class C> bbString( const C *data,int length ):_rep( Rep::create( data,length ) ){
	}
	
	bbString( bbInt n ){
		char data[64];
		sprintf( data,"%i",n );
		_rep=Rep::create( data );
	}
	
	bbString( bbUInt n ){
		char data[64];
		sprintf( data,"%u",n );
		_rep=Rep::create( data );
	}
	
	bbString( bbLong n ){
		char data[64];
		sprintf( data,"%lld",n );
		_rep=Rep::create( data );
	}
	
	bbString( bbULong n ){
		char data[64];
		sprintf( data,"%llu",n );
		_rep=Rep::create( data );
	}
	
	bbString( float n ){
		char data[64];
		sprintf( data,"%.9g",n );
		_rep=Rep::create( data );
	}
	
	bbString( double n ){
		char data[64];
		sprintf( data,"%.17g",n );
		_rep=Rep::create( data );
	}
	
	~bbString(){
		release();
	}
	
	const bbChar *data()const{
		return _rep->data;
	}
	
	int length()const{
		return _rep->length;
	}
	
	bbChar operator[]( int index )const{
		return data()[index];
	}
	
	bbString operator+()const{
		return *this;
	}
	
	bbString operator-()const{
		Rep *rep=Rep::alloc( length() );
		const bbChar *p=data()+length();
		for( int i=0;i<rep->length;++i ) rep->data[i]=*--p;
		return rep;
	}
	
	bbString operator+( const bbString &str ){
	
		if( !length() ) return str;
		if( !str.length() ) return *this;
		
		Rep *rep=Rep::alloc( length()+str.length() );
		t_memcpy( rep->data,data(),length() );
		t_memcpy( rep->data+length(),str.data(),str.length() );

		return rep;
	}
	
	bbString operator+( const char *str ){
		return operator+( bbString( str ) );
	}
	
	bbString operator*( int n ){
		Rep *rep=Rep::alloc( length()*n );
		bbChar *p=rep->data;
		for( int j=0;j<n;++j ){
			for( int i=0;i<_rep->length;++i ) *p++=data()[i];
		}
		return rep;
	}
	
	bbString &operator=( const bbString &str ){
		str.retain();
		release();
		_rep=str._rep;
		return *this;
	}
	
	template<class C> bbString &operator=( const C *data ){
		release();
		_rep=Rep::create( data );
		return *this;
	}
	
	bbString &operator+=( const bbString &str ){
		*this=*this+str;
		return *this;
	}
	
	bbString &operator+=( const char *str ){
		return operator+=( bbString( str ) );
	}
	
	int find( bbString str,int from=0 )const{
		if( from<0 ) from=0;
		for( int i=from;i<=length()-str.length();++i ){
			if( !t_memcmp( data()+i,str.data(),str.length() ) ) return i;
		}
		return -1;
	}
	
	int findLast( const bbString &str,int from=0 )const{
		if( from<0 ) from=0;
		for( int i=length()-str.length();i>=from;--i ){
			if( !t_memcmp( data()+i,str.data(),str.length() ) ) return i;
		}
		return -1;
	}
	
	bool contains( const bbString &str )const{
		return find( str )!=-1;
	}
	
	bbString slice( int from )const{
		int length=this->length();
		if( from<0 ){
			from+=length;
			if( from<0 ) from=0;
		}else if( from>length ){
			from=length;
		}
		if( !from ) return *this;
		return bbString( data()+from,length-from );
	}
	
	bbString slice( int from,int term )const{
		int length=this->length();
		if( from<0 ){
			from+=length;
			if( from<0 ) from=0;
		}else if( from>length ){
			from=length;
		}
		if( term<0 ){
			term+=length;
			if( term<from ) term=from;
		}else if( term<from ){
			term=from;
		}else if( term>length ){
			term=length;
		}
		if( !from && term==length ) return *this;
		return bbString( data()+from,term-from );
	}

	bbString left( int count )const{
		return slice( 0,count );
	}
	
	bbString right( int count )const{
		return slice( -count );
	}
	
	bbString mid( int from,int count )const{
		return slice( from,from+count );
	}
	
	bool startsWith( const bbString &str )const{
		if( str.length()>length() ) return false;
		return t_memcmp( data(),str.data(),str.length() )==0;
	}
	
	bool endsWith( const bbString &str )const{
		if( str.length()>length() ) return false;
		return t_memcmp( data()+(length()-str.length()),str.data(),str.length() )==0;
	}
	
	bbString toUpper()const{
		Rep *rep=Rep::alloc( length() );
		for( int i=0;i<length();++i ) rep->data[i]=std::toupper( data()[i] );
		return rep;
	}
	
	bbString toLower()const{
		Rep *rep=Rep::alloc( length() );
		for( int i=0;i<length();++i ) rep->data[i]=std::tolower( data()[i] );
		return rep;
	}
	
	bbString capitalize()const{
		if( !length() ) return &_nullRep;
		Rep *rep=Rep::alloc( length() );
		rep->data[0]=std::toupper( data()[0] );
		for( int i=1;i<length();++i ) rep->data[i]=data()[i];
		return rep;
	}
	
	bbString trim()const{
		const bbChar *beg=data();
		const bbChar *end=data()+length();
		while( beg!=end && *beg<=32 ) ++beg;
		while( beg!=end && *(end-1)<=32 ) --end;
		if( end-beg==length() ) return *this;
		return bbString( beg,end-beg );
	}
	
	bbString replace( const bbString &str,const bbString &repl )const{
	
		int n=0;
		for( int i=0;; ){
			i=find( str,i );
			if( i==-1 ) break;
			i+=str.length();
			++n;
		}
		if( !n ) return *this;
		
		Rep *rep=Rep::alloc( length()+n*(repl.length()-str.length()) );
		
		bbChar *dst=rep->data;
		
		for( int i=0;; ){
		
			int i2=find( str,i );
			if( i2==-1 ){
				t_memcpy( dst,data()+i,(length()-i) );
				break;
			}
			
			t_memcpy( dst,data()+i,(i2-i) );
			dst+=(i2-i);
			
			t_memcpy( dst,repl.data(),repl.length() );
			dst+=repl.length();
			
			i=i2+str.length();
		}
		return rep;
	}
	
	bbArray<bbString> *split( bbString sep )const;
	
	bbString join( bbArray<bbString> *bits )const;
	
	int compare( const bbString &t )const{
		int len=length()<t.length() ? length() : t.length();
		for( int i=0;i<len;++i ){
			if( int n=data()[i]-t.data()[i] ) return n;
		}
		return length()-t.length();
	}
	
	bool operator<( const bbString &t )const{
		return compare( t )<0;
	}
	
	bool operator>( const bbString &t )const{
		return compare( t )>0;
	}
	
	bool operator<=( const bbString &t )const{
		return compare( t )<=0;
	}
	
	bool operator>=( const bbString &t )const{
		return compare( t )>=0;
	}
	
	bool operator==( const bbString &t )const{
		return compare( t )==0;
	}
	
	bool operator!=( const bbString &t )const{
		return compare( t )!=0;
	}
	
	operator bbBool()const{
		return length();
	}
	
	operator bbInt()const{
		return std::atoi( c_str() );
	}
	
	operator bbByte()const{
		return operator bbInt() & 0xff;
	}
	
	operator bbUByte()const{
		return operator bbInt() & 0xffu;
	}
	
	operator bbShort()const{
		return operator bbInt() & 0xffff;
	}
	
	operator bbUShort()const{
		return operator bbInt() & 0xffffu;
	}
	
	operator bbUInt()const{
		bbUInt n;
		sscanf( c_str(),"%u",&n );
		return n;
	}
	
	operator bbLong()const{
		bbLong n;
		sscanf( c_str(),"%lld",&n );
		return n;
	}
	
	operator bbULong()const{
		bbULong n;
		sscanf( c_str(),"%llu",&n );
		return n;
	}
	
	operator float()const{
		return std::atof( c_str() );
	}
	
	operator double()const{
		return std::atof( c_str() );
	}
	
	int utf8Length()const;
	
	void toCString( void *buf,int size )const;

	void toWString( void *buf,int size )const;
	
	void toUtf8String( void *buf,int size )const;
	
	static bbString fromChar( int chr );
	
	static bbString fromCString( const void *data,int size );

	static bbString fromWString( const void *data,int size );
	
	static bbString fromUtf8String( const void *data,int size );
	
	static bbString fromTString( const void *data,int size ){
#if _WIN32
		return fromCString( data,size );
#else
		return fromUtf8String( data,size );
#endif
	}
	
	static bbString fromCString( const void *data ){
		return fromCString( data,0x7fffffff );
	}
	
	static bbString fromWString( const void *data ){
		return fromWString( data,0x7fffffff );
	}
	
	static bbString fromUtf8String( const void *data ){
		return fromUtf8String( data,0x7fffffff );
	}
	
	static bbString fromTString( const void *data ){
		return fromTString( data,0x7fffffff );
	}
};

class bbCString{
	bbString _str;
	mutable char *_data=nullptr;
	
	public:
	
	bbCString( const bbString &str ):_str( str ){
	}
	
	bbCString( const bbCString &str ):_str( str._str ){
	}
	
	~bbCString(){
		free( _data );
	}
	
	operator bbString()const{
		return _str;
	}
	
	bbCString &operator=( const bbCString &str ){
		free( _data );
		_data=nullptr;
		_str=str._str;
		return *this;
	}
	
	char *data()const{
		if( _data ) return _data;
		int size=_str.length()+1;
		_data=(char*)malloc( size );
		_str.toCString( _data,size );
		return _data;
	}
	
	operator char*()const{
		return data();
	}
};

class bbWString{
	bbString _str;
	mutable wchar_t *_data=nullptr;
	
	public:
	
	bbWString( const bbString &str ):_str( str ){
	}
	
	bbWString( const bbWString &str ):_str( str._str ){
	}
	
	~bbWString(){
		free( _data );
	}
	
	operator bbString()const{
		return _str;
	}
	
	bbWString &operator=( const bbWString &str ){
		free( _data );
		_data=nullptr;
		_str=str._str;
		return *this;
	}
	
	wchar_t *data()const{
		if( _data ) return _data;
		int size=(_str.length()+1)*sizeof(wchar_t);
		_data=(wchar_t*)malloc( size );
		_str.toWString( _data,size );
		return _data;
	}
	
	operator wchar_t*()const{
		return data();
	}
};

class bbUtf8String{
	bbString _str;
	mutable unsigned char *_data=nullptr;
	
	public:
	
	bbUtf8String( const bbString &str ):_str( str ){
	}
	
	bbUtf8String( const bbUtf8String &str ):_str( str._str ){
	}
	
	~bbUtf8String(){
		free( _data );
	}
	
	bbUtf8String &operator=( const bbUtf8String &str ){
		free( _data );
		_data=nullptr;
		_str=str._str;
		return *this;
	}
	
	operator bbString()const{
		return _str;
	}
	
	unsigned char *data()const{
		if( _data ) return _data;
		int size=_str.utf8Length()+1;
		_data=(unsigned char*)malloc( size );
		_str.toUtf8String( _data,size );
		return _data;
	}
	
	operator char*()const{
		return (char*)data();
	}
	
	operator unsigned char*()const{
		return data();
	}
};

#if _WIN32
typedef bbCString bbTString;
#else
typedef bbUtf8String bbTString;
#endif

template<class C> bbString operator+( const C *str,const bbString &str2 ){
	return bbString( str )+str2;
}

template<class C> bbString BB_T( const C *p ){
	return bbString( p );
}

inline void bbGCMark( const bbString &t ){
}

#endif
