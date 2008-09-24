\name{hilbertDefaultPalette}
\alias{hilbertDefaultPalette}

\title{Default palette for Hilbert curve visualization}

\description{
Calculates a colour palette of length \code{size}. This palette is used 
as default by \code{\link{hilbertDisplay}}.
}

\usage{
hilbertDefaultPalette( size, asArray=TRUE )
}

\arguments{
\item{size}{The number of desired colours.}
\item{asArray}{Whether to return an array of RGB values or a character vector of
color specs.}
}

\value{
* if \code{asArray=TRUE} (default): An array with 3 rows and \code{size} columns, containing RGB values. This is
the same format as returned by \code{\link{col2rgb}}.

* if \code{asArray=FALSE}: A character vector of color specs, suitable to be
passed to the \code{col} argument of \code{\link{plot}}.
}

\seealso{
\code{\link{hilbertDefaultPalette}}
}
\examples{
# Get a palette
palette <- hilbertDefaultPalette(30)
# Transform from RGB triples to color strings (i.e., do the 
# reverse of col2rgb)
colors <- apply( palette, 2, function(a) rgb(a[1], a[2], a[3], max=255) )
# Plot the palette
plot.new()
plot.window( xlim=c(.5,30.5), ylim=c(0,1) )
rect( 1:30-.5, 0, 1:30+.5, 1, col=colors )
}
\author{Simon Anders, EMBL-EBI, \email{sanders@fs.tum.de}}
