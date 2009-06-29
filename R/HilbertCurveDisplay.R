simpleLinPlot <- function( data, info ) {
   hw <- ( info$dispHi - info$dispLo ) / 1024
   left <- max( 1, info$bin - hw )
   right <- min( info$bin + hw, length(data) )
   plotLongVector( 
      data[left:right],
      offset = left,
      main=info$seqName,
      shrinkLength = min( 2*hw + 1, 4000 ) ) }    


hilbertDisplayThreeChannel <- function( 
      dataRed, dataGreen=0, dataBlue=0, 
      naColor = col2rgb( "gray" ),
      fullLength = max( length(dataRed), length(dataGreen), length(dataBlue) ),
      portrait = FALSE )
   .Call( `R_display_hilbert_3channel`, dataRed, dataGreen, dataBlue, 
      naColor, fullLength, portrait )
      
      
hilbertDisplay <- function( 
      ..., 
      palettePos = colorRampPalette( c( "white", "red" ) )( 300 ),
      paletteNeg = colorRampPalette( c( "white", "blue" ) )( length(palettePos) ),
      maxPaletteValue = NULL,
      naColor = "gray", 
      plotFunc = simpleLinPlot, 
      names=NULL,
      sameScale=TRUE,
      pow2=FALSE,
      portrait=TRUE,
      fullLengths = NULL ) {
      
   numargs <- length( dotsapply( function(x) NULL, ... ) )
   if( numargs == 0)
      stop( "Supply at least one data vector." )
   stopifnot( is.function( plotFunc ) )
   stopifnot( is.character( palettePos ) )
   stopifnot( is.character( paletteNeg ) )
   stopifnot( is.character( naColor ) )
   stopifnot( length( naColor ) == 1 )
   stopifnot( length( palettePos ) == length( paletteNeg ) )
   palettePos <- col2rgb( palettePos )
   paletteNeg <- col2rgb( paletteNeg )
   naColor <- col2rgb( naColor )
   if( is.null(names) )
      names <- sapply( substitute(list(...)), deparse )[-1]
   stopifnot( is.character( names ) )
   if( sameScale ) {
      if( !is.null(fullLengths) )
         warning( "Argument 'fullLengths' overriden by argument 'sameScale'." )
      maxlength <- max( unlist( dotsapply( length, ... ) ) )
      fullLengths <- rep( maxlength, numargs )
   }
   if( pow2 ) {
      if( is.null(fullLengths) )
         fullLengths <- unlist( dotsapply( length, ... ) )
      fullLengths <- sapply( fullLengths, function(a)
         2**ceiling( log( a, 2 ) ) )
   }
   if( ! is.null( fullLengths ) )
      fullLengths <- as.integer( fullLengths )
   if( is.null( maxPaletteValue ) ) {
      maxPaletteValue <- median( unlist( dotsapply( function(x) max(abs(x),na.rm=TRUE), ... ) ), na.rm=TRUE )
      maxPaletteValue <- 10**( ceiling( log10( maxPaletteValue ) * 4 ) / 4 )
   }
   stopifnot( mode( maxPaletteValue ) == "numeric" & length( maxPaletteValue ) == 1 )
   stopifnot( ! is.na( maxPaletteValue ) )
   
   names <- substr( names, 0, 40 )
   
   .External( `R_display_hilbert`, function( data, info ) try( plotFunc( data, info ) ), 
      names, as.integer( palettePos ), as.integer( paletteNeg ), 
      as.integer( naColor ), maxPaletteValue, fullLengths, portrait, ... )   
      
   invisible( )
}
      
