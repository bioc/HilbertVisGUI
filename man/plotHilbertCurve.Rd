\name{plotHilbertCurve}

\alias{plotHilbertCurve}

\title{ Plotting the Hilbert curve (for demonstation purposes). }
\description{
  This function plots the Hilbert curve fractal at a chosen iteration level in
  order to give you an impression how it looks like.
}

\usage{plotHilbertCurve( lv, new.page = TRUE )}

\arguments{
  \item{lv}{The iteration level. A Hilbert curve of level \code{lv} spans a square with
  side length \code{2^lv} (coordinates ranging from 0 to \code{2^lv-1}) and has \code{4^lv}
  points. Values \code{lv > 7} will take very long and yield a cluttered mash of
  undistuingishable lines.}
  \item{new.page}{Boolean indicating whether to start a new graphics page (default: yes).}
}


\value{An invisble NULL is returned. Furthermore, a plot is created.}

\author{ Simon Anders, EMBL-EBI, \email{sanders@fs.tum.de}}

\seealso{ \code{\link{hilbertCurve}} }

\examples{
   plotHilbertCurve( 3 )
}   
