\name{makeWiggleVector}
\alias{makeWiggleVector}
\title{ generate a "wiggle vector" from start/end/value data }
\description{
Given intervals in the form of a "start" and an "end" vectors and corresponding
values, generate a "wiggle vector" of a given length that contains the specified
values in the vector elements indicated by the intervals.
}
\usage{
makeWiggleVector(start, end, value, chrlength )
}
\arguments{
  \item{start}{ The start coordinates of the intervals. As usual in R, these ar 1-based. }
  \item{end}{ The end coordinates of the intervals. As usual, the end points are
  included }
  \item{value}{ The values to be put in the wiggle vector. Where intervals
  overlap, the values are added. }
  \item{chrlength}{ The desired length of the returned vector. }
}
\value{
  A vector as described above.
}
\author{ Simon Anders, EMBL-EBI, sanders\@fs.tum.de }
\seealso{ For a value vector containing only ones, this function acts similar as
the \code{pileup} function in the ShortRead package. }
\examples{
   intervalStarts <- c(3,10,17,22)
   intervalEnds <- c(7,13,20,26)
   values <- c(2, 1.5, .3, 4)
   chrlength <- 30
   wig <- makeWiggleVector( intervalStarts, intervalEnds, values, chrlength )
   # The same effect can be achieved with the following R code, which, however
   # is much slower:
   wig2 <- numeric(chrlength)
   for( i in 1:length(values) )
      wig2[ intervalStarts[i]:intervalEnds[i] ] <- 
         wig2[ intervalStarts[i]:intervalEnds[i] ] + values[i]
   # Let's check that we got the same:
   all( wig == wig2 )      
}
