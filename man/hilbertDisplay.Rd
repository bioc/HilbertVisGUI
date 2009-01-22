\name{hilbertDisplay}
\alias{hilbertDisplay}

\title{Hilbert curve visualization}

\description{
Display one or several long integer vectors in an inetractive fashion 
by means of the Hilbert curve.
}

\usage{
hilbertDisplay( 
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
      fullLengths = NULL )
}

\arguments{

\item{...}{The data vectors to be visualized. This may be integer or real vectors. 
Care is taken within \code{hilbertDisplay} that these vectors do not get duplicated, 
so that you can pass very large vectors. }

\item{palettePos}{The color palette used to visualize positive valued bins in the data. 
This must be either be a character vector of valid color names. If no palette is 
supplied, a ramp from white to red is used.}

\item{paletteNeg}{The color palette for bins with negative values. Both palettes 
must have the same number of colours.}

\item{maxPaletteValue}{The absolute bin value that should correspond to the end of the 
two palettes. (The beginning of the palettes always represents 0.) Within the graphical
user interface, buttons are provided to adjust this interactively. If no value is
provided, the median of the absolute values of maxima and minima of the data vectors
is used.}

\item{naColor}{The color to be used for bins which contain NAs or correspond to
data outside the limits of the data vector. Pass a color name or a triple of RGB values. By 
default, "gray" is used.} 

\item{plotFunc}{An R function that is called if you use the "Linear plot" function 
offer by HilbertCurveDisplay's GUI. If you enable this function and then click on a
pixel in the display, the function supplied as \code{plotFunc} is called. If you do
not supply this parameter, the function \code{\link{simpleLinPlot}} (part of this
package) is used. If you supply your own function, it must accept two parameters: 
\code{data} and \code{info}. \code{data} will be the currently displayed data
vector. Be careful that your function does not duplicate it (check with
\code{\link{tracemem}}, if in doubt) in order to avoid performance problems when
dealing with large data. The second argument, \code{info}, is a list, supplying the
following fields, all of which, except for the last one, are single integers:
\code{info$binLo}, \code{info$bin}, and \code{info$binHi} are the lower, middle,
and upper coordinate (i.e., vector index) of the bin represented by the pixel onto 
which the user has clicked. \code{info$dispLo} and \code{info$dispHi} are the 
lowest and highest index of the part of the vector currently displayed.
\code{info$seqIdx} is the index of the currently displayed vector (i.e., its
position in the '...' argument) and \code{ionfo$seqName} is its name. All indices
are one-based. Your function should plot a region of interest around
\code{data[info$bin]}, or do some other useful operation. Any return value is
ignored. For a very simple example, see the body of \code{\link{simpleLinPlot}}.}

\item{names}{The names of the sequences. A character vector. If not supplied, the
expressions used in the '...' argument, as reported by \code{\link{substitute}}
are used.}

\item{sameScale}{Setting this argument to \code{TRUE} pads all but the largest vector
with \code{NA}s such that all vectors have the same length. (The padding is done
"virtually", i.e. no dupliocation in memory occurs.) The purpose of this is to make
sure that the bin size (i.e. the number of values depicted by one pixel) stays
constant, when using the DisplayHilbertViewer GUI's "Prev" and "Next" buttons,
which switch the display through the supplied data vectors.}

\item{pow2}{Setting thsi argument to \code{TRUE} pads all vectors virtually with
\code{NA}s such
that their length becomes a power of 2. The purpose of this becomes apparent if you
zoom in so much that several pixels correspond to the same data vector element.
Then, without this options, the values take on strange fractal forms, while they
are square in case of a power-of-2 length.}

\item{portrait}{Setting this option to \code{FALSE} changes the GUI layout such that the
controls appear to the right of the curve display ("landscape layout") as opposed to the usual case of
them appearing below ("portrait layout"). This is useful for small screens as the GUI window may be two
tall to fit on the screen in portrait mode.}

\item{fullLengths}{This option allows you to manually control the padding of
vectors with \code{NA}s if you do not like the result of the \code{same.scale} or
\code{pow2} option. Supply an integer vector with as many values as their are vectors in the
'...' argument, specifying the length including padding for each data vector.
Passing numbers smaller than the length of the data.vector results in only the
beginning of the vector being displayed.}
}

\value{Returns an invisible NULL.}

\seealso{
  \code{\link{simpleLinPlot}}, \code{\link[HilbertVis:hilbertDefaultPalette]{hilbertDefaultPalette}}
}
\examples{
random <- c( as.integer( runif(100000)*30 ) ) 
ramp <- c( as.integer( 0:19999/100 ) )
try( hilbertDisplay( random, ramp ) )
}
\author{Simon Anders, EMBL-EBI, \email{sanders@fs.tum.de}}
