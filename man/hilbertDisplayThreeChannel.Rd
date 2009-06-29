\name{hilbertDisplayThreeChannel}
\alias{hilbertDisplayThreeChannel}
\title{ Show up to three data vectors in the Hilbert curve display GUI.}
\description{
This is a variant of \code{\link{hilbertDisplay}} that takes up to three long data vector but does not display them in different panels (allowing to switch with the ``Previous'' and ``Next'' buttons but instead put them in one picture, using the three colour channels to overlay them. This is look for correlations in spatial distribution of the different data vectors.
}
\usage{
hilbertDisplayThreeChannel(
   dataRed, 
   dataGreen = 0, 
   dataBlue = 0, 
   naColor = col2rgb("gray"), 
   fullLength = max(length(dataRed), length(dataGreen), length(dataBlue)), 
   portrait = FALSE)
}
%- maybe also 'usage' for other objects documented here.
\arguments{
  \item{dataRed}{ The data to be displayed in the red colour channel, to be provided as a numeric vector with values between 0 (black) and 1 (bright red). You may either pass an ordinary R vector or an \code{Rle} vector
  as defined in the \code{IRanges} package.}
  \item{dataGreen}{ As \code{dataRed} but for the green channel. }
  \item{dataBlue}{ As \code{dataRed} but for the blue channel. }
  \item{naColor}{ The color to use to indicate NA. }
  \item{fullLength}{ The vector length that should correspond to the full display. By default, the length of the longest data vector. It may make sense to round up to the next power of two.  }
  \item{portrait}{ Whether to arrange the buttons to the right of the display. Useful on small screens. }
}
\value{ Invisible NULL.
}
\author{ Simon Anders }
\note{ This function is a bit less mature than the function \code{\link{hilbertDisplay}}. Especially, the GUI functions ``Linear plot'', ``Previous'', and ``Next'' are displayed but defunct.
}
\seealso{ \code{\link{hilbertDisplay}} }
