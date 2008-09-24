\name{simpleLinPlot}
\alias{simpleLinPlot}

\title{Default plot callback function for hilbertDisplay}

\description{
Plots a small part of the supplied vector.
}

\usage{
simpleLinPlot( data, info )
}

\arguments{
\item{data}{The data vector, a vector of integers.}
\item{info}{A list with fields describing the context, as described in the help
page for \code{\link{hilbertDisplay}}. }
}

\value{Null.}

\details{This is a very simple function that is called by default by 
\code{\link{hilbertDisplay}} when the user chooses the "Linear plot" option and
clicks on a pixel in the display. It displays a piece of 2000 values of the
\code{data} vector, centered around \code{info$bin}.
You will often want to replace this function by one tailored to your needs.}

\seealso{
\code{\link{hilbertDisplay}}
}
\author{Simon Anders, EMBL-EBI, \email{sanders@fs.tum.de}}
