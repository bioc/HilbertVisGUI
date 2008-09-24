\name{plotLongVector}
\alias{plotLongVector}
\title{ A simple function to plot a very long vector. }
\description{
This function does basically the same as just calling \code{plot(vec)} but is much faster in case of a very long vector. This is because it first calls \code{\link{shrinkVector}}.
}
\usage{
plotLongVector(vec, offset = 1, shrinkLength = 4000, xlab = "", ylab = "", type = "h", ...)
}
%- maybe also 'usage' for other objects documented here.
\arguments{
  \item{vec}{ The numerical vector to be plotted }
  \item{offset}{ The x axis is labelled with numbers from \code{offset} to \code{offset+length(vec)-1}. }
  \item{shrinkLength}{ To which length to shrink the vector before plotting it. Should be at least the width of your plot in pixels.}
  \item{xlab}{ The label of the x axis, to be passed to \code{\link{plot}}. }
  \item{ylab}{ The label of the y axis, to be passed to \code{\link{plot}}. }
  \item{type}{ The plot type, to be passed to \code{\link{plot}}. By default, type 'h', i.e., needles, are used. }
  \item{\dots}{ Further arguments to be passed to \code{\link{plot}}.}
}
\value{
  Invisible Null and a plot.
}
\author{ Simon Anders, EMBL-EBI, sanders@fs.tum.de }

\examples{
plotLongVector( rep( 1:100000, 20 ) )
}
