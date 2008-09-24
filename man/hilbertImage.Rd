\name{hilbertImage}
\alias{hilbertImage}

\title{ Produce a matrix that visualizes a long data vector along a Hilbert curve }

\description{
   Calculate a Hilbert curve visualization of a long data vector and return it as a square matrix.
}

\usage{
hilbertImage(data, level = 9, forEBImage = FALSE)
}

\arguments{
  \item{data}{ A (potentially very long) vector of numerical data. }
  \item{level}{ The level of the Hilbert curve, determining the size of the returned matrix }
  \item{forEBImage}{ If set to TRUE, an Image object (as defined in the EBImage package) will be returned. }
}

\details{
See the package vignette for an explanation of this visualization technique.
}

\value{
  A matrix of dimension \code{2^level} x \code{2^level}. Each matrix element corresponds to a bin of consecutive elements
    of the data vector, the bins arranged to follow the Hilbert curve of the given level. The value of a 
    matrix element is the maximum value of the data vector elements in the bin.
    
  If \code{forEBIImage=TRUE}, a true-color Image object is returned instead, where each pixel is colored
  according to the palette constructed by the function \code{\link{hilbertDefaultPalette}}.
}
\author{ Simon Anders, EMBL-EBI, sanders@fs.tum.de }
\note{ For an interactive GUI to explore a Hilbert curve visualisation, use the function
\code{\link{hilbertDisplay}}.
}
\examples{
   # Get a vector with example data
   dataVec <- makeRandomTestData( )
   
   # Plot it in conventional (linear) fashion
   plotLongVector( dataVec )
   
   # Note how the peaks look quite uniform

   # Get the Hilbert curve matrix
   hMat <- hilbertImage( dataVec )
   
   # Plot it with the 'image' function and the 'hilbertDefaultPalette'
   image( hMat, col=hilbertDefaultPalette( max(hMat), asArray=FALSE ) )
   
   # Note how you can now see the non-uniformity hidden in the previous plot.
   # Note also the ugly aliasing when you change the size of the plot window.
   # Using EBImage allows to display in each matrix element as one pixel:
   
   library( EBImage )
   display( hilbertImage( dataVec, forEBImage=TRUE ) )

}
