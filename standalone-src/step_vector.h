#ifndef _STEP_VECTOR_H_
#define _STEP_VECTOR_H_

#include <map>
#include <stdexcept>
#include <limits>
#include <iostream>  //for now only

template< class T >
class step_vector {
  protected:
   std::map< long int, T > m;
  public: 
   long int min_index;
   long int max_index;
   typedef typename std::map< long int, T >::const_iterator const_iterator;
   step_vector( long int length, long int min_index_=0 );
   const T operator[]( long int i ) const; // How can I make sure that this is not used as l-value?
   void set_value( long int from, long int to, T value );
   void add_value( long int from, long int to, T value );
   const_iterator get_values( long int from );
   const_iterator begin( );
   const_iterator end( );
   T get_max( long int from, long int to );
   T get_min( long int from, long int to );
   std::pair< T, T > get_minmax( long int from, long int to );
};

template< class T >
step_vector<T>::step_vector( long int length, long int min_index_ ) 
 : min_index( min_index_ ),
   max_index( min_index_ + length - 1 )
{
   m[ min_index ] =  T();
}

template< class T >
const T step_vector<T>::operator[]( long int i ) const
{
   if( i > max_index ) 
      throw std::out_of_range( "Index too large in step_vector." );
   if( i < min_index ) 
      throw std::out_of_range( "Index too small in step_vector." );
   const_iterator it = m.upper_bound( i );
   it--;
   return it->second;
}

template< class T >
void step_vector<T>::set_value( long int from, long int to, T value )
{
   if( from > to )
      throw std::out_of_range( "Indices reversed in step_vector." );
   if( to > max_index )
      throw std::out_of_range( "Index too large in step_vector." );
   if( from < min_index )
      throw std::out_of_range( "Index too small in step_vector." );
   if( to < max_index ) {
      T next_value = (*this)[to+1];
      m[ to + 1 ] = next_value;
   }
   typename std::map< long int, T>::iterator it = m.lower_bound( from );
   if( it->first <= to ) {
      m.erase( it, m.upper_bound( to ) );
   }
   m[ from ] = value;
}

template< class T >
void step_vector<T>::add_value( long int from, long int to, T value )
{
   if( from > to )
      throw std::out_of_range( "Indices reversed in step_vector." );
   if( to > max_index )
      throw std::out_of_range( "Index too large in step_vector." );
   if( from < min_index )
      throw std::out_of_range( "Index too small in step_vector." );
   
   if( to < max_index ) {
      T next_value = (*this)[to+1];
      m[ to + 1 ] = next_value;
   }

   typename std::map< long int, T>::iterator it = m.upper_bound( from );
   it--;
   bool need_to_insert_step_at_from = it->first < from;
   T old_val_at_from;
   if( need_to_insert_step_at_from ) {
      old_val_at_from = it->second;
      it++;
   }
   // Now, it points to the first element with it->first >= from
   
   for( ; it != m.end() & it->first <= to; it++ ) 
      it->second += value;
   
   if( need_to_insert_step_at_from )
      m[ from ] = old_val_at_from + value;
}

template< class T >
typename step_vector<T>::const_iterator step_vector<T>::get_values( long int from )
{
   return --m.upper_bound( from );
}

template< class T >
typename step_vector<T>::const_iterator step_vector<T>::begin( )
{
   return m.begin();
}   

template< class T >
typename step_vector<T>::const_iterator step_vector<T>::end( )
{
   return m.end();
}   

template< class T >
T step_vector<T>::get_max( long int from, long int to )
{
   double mx = -std::numeric_limits<double>::max() ;
   for( const_iterator i = get_values( from ); 
         i != end() && i->first <= to; i++ ) {
      if( i->second > mx )
         mx = i->second;
   };
   return mx;
}

template< class T >
T step_vector<T>::get_min( long int from, long int to )
{
   double mn = std::numeric_limits<double>::max() ;
   for( const_iterator i = get_values( from ); 
         i != end() && i->first <= to; i++ ) {
      if( i->second < mn )
         mn = i->second;
   };
   return mn;
}

template< class T >
std::pair< T, T > step_vector<T>::get_minmax( long int from, long int to )
{
   double mn =  std::numeric_limits<double>::max() ;
   double mx = -std::numeric_limits<double>::max() ;
   for( const_iterator i = get_values( from ); 
         i != end() && i->first <= to; i++ ) {
      if( i->second < mn )
         mn = i->second;
      if( i->second > mx )
         mx = i->second;
   };
   return std::pair<T,T>( mn, mx );
}


#endif //_STEP_VECTOR_H_
