\name{hilbertCurve}

\alias{hilbertCurve}
\alias{hilbertCurvePoint}

\title{ calculate finite approximations of the Hilbert curve }
\description{
   These functions calculate the Hilbert curve in its finite approximations.
   \code{hilbertCurvePoint} gives teh coordinates of one point and \code{hilbertCurve} 
   returns an array with the coordinates of all \code{4^lv} points. The
   functions are not needed for \code{\link{hilbertDisplay}} and only provided
   for demonstration purposes. \code{\link{plotHilbertCurve}} makes use of them.
}

\usage{
hilbertCurve( lv )
hilbertCurvePoint( t, lv )
}

\arguments{
  \item{lv}{The iteration level. A hilbert curve of level \code{lv} spans a square with
  side length \code{2^lv} (coordinates ranging from 0 to \code{2^lv-1}) and has \code{4^lv}
  points.}
  \item{t}{The point index in the Hilbert curve. Must be an integer in \code{0:(4^lv-1)}.}
}

\value{
   \code{hilbertCurvePoint} returns a vector of two integer numbers, both in the range
   \code{0:(2^lv-1)}, indicating the coordinates of point \code{t}. \code{huilbertCurve} returns a
   matrix with \code{4^lv} rows and 2 columns, giving all points of the curve at level \code{lv}.
}
\author{ Simon Anders, EMBL-EBI, \email{sanders@fs.tum.de}}
\seealso{ \code{\link{plotHilbertCurve}} }
\examples{
   hilbertCurvePoint( 67, 4 )
   hilbertCurve( 4 )
}   
