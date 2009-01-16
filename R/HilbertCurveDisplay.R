simpleLinPlot <- function( data, info ) {
   hw <- ( info$dispHi - info$dispLo ) / 1024
   left <- max( 1, info$bin - hw )
   right <- min( info$bin + hw, length(data) )
   plotLongVector( 
      data[left:right],
      offset = left,
      main=info$seqName,
      shrinkLength = min( 2*hw + 1, 4000 ) ) }    


hilbertDisplayOld <- function( 
      ..., 
      palette = hilbertDefaultPalette( 1 + min( 1000, max(..., na.rm=TRUE) ) ),
      paletteSteps = 1:(ncol(palette)-1),
      naColor = col2rgb( "gray" ), 
      plotFunc = simpleLinPlot, 
      names=NULL,
      sameScale=FALSE,
      pow2=FALSE,
      portrait=TRUE,
      fullLengths = NULL ) {
      
   numargs <- length( dotsapply( function(x) NULL, ... ) )
   if( numargs == 0)
      stop( "Supply at least one data vector." )
   stopifnot( is.function( plotFunc ) )
   stopifnot( nrow( palette ) == 3 )
   stopifnot( length( naColor ) == 3 )
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
   if( min(...) < 0 )
      warning( "Data contains negative values. The current implementation may display these incorrectly." )
      
   .External( `R_display_hilbert_old`, plotFunc, 
      names, as.integer( palette ), as.integer( naColor ), 
      as.numeric( paletteSteps ), fullLengths, portrait, ... )   
      
   invisible( )
}

hilbertDisplayThreeChannel <- function( 
      dataRed, dataGreen, dataBlue, 
      naColor = col2rgb( "gray" ),
      fullLength = max( length(dataRed), length(dataGreen), length(dataBlue) ),
      portrait = FALSE )
   .Call( `R_display_hilbert_3channel`, dataRed, dataGreen, dataBlue, 
      naColor, fullLength, portrait )
      
      
hilbertDisplay <- function( 
      ..., 
      palettePos = colorRampPalette( c( "white", "red" ) )( 300 ),
      paletteNeg = colorRampPalette( c( "white", "blue" ) )( 300 ),
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
   if( mode( palettePos ) == "character" )
      palettePos <- col2rgb( palettePos )
   if( mode( paletteNeg ) == "character" )
      paletteNeg <- col2rgb( paletteNeg )
   if( mode( naColor ) == "character" )
      naColor <- col2rgb( naColor )
   stopifnot( nrow( palettePos ) == 3 )
   stopifnot( nrow( paletteNeg ) == 3 )
   stopifnot( length( naColor ) == 3 )
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
      maxPaletteValue <- median( unlist( dotsapply( function(x) max(abs(x)), ... ) ) )
      maxPaletteValue <- 10**( ceiling( log10( maxPaletteValue ) * 4 ) / 4 )
   }
   stopifnot( mode( maxPaletteValue ) == "numeric" & length( maxPaletteValue ) == 1 )
   
   names <- substr( names, 0, 40 )
      
   .External( `R_display_hilbert`, plotFunc, 
      names, as.integer( palettePos ), as.integer( paletteNeg ), 
      as.integer( naColor ), maxPaletteValue, fullLengths, portrait, ... )   
      
   invisible( )
}
      
