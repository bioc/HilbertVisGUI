\name{shrinkVector}
\alias{shrinkVector}

\title{ shrink a vector by partitioning it into bins and taking the maxima in
the bins }
\description{
Given a (potentially very long) vector, the vector is partitioned into a given 
number of (up to rounding errors) equally long parts, and a vector of the 
maxima within each of the parts it returned. 
}
\usage{
shrinkVector(vec, newLength)
}
\arguments{
  \item{vec}{ The vector to be shrunk. May be numeric or integer.}
  \item{newLength}{ The desired size of the return vector, i.e., the number of
  partitions }
}
\details{
   This function is useful when plotting a very long vector such as those
   returned by \code{ShortRead::pileup}. For a sufficiently long vector \code{x}, 
   the two commands \code{plot( x, type="h" )} and \code{plot( shrinkVector( x,
   10000 ), type="h" )} produce essential the same plot (up to a rescaling of
   the x axis) but the former takes much longer to execute because R plots many
   needles on top of each other.
   }
\value{
   A vector of length \code{newLength} with the maxima of each of the partitions
   of \code{vector}.
}
\author{ Simon Anders, EMBL-EBI (sanders\@fs.tum.de) }
\note{ 
   Note that, as the maximum is returned, the result may mot be what you want if
   negative numbers are present. Note further that NAs may not be handles
   correctly.
}
\seealso{ \code{ShortRead::pileup}, \code{\link{simpleLinPlot}} }
\examples{
   shrinkVector( 100000 + 1:1000, 17 )
}
